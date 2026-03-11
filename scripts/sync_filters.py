#!/usr/bin/env python3
"""Synchronize VS .vcxproj.filters structure to disk folder layout.

Reads .vcxproj.filters as the source of truth and moves files on disk
to match the filter hierarchy, updating all Include paths and #include
directives accordingly.

Usage:
    python scripts/sync_filters.py                    # dry-run (default)
    python scripts/sync_filters.py --apply            # actually move files
    python scripts/sync_filters.py --reverse          # update filters to match disk
    python scripts/sync_filters.py --project-dir DIR  # specify project directory
"""

import argparse
import os
import re
import shutil
import sys
import xml.etree.ElementTree as ET
from dataclasses import dataclass, field
from pathlib import Path


NS = "{http://schemas.microsoft.com/developer/msbuild/2003}"


@dataclass
class FilterEntry:
    """A file entry parsed from .vcxproj.filters."""
    include_path: str       # Forward-slash path from Include= attr
    filter_path: str        # Backslash path from <Filter> text
    element_tag: str        # "ClInclude" or "ClCompile"
    expected_folder: str    # Derived from filter_path (forward slashes)
    actual_folder: str      # Derived from include_path
    filename: str


@dataclass
class FileMove:
    """A planned file move operation."""
    filename: str
    old_rel_path: str       # Actual disk path (forward slashes)
    new_rel_path: str       # Target relative path (forward slashes)
    include_path: str       # Current Include= attr in vcxproj (may differ from old_rel_path)
    source_exists: bool = True
    target_collision: bool = False


@dataclass
class IncludeUpdate:
    """A #include directive that was updated in a source file."""
    source_file: str
    old_include: str
    new_include: str
    count: int = 0


def find_project_dir(start_dir: Path) -> Path | None:
    """Find the directory containing a .vcxproj file."""
    # Check start_dir itself
    for f in start_dir.iterdir():
        if f.suffix == ".vcxproj":
            return start_dir
    # Check immediate subdirectories
    for d in start_dir.iterdir():
        if d.is_dir():
            for f in d.iterdir():
                if f.suffix == ".vcxproj":
                    return d
    return None


def find_vcxproj_files(project_dir: Path) -> tuple[Path, Path]:
    """Find the .vcxproj and .vcxproj.filters files."""
    vcxproj = None
    filters = None
    for f in project_dir.iterdir():
        if f.suffix == ".vcxproj" and not f.name.endswith(".filters"):
            vcxproj = f
        elif f.name.endswith(".vcxproj.filters"):
            filters = f
    if not vcxproj:
        print(f"ERROR: No .vcxproj file found in {project_dir}", file=sys.stderr)
        sys.exit(1)
    if not filters:
        print(f"ERROR: No .vcxproj.filters file found in {project_dir}", file=sys.stderr)
        sys.exit(1)
    return vcxproj, filters


def parse_top_level_filters(root: ET.Element) -> set[str]:
    """Extract category filter names (those with <Extensions> children).

    Only VS built-in category filters (Source Files, Header Files, Resource Files)
    have <Extensions> elements. User-created filters like 'SourceFiles' do not,
    and should be treated as real folder names, not stripped as prefixes.
    """
    categories = set()
    for item_group in root.findall(f"{NS}ItemGroup"):
        for filt in item_group.findall(f"{NS}Filter"):
            name = filt.get("Include", "")
            has_extensions = filt.find(f"{NS}Extensions") is not None
            if has_extensions and name:
                categories.add(name)
    return categories


def strip_category_prefix(filter_path: str, top_level_filters: set[str]) -> str:
    """Strip the category prefix from a filter path to get the expected folder.

    'Header Files\\Chess' -> 'Chess'
    'Source Files' -> ''
    'Header Files' -> ''
    """
    for prefix in top_level_filters:
        if filter_path == prefix:
            return ""
        if filter_path.startswith(prefix + "\\"):
            remainder = filter_path[len(prefix) + 1:]
            return remainder.replace("\\", "/")
    # Unknown prefix - return whole thing
    return filter_path.replace("\\", "/")


def find_file_on_disk(filename: str, project_dir: Path) -> str:
    """Find where a file actually lives on disk, returning its folder relative to project_dir.

    When VS updates Include paths to match a new filter, the file hasn't actually moved.
    This finds the real location by searching the project directory.
    """
    skip_dirs = {"x64", "old", ".vs", "Debug", "Release", ".artifacts"}
    for root, dirs, files in os.walk(project_dir):
        dirs[:] = [d for d in dirs if d not in skip_dirs]
        if filename in files:
            rel = os.path.relpath(root, project_dir).replace("\\", "/")
            return "" if rel == "." else rel
    return None


