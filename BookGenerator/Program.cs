namespace BookGen;

class Program
{
	private const int NumSlots = 17; // 16 regular plies + 1 special
	private const int SpecialSlot = 17; // 1-indexed
	private const int HeaderSize = NumSlots * 8; // 136 bytes

	static int Main(string[] args)
	{
		string inputFile = "";
		string outputFile = "book.bin";
		int minPly = 2;
		int maxPly = 8;
		bool appendMode = false;
		bool specialMode = false;

		for (int argIndex = 0; argIndex < args.Length; argIndex++)
		{
			switch (args[argIndex])
			{
				case "--append": appendMode = true; break;
				case "--special": specialMode = true; break;
				case "--input" when argIndex + 1 < args.Length: inputFile = args[++argIndex]; break;
				case "--output" when argIndex + 1 < args.Length: outputFile = args[++argIndex]; break;
				case "--min-ply" when argIndex + 1 < args.Length: minPly = int.Parse(args[++argIndex]); break;
				case "--max-ply" when argIndex + 1 < args.Length: maxPly = int.Parse(args[++argIndex]); break;
			}
		}

		if (string.IsNullOrEmpty(inputFile))
		{
			Console.Error.WriteLine("Usage: BookGen --input <pgn> [--output book.bin] [--min-ply 2] [--max-ply 8] [--append] [--special]");
			Console.Error.WriteLine();
			Console.Error.WriteLine("  --special   Store only the final position of each game in the special slot (ply 17)");
			Console.Error.WriteLine("  --append    Merge into existing book.bin instead of overwriting");
			return 1;
		}

		if (!specialMode && (minPly < 1 || maxPly > 16 || minPly > maxPly))
		{
			Console.Error.WriteLine("Ply range must be within 1-16 and min <= max.");
			return 1;
		}

		Console.WriteLine($"Input:    {inputFile}");
		Console.WriteLine($"Output:   {outputFile}");
		Console.WriteLine($"Mode:     {(specialMode ? "special (ending positions -> slot 17)" : $"normal (ply {minPly}-{maxPly})")}");
		Console.WriteLine($"Write:    {(appendMode ? "append" : "overwrite")}");
		Console.WriteLine();

		// Load existing book if appending
		var positionsBySlot = new Dictionary<int, List<BookPosition>>();
		var seenBySlot = new Dictionary<int, HashSet<PositionKey>>();
		int existingPositionCount = 0;

		if (appendMode && File.Exists(outputFile))
		{
			Console.WriteLine("Loading existing book...");
			existingPositionCount = LoadExistingBook(outputFile, positionsBySlot, seenBySlot);
			Console.WriteLine($"  Loaded {existingPositionCount} existing positions.");
		}

		// Ensure slots exist for the plies we'll write to
		if (specialMode)
		{
			EnsureSlotExists(positionsBySlot, seenBySlot, SpecialSlot);
		}
		else
		{
			for (int ply = minPly; ply <= maxPly; ply++)
				EnsureSlotExists(positionsBySlot, seenBySlot, ply);
		}

		// Stream PGN and collect positions
		Console.WriteLine("Processing PGN...");

		int gamesProcessed = 0;
		int gamesSkipped = 0;
		int newPositions = 0;
		int duplicatesSkipped = 0;

		foreach (var moves in PgnParser.StreamGames(inputFile))
		{
			try
			{
				if (specialMode)
				{
					ProcessGameSpecialMode(moves, positionsBySlot, seenBySlot, ref newPositions, ref duplicatesSkipped);
				}
				else
				{
					ProcessGameNormalMode(moves, minPly, maxPly, positionsBySlot, seenBySlot, ref newPositions, ref duplicatesSkipped);
				}
				gamesProcessed++;

				if (gamesProcessed % 100000 == 0)
					Console.WriteLine($"  {gamesProcessed} games processed, {newPositions} new positions...");
			}
			catch (Exception exception)
			{
				gamesSkipped++;
				if (gamesSkipped <= 5)
					Console.Error.WriteLine($"  Skipping game {gamesProcessed + gamesSkipped}: {exception.Message}");
				else if (gamesSkipped == 6)
					Console.Error.WriteLine("  (suppressing further warnings)");
			}
		}

		int totalPositions = existingPositionCount + newPositions;

		Console.WriteLine($"Processed: {gamesProcessed} games, skipped: {gamesSkipped}");
		Console.WriteLine($"New positions: {newPositions}, duplicates skipped: {duplicatesSkipped}");
		Console.WriteLine($"Total positions in book: {totalPositions}");

		for (int slot = 1; slot <= NumSlots; slot++)
		{
			if (positionsBySlot.TryGetValue(slot, out var positions) && positions.Count > 0)
			{
				string label = slot == SpecialSlot ? "Special" : $"Ply {slot,2}";
				Console.WriteLine($"  {label}: {positions.Count} positions");
			}
		}

		// Write book
		Console.WriteLine($"\nWriting {outputFile}...");
		WriteBook(outputFile, positionsBySlot);

		var fileInfo = new FileInfo(outputFile);
		Console.WriteLine($"Done. File size: {fileInfo.Length:N0} bytes");
		return 0;
	}

