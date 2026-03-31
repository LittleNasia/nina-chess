"""
Removes duplicate positions and shuffles a game generation data file.
Deduplication is based on the 25 bitboard features (first 200 bytes of each entry).
For duplicates, SearchScore and Result are averaged across all occurrences.

Entry layout (216 bytes):
  [0..199]   Features    25 x uint64  (dedup key)
  [200..203] SearchScore int32        (averaged)
  [204..207] BestMove    uint32       (kept from first)
  [208..211] Result      uint32       (averaged)
  [212..215] SideToMove  uint32       (kept from first)

Usage: python shuffle_data.py input.bin output.bin
       python shuffle_data.py data.bin  (overwrites in place)
"""

import sys
import struct
import random

ENTRY_SIZE = 216
FEATURES_SIZE = 200  # 25 * 8 bytes, used as dedup key

SEARCH_SCORE_OFFSET = 200
BEST_MOVE_OFFSET = 204
RESULT_OFFSET = 208
SIDE_TO_MOVE_OFFSET = 212


def parse_entry(data, offset):
    features = data[offset : offset + FEATURES_SIZE]
    search_score = struct.unpack_from("<i", data, offset + SEARCH_SCORE_OFFSET)[0]
    best_move = struct.unpack_from("<I", data, offset + BEST_MOVE_OFFSET)[0]
    result = struct.unpack_from("<I", data, offset + RESULT_OFFSET)[0]
    side_to_move = struct.unpack_from("<I", data, offset + SIDE_TO_MOVE_OFFSET)[0]
    return features, search_score, best_move, result, side_to_move


def build_entry(features, search_score, best_move, result, side_to_move):
    return features + struct.pack("<iIII", search_score, best_move, result, side_to_move)


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <input.bin> [output.bin]")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2] if len(sys.argv) > 2 else input_path

    with open(input_path, "rb") as file:
        data = file.read()

    total_entries = len(data) // ENTRY_SIZE
    if len(data) % ENTRY_SIZE != 0:
        print(f"Warning: file size {len(data)} is not a multiple of {ENTRY_SIZE}, truncating")

    print(f"Loaded {total_entries} entries ({len(data) / (1024 * 1024):.1f} MB)")

    # Group by features, accumulate scores for averaging
    # For each unique feature set, store: (score_sum, result_sum, count, best_move, side_to_move)
    groups = {}
    for entry_index in range(total_entries):
        offset = entry_index * ENTRY_SIZE
        features, search_score, best_move, result, side_to_move = parse_entry(data, offset)

        if features in groups:
            score_sum, result_sum, count, first_best_move, first_side_to_move = groups[features]
            groups[features] = (score_sum + search_score, result_sum + result, count + 1, first_best_move, first_side_to_move)
        else:
            groups[features] = (search_score, result, 1, best_move, side_to_move)

    duplicates_removed = total_entries - len(groups)
    print(f"Removed {duplicates_removed} duplicates, {len(groups)} unique entries remain")

    # Build averaged entries
    entries = []
    for features, (score_sum, result_sum, count, best_move, side_to_move) in groups.items():
        averaged_score = round(score_sum / count)
        averaged_result = round(result_sum / count)
        entries.append(build_entry(features, averaged_score, best_move, averaged_result, side_to_move))

    # Shuffle
    random.shuffle(entries)
    print("Shuffled")

    # Write
    with open(output_path, "wb") as file:
        for entry in entries:
            file.write(entry)

    output_size = len(entries) * ENTRY_SIZE
    print(f"Written to {output_path} ({output_size / (1024 * 1024):.1f} MB)")


if __name__ == "__main__":
    main()