def parse_filter_entries(root: ET.Element, top_level_filters: set[str], project_dir: Path) -> list[FilterEntry]:
    """Parse all ClInclude and ClCompile entries from the filters file."""
    entries = []
    for item_group in root.findall(f"{NS}ItemGroup"):
        for tag_suffix in ("ClInclude", "ClCompile"):
            for elem in item_group.findall(f"{NS}{tag_suffix}"):
                include_path = elem.get("Include", "")
                if not include_path:
                    continue
                # Normalize to forward slashes for comparison
                include_norm = include_path.replace("\\", "/")
                filter_elem = elem.find(f"{NS}Filter")
                filter_path = filter_elem.text if filter_elem is not None and filter_elem.text else ""

                expected_folder = strip_category_prefix(filter_path, top_level_filters)
                # Derive filename from include path
                parts = include_norm.rsplit("/", 1)
                filename = parts[-1]

                # Find where the file actually is on disk
                disk_folder = find_file_on_disk(filename, project_dir)
                if disk_folder is None:
                    # File not found on disk — use include path as fallback
                    actual_folder = parts[0] if len(parts) > 1 else ""
                else:
                    actual_folder = disk_folder

                entries.append(FilterEntry(
                    include_path=include_norm,
                    filter_path=filter_path,
                    element_tag=tag_suffix,
                    expected_folder=expected_folder,
                    actual_folder=actual_folder,
                    filename=filename,
                ))
    return entries


def compute_moves(entries: list[FilterEntry], project_dir: Path) -> list[FileMove]:
    """Compare expected vs actual disk folders, return planned moves for mismatches."""
    moves = []
    for entry in entries:
        if entry.expected_folder.lower() != entry.actual_folder.lower():
            new_path = (entry.expected_folder + "/" + entry.filename) if entry.expected_folder else entry.filename
            # The file's real location on disk (actual_folder), not the include path
            old_path = (entry.actual_folder + "/" + entry.filename) if entry.actual_folder else entry.filename
            old_abs = project_dir / old_path
            new_abs = project_dir / new_path

            move = FileMove(
                filename=entry.filename,
                old_rel_path=old_path,
                new_rel_path=new_path,
                include_path=entry.include_path,
                source_exists=old_abs.exists(),
                target_collision=new_abs.exists(),
            )
            moves.append(move)
    return moves


def collect_source_files(project_dir: Path) -> list[Path]:
    """Find all .h and .cpp files in the project directory for include rewriting."""
    source_files = []
    skip_dirs = {"x64", "old", ".vs", "Debug", "Release"}
    for root, dirs, files in os.walk(project_dir):
        # Skip build/legacy directories
        dirs[:] = [d for d in dirs if d not in skip_dirs]
        for f in files:
            if f.endswith((".h", ".hpp", ".cpp", ".c", ".cc")):
                source_files.append(Path(root) / f)
    return source_files


def update_xml_file(file_path: Path, old_path: str, new_path: str) -> bool:
    """Update Include= attributes in a .vcxproj or .vcxproj.filters file.

    Uses string replacement to preserve exact XML formatting.
    Returns True if any changes were made.
    """
    content = file_path.read_text(encoding="utf-8-sig")
    original = content

    # Replace forward-slash version (primary)
    content = content.replace(f'Include="{old_path}"', f'Include="{new_path}"')

    # Replace backslash version (in case some entries use backslashes)
    old_bs = old_path.replace("/", "\\")
    new_bs = new_path.replace("/", "\\")
    if old_bs != old_path:
        content = content.replace(f'Include="{old_bs}"', f'Include="{new_bs}"')

    if content != original:
        file_path.write_text(content, encoding="utf-8-sig")
        return True
    return False


def update_includes_in_file(filepath: Path, replacements: dict[str, str]) -> list[IncludeUpdate]:
    """Apply all include path replacements to a single source file.

    replacements: {old_path: new_path} mapping.
    Returns list of IncludeUpdate for changes that were actually made.
    """
    try:
        content = filepath.read_text(encoding="utf-8-sig")
    except (UnicodeDecodeError, PermissionError):
        return []

    original = content
    updates = []

    for old_path, new_path in replacements.items():
        # Match #include "old/path.h" (with optional whitespace)
        pattern = re.compile(r'#include\s+"' + re.escape(old_path) + r'"')
        new_content = pattern.sub(f'#include "{new_path}"', content)
        if new_content != content:
            count = len(pattern.findall(content))
            updates.append(IncludeUpdate(
                source_file=str(filepath),
                old_include=old_path,
                new_include=new_path,
                count=count,
            ))
            content = new_content

    if content != original:
        filepath.write_text(content, encoding="utf-8-sig")

    return updates