	static void ProcessGameNormalMode(List<string> moves, int minPly, int maxPly,
		Dictionary<int, List<BookPosition>> positionsBySlot, Dictionary<int, HashSet<PositionKey>> seenBySlot,
		ref int newPositions, ref int duplicatesSkipped)
	{
		var board = new ChessBoard();
		int movesToProcess = Math.Min(moves.Count, maxPly);

		for (int moveIndex = 0; moveIndex < movesToProcess; moveIndex++)
		{
			board.MakeMoveSan(moves[moveIndex]);
			int ply = moveIndex + 1;

			if (ply >= minPly && ply <= maxPly)
			{
				var bookPosition = board.ToBookPosition();
				var positionKey = bookPosition.ToKey();

				if (seenBySlot[ply].Add(positionKey))
				{
					positionsBySlot[ply].Add(bookPosition);
					newPositions++;
				}
				else
				{
					duplicatesSkipped++;
				}
			}
		}
	}

	static void ProcessGameSpecialMode(List<string> moves,
		Dictionary<int, List<BookPosition>> positionsBySlot, Dictionary<int, HashSet<PositionKey>> seenBySlot,
		ref int newPositions, ref int duplicatesSkipped)
	{
		if (moves.Count == 0) return;

		var board = new ChessBoard();
		for (int moveIndex = 0; moveIndex < moves.Count; moveIndex++)
			board.MakeMoveSan(moves[moveIndex]);

		var bookPosition = board.ToBookPosition();
		var positionKey = bookPosition.ToKey();

		if (seenBySlot[SpecialSlot].Add(positionKey))
		{
			positionsBySlot[SpecialSlot].Add(bookPosition);
			newPositions++;
		}
		else
		{
			duplicatesSkipped++;
		}
	}

	static void EnsureSlotExists(Dictionary<int, List<BookPosition>> positionsBySlot, Dictionary<int, HashSet<PositionKey>> seenBySlot, int slot)
	{
		positionsBySlot.TryAdd(slot, []);
		seenBySlot.TryAdd(slot, []);
	}

	static int LoadExistingBook(string path, Dictionary<int, List<BookPosition>> positionsBySlot, Dictionary<int, HashSet<PositionKey>> seenBySlot)
	{
		using var reader = new BinaryReader(File.OpenRead(path));

		ulong[] offsets = new ulong[NumSlots];
		for (int slotIndex = 0; slotIndex < NumSlots; slotIndex++)
			offsets[slotIndex] = reader.ReadUInt64();

		long fileSize = reader.BaseStream.Length;
		int totalLoaded = 0;

		for (int slotIndex = 0; slotIndex < NumSlots; slotIndex++)
		{
			int slot = slotIndex + 1;
			ulong startByte = offsets[slotIndex];
			ulong endByte = (slotIndex + 1 < NumSlots) ? offsets[slotIndex + 1] : (ulong)fileSize;
			int positionCount = (int)((endByte - startByte) / (ulong)BookPosition.Size);

			if (positionCount == 0) continue;

			reader.BaseStream.Seek((long)startByte, SeekOrigin.Begin);
			var positions = new List<BookPosition>(positionCount);
			var seen = new HashSet<PositionKey>();

			for (int positionIndex = 0; positionIndex < positionCount; positionIndex++)
			{
				var position = BookPosition.ReadFrom(reader);
				positions.Add(position);
				seen.Add(position.ToKey());
			}

			positionsBySlot[slot] = positions;
			seenBySlot[slot] = seen;
			totalLoaded += positionCount;
		}

		return totalLoaded;
	}

	static void WriteBook(string path, Dictionary<int, List<BookPosition>> positionsBySlot)
	{
		using var writer = new BinaryWriter(File.Create(path));

		// Compute byte offsets for each slot
		ulong currentOffset = (ulong)HeaderSize;
		ulong[] offsets = new ulong[NumSlots];

		for (int slot = 1; slot <= NumSlots; slot++)
		{
			offsets[slot - 1] = currentOffset;
			if (positionsBySlot.TryGetValue(slot, out var positions))
				currentOffset += (ulong)(positions.Count * BookPosition.Size);
		}

		// Write header
		for (int headerIndex = 0; headerIndex < NumSlots; headerIndex++)
			writer.Write(offsets[headerIndex]);

		// Write positions grouped by slot
		for (int slot = 1; slot <= NumSlots; slot++)
		{
			if (positionsBySlot.TryGetValue(slot, out var positions))
			{
				foreach (var position in positions)
					position.WriteTo(writer);
			}
		}
	}
}