def find_cmakelists(project_dir: Path) -> Path | None:
    """Find CMakeLists.txt in the project dir or its parent (solution root)."""
    # Check parent first (solution root is the common location)
    parent_cmake = project_dir.parent / "CMakeLists.txt"
    if parent_cmake.exists():
        return parent_cmake
    local_cmake = project_dir / "CMakeLists.txt"
    if local_cmake.exists():
        return local_cmake
    return None


def update_cmakelists(cmake_path: Path, old_path: str, new_path: str, project_dir: Path) -> bool:
    """Update source file paths in CMakeLists.txt.

    Handles the ./nina-chess/ prefix that CMakeLists uses relative to the solution root.
    Returns True if any changes were made.
    """
    content = cmake_path.read_text(encoding="utf-8")
    original = content

    # CMakeLists uses paths relative to solution root with ./nina-chess/ prefix
    project_name = project_dir.name
    cmake_old = f"./{project_name}/{old_path}"
    cmake_new = f"./{project_name}/{new_path}"

    content = content.replace(cmake_old, cmake_new)

    # Also try backslash variants just in case
    content = content.replace(cmake_old.replace("/", "\\"), cmake_new.replace("/", "\\"))

    if content != original:
        cmake_path.write_text(content, encoding="utf-8")
        return True
    return False


def remove_empty_dirs(project_dir: Path, folders: set[str]):
    """Remove directories that became empty after moves."""
    for folder in sorted(folders, key=lambda f: f.count("/"), reverse=True):
        dir_path = project_dir / folder
        if dir_path.is_dir():
            try:
                remaining = list(dir_path.iterdir())
                if not remaining:
                    dir_path.rmdir()
                    print(f"  Removed empty directory: {folder}/")
            except OSError:
                pass


def print_plan(moves: list[FileMove], entries: list[FilterEntry], cmake_path: Path | None = None, project_dir: Path | None = None):
    """Print dry-run summary of planned changes."""
    matched = sum(1 for e in entries if e.expected_folder.lower() == e.actual_folder.lower())
    print(f"\n{'=' * 60}")
    print(f"  Sync Filters -> Disk  (DRY RUN)")
    print(f"{'=' * 60}")
    print(f"\n  Files already in correct location: {matched}")
    print(f"  Files to move: {len(moves)}")

    if not moves:
        print("\n  Everything is in sync!")
        return

    valid_moves = [m for m in moves if m.source_exists and not m.target_collision]
    skip_missing = [m for m in moves if not m.source_exists]
    skip_collision = [m for m in moves if m.target_collision]

    if valid_moves:
        print(f"\n  Planned moves ({len(valid_moves)}):")
        for m in valid_moves:
            print(f"    {m.old_rel_path}  ->  {m.new_rel_path}")

    # Check which moves would affect CMakeLists.txt
    if cmake_path and project_dir and valid_moves:
        cmake_content = cmake_path.read_text(encoding="utf-8")
        project_name = project_dir.name
        cmake_affected = [m for m in valid_moves
                          if f"./{project_name}/{m.old_rel_path}" in cmake_content
                          or f"./{project_name}/{m.include_path}" in cmake_content]
        if cmake_affected:
            print(f"\n  CMakeLists.txt will also be updated for:")
            for m in cmake_affected:
                print(f"    {m.old_rel_path}")

    if skip_missing:
        print(f"\n  WARNING: Files in filters but missing on disk ({len(skip_missing)}):")
        for m in skip_missing:
            print(f"    {m.old_rel_path}  (not found)")

    if skip_collision:
        print(f"\n  ERROR: Target collision ({len(skip_collision)}):")
        for m in skip_collision:
            print(f"    {m.old_rel_path}  ->  {m.new_rel_path}  (target exists!)")

    print(f"\n  Run with --apply to execute these changes.")
    print(f"  TIP: Commit or stash your work first so you can revert with git.")


def apply_moves(
    moves: list[FileMove],
    project_dir: Path,
    vcxproj_path: Path,
    filters_path: Path,
    cmake_path: Path | None,
):
    """Execute file moves and update all references."""
    valid_moves = [m for m in moves if m.source_exists and not m.target_collision]
    if not valid_moves:
        print("\nNo valid moves to apply.")
        return

    print(f"\n{'=' * 60}")
    print(f"  Applying {len(valid_moves)} file move(s)...")
    print(f"{'=' * 60}")

    # Build replacement maps:
    # - include_replacements: old_include_path -> new_path (for #include directives and XML that VS hasn't updated)
    # - xml_replacements: current_include_attr -> new_path (for XML that VS already updated to match filter)
    include_replacements: dict[str, str] = {}
    xml_replacements: dict[str, str] = {}
    cmake_replacements: dict[str, str] = {}
    source_dirs_affected: set[str] = set()

    for move in valid_moves:
        # The actual disk path (where #include directives point)
        include_replacements[move.old_rel_path] = move.new_rel_path
        # The current Include= attr in XML (VS may have already changed this)
        if move.include_path != move.new_rel_path:
            xml_replacements[move.include_path] = move.new_rel_path
        # Also replace old disk path in XML if different from include_path
        if move.old_rel_path != move.include_path and move.old_rel_path != move.new_rel_path:
            xml_replacements[move.old_rel_path] = move.new_rel_path
        # For CMakeLists, replace both old disk path and current include path
        cmake_replacements[move.old_rel_path] = move.new_rel_path
        if move.include_path != move.old_rel_path:
            cmake_replacements[move.include_path] = move.new_rel_path

        # Track source directories for empty dir cleanup
        parts = move.old_rel_path.rsplit("/", 1)
        if len(parts) > 1:
            source_dirs_affected.add(parts[0])

    # Step 1: Move files on disk
    print("\n  Moving files:")
    for move in valid_moves:
        old_abs = project_dir / move.old_rel_path
        new_abs = project_dir / move.new_rel_path

        # Create target directory
        new_abs.parent.mkdir(parents=True, exist_ok=True)

        # Move the file
        shutil.move(str(old_abs), str(new_abs))
        print(f"    {move.old_rel_path}  ->  {move.new_rel_path}")

    # Step 2: Update .vcxproj and .vcxproj.filters
    print("\n  Updating project files:")
    for old_path, new_path in xml_replacements.items():
        changed_proj = update_xml_file(vcxproj_path, old_path, new_path)
        changed_filt = update_xml_file(filters_path, old_path, new_path)
        if changed_proj:
            print(f"    {vcxproj_path.name}: {old_path} -> {new_path}")
        if changed_filt:
            print(f"    {filters_path.name}: {old_path} -> {new_path}")

    # Step 3: Update CMakeLists.txt
    if cmake_path:
        cmake_updated = False
        for old_path, new_path in cmake_replacements.items():
            if update_cmakelists(cmake_path, old_path, new_path, project_dir):
                cmake_updated = True
                print(f"    {cmake_path.name}: {old_path} -> {new_path}")
        if cmake_updated:
            print(f"\n  Updated {cmake_path.name}")
        else:
            print(f"\n  {cmake_path.name}: no changes needed")

    # Step 4: Update #include directives in all source files
    print("\n  Updating #include directives:")
    source_files = collect_source_files(project_dir)
    total_updates = 0
    for src_file in source_files:
        updates = update_includes_in_file(src_file, include_replacements)
        for u in updates:
            rel = os.path.relpath(u.source_file, project_dir)
            print(f"    {rel}: \"{u.old_include}\" -> \"{u.new_include}\" ({u.count}x)")
            total_updates += u.count

    if total_updates == 0:
        print("    (no #include directives needed updating)")

    # Step 5: Clean up empty directories
    if source_dirs_affected:
        print("\n  Cleaning up:")
        remove_empty_dirs(project_dir, source_dirs_affected)

    # Summary
    print(f"\n{'=' * 60}")
    print(f"  Done! {len(valid_moves)} file(s) moved, {total_updates} #include(s) updated.")
    print(f"  TIP: Run 'git diff' to review all changes.")
    print(f"{'=' * 60}")


def run_reverse_sync(
    entries: list[FilterEntry],
    project_dir: Path,
    top_level_filters: set[str],
    filters_path: Path,
    apply: bool,
):
    """Update .vcxproj.filters to match actual disk locations (reverse direction)."""
    mismatched = [e for e in entries if e.expected_folder.lower() != e.actual_folder.lower()]

    print(f"\n{'=' * 60}")
    print(f"  Sync Disk -> Filters  {'(DRY RUN)' if not apply else ''}")
    print(f"{'=' * 60}")

    matched = len(entries) - len(mismatched)
    print(f"\n  Files already in sync: {matched}")
    print(f"  Filters to update: {len(mismatched)}")

    if not mismatched:
        print("\n  Everything is in sync!")
        return

    for entry in mismatched:
        # Derive the new filter path from the actual disk location
        if entry.actual_folder:
            # Find which top-level category this belongs to
            category = ""
            for prefix in top_level_filters:
                ext_match = entry.filename.rsplit(".", 1)[-1].lower() if "." in entry.filename else ""
                if ext_match in ("h", "hpp", "hxx", "hh") and "Header" in prefix:
                    category = prefix
                    break
                elif ext_match in ("cpp", "c", "cc", "cxx") and "Source" in prefix:
                    category = prefix
                    break
            if not category:
                category = "Header Files" if entry.element_tag == "ClInclude" else "Source Files"
            new_filter = category + "\\" + entry.actual_folder.replace("/", "\\")
        else:
            new_filter = "Header Files" if entry.element_tag == "ClInclude" else "Source Files"

        print(f"    {entry.include_path}: \"{entry.filter_path}\" -> \"{new_filter}\"")

        if apply:
            # Update the <Filter> text in the .filters file via string replacement
            content = filters_path.read_text(encoding="utf-8-sig")
            # Find the specific entry and update its filter text
            # We need to match the Include path and its following Filter element
            old_pattern = f'Include="{entry.include_path}">\n      <Filter>{entry.filter_path}</Filter>'
            new_pattern = f'Include="{entry.include_path}">\n      <Filter>{new_filter}</Filter>'
            content = content.replace(old_pattern, new_pattern)
            # Also try backslash version of include path
            old_bs = entry.include_path.replace("/", "\\")
            old_pattern_bs = f'Include="{old_bs}">\n      <Filter>{entry.filter_path}</Filter>'
            new_pattern_bs = f'Include="{old_bs}">\n      <Filter>{new_filter}</Filter>'
            content = content.replace(old_pattern_bs, new_pattern_bs)
            filters_path.write_text(content, encoding="utf-8-sig")

    if not apply:
        print(f"\n  Run with --apply --reverse to execute these changes.")


def main():
    parser = argparse.ArgumentParser(
        description="Synchronize VS .vcxproj.filters structure to disk folder layout.",
    )
    parser.add_argument(
        "--apply",
        action="store_true",
        help="Actually perform moves and edits (default: dry-run)",
    )
    parser.add_argument(
        "--project-dir",
        type=Path,
        default=None,
        help="Path to directory containing .vcxproj (default: auto-detect)",
    )
    parser.add_argument(
        "--reverse",
        action="store_true",
        help="Update .vcxproj.filters to match disk layout (instead of moving files)",
    )
    args = parser.parse_args()

    # Find project directory
    if args.project_dir:
        project_dir = args.project_dir.resolve()
    else:
        cwd = Path.cwd()
        project_dir = find_project_dir(cwd)
        if not project_dir:
            print("ERROR: Could not find .vcxproj file. Use --project-dir.", file=sys.stderr)
            sys.exit(1)

    print(f"Project directory: {project_dir}")

    # Find project files
    vcxproj_path, filters_path = find_vcxproj_files(project_dir)
    print(f"Project file:  {vcxproj_path.name}")
    print(f"Filters file:  {filters_path.name}")

    # Parse filters
    tree = ET.parse(filters_path)
    root = tree.getroot()

    top_level_filters = parse_top_level_filters(root)
    print(f"Top-level filters: {', '.join(sorted(top_level_filters))}")

    entries = parse_filter_entries(root, top_level_filters, project_dir)
    print(f"Files in project: {len(entries)}")

    # Find CMakeLists.txt
    cmake_path = find_cmakelists(project_dir)
    if cmake_path:
        print(f"CMakeLists:    {cmake_path}")

    if args.reverse:
        run_reverse_sync(entries, project_dir, top_level_filters, filters_path, args.apply)
        return

    # Forward sync: move files to match filters
    moves = compute_moves(entries, project_dir)

    if args.apply:
        if not moves:
            print("\nEverything is already in sync. Nothing to do.")
            return
        apply_moves(moves, project_dir, vcxproj_path, filters_path, cmake_path)
    else:
        print_plan(moves, entries, cmake_path, project_dir)


if __name__ == "__main__":
    main()
