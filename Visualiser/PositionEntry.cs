using System.IO;
using System.Numerics;

namespace Visualiser;

public class PositionEntry
{
	public const int NumFeatures = 25;
	public const int EntrySize = NumFeatures * 8 + 4 * 4; // 216 bytes

	// Feature indices - white pieces
	public const int WhitePawns = 0;
	public const int WhiteKnights = 1;
	public const int WhiteBishops = 2;
	public const int WhiteRooks = 3;
	public const int WhiteQueens = 4;
	public const int WhiteKing = 5;

	// Feature indices - black pieces
	public const int BlackPawns = 6;
	public const int BlackKnights = 7;
	public const int BlackBishops = 8;
	public const int BlackRooks = 9;
	public const int BlackQueens = 10;
	public const int BlackKing = 11;

	// Feature indices - board state
	public const int EnPassantIndex = 12;
	public const int CastlingIndex = 13;

	// Feature indices - move generation data
	public const int PawnMovesIndex = 14;
	public const int KnightMovesIndex = 15;
	public const int BishopMovesIndex = 16;
	public const int RookMovesIndex = 17;
	public const int QueenMovesIndex = 18;
	public const int KingMovesIndex = 19;
	public const int PinmaskIndex = 20;
	public const int CheckmaskIndex = 21;
	public const int PinnersIndex = 22;
	public const int CheckersIndex = 23;
	public const int AttackedSquaresIndex = 24;

	// Move encoding bit layout
	private const int FromOffset = 0;
	private const uint FromMask = 0x3F;
	private const int ToOffset = 6;
	private const uint ToMask = 0x3F << ToOffset;
	private const int PieceOffset = 12;
	private const uint PieceMask = 0xFu << PieceOffset;
	private const int PromotionOffset = 16;
	private const uint PromotionMask = 0xFu << PromotionOffset;
	private const int MoveTypeOffset = 20;
	private const uint MoveTypeMask = 0xFu << MoveTypeOffset;

	// Data fields
	public ulong[] Features { get; } = new ulong[NumFeatures];
	public int SearchScore { get; set; }
	public uint EncodedMove { get; set; }
	public uint Result { get; set; }
	public uint SideToMove { get; set; }

	// Decoded move properties
	public int MoveFrom => (int)(EncodedMove & FromMask);
	public int MoveTo => (int)((EncodedMove & ToMask) >> ToOffset);
	public int MovePiece => (int)((EncodedMove & PieceMask) >> PieceOffset);
	public int MovePromotion => (int)((EncodedMove & PromotionMask) >> PromotionOffset);
	public int MoveType => (int)((EncodedMove & MoveTypeMask) >> MoveTypeOffset);

	/// <summary>
	/// Converts a bit index (0-63) to a chess square name.
	/// Engine mapping: bit 0 = h1, bit 7 = a1, bit 56 = h8, bit 63 = a8.
	/// </summary>
	public static string SquareName(int bitIndex)
	{
		int row = bitIndex / 8;
		int col = bitIndex % 8;
		char file = (char)('h' - col);
		char rank = (char)('1' + row);
		return $"{file}{rank}";
	}

	public static int BitScanForward(ulong value) =>
		value == 0 ? -1 : BitOperations.TrailingZeroCount(value);

	public string MoveToUci()
	{
		string uci = SquareName(MoveFrom) + SquareName(MoveTo);
		string promo = MovePromotion switch
		{
			1 => "n",
			2 => "b",
			3 => "r",
			4 => "q",
			_ => ""
		};
		return uci + promo;
	}

	public static string PieceTypeName(int pieceType) => pieceType switch
	{
		0 => "Pawn",
		1 => "Knight",
		2 => "Bishop",
		3 => "Rook",
		4 => "Queen",
		5 => "King",
		_ => "None"
	};

	public static string MoveTypeName(int moveType) => moveType switch
	{
		0 => "Normal",
		1 => "Capture",
		2 => "Double Pawn Push",
		3 => "En Passant",
		4 => "Kingside Castle",
		5 => "Queenside Castle",
		6 => "Promote Queen",
		7 => "Promote Rook",
		8 => "Promote Bishop",
		9 => "Promote Knight",
		10 => "Promote Queen + Capture",
		11 => "Promote Rook + Capture",
		12 => "Promote Bishop + Capture",
		13 => "Promote Knight + Capture",
		_ => "Unknown"
	};

	public static string ResultName(uint result) => result switch
	{
		0 => "Black Win",
		1 => "Draw",
		2 => "White Win",
		_ => "Unknown"
	};

	public string FormatScore()
	{
		return SearchScore switch
		{
			1000 => "Win (mate)",
			-1000 => "Loss (mate)",
			> 1000 => $"+INF ({SearchScore})",
			< -1000 => $"-INF ({SearchScore})",
			>= 0 => $"+{SearchScore} cp",
			_ => $"{SearchScore} cp"
		};
	}

	public string CastlingRights()
	{
		ulong castling = Features[CastlingIndex];
		string rights = "";
		if ((castling & 1) != 0) rights += "K";
		if ((castling & 2) != 0) rights += "Q";
		if ((castling & 4) != 0) rights += "k";
		if ((castling & 8) != 0) rights += "q";
		return rights.Length > 0 ? rights : "-";
	}

	public static List<PositionEntry> LoadFromFile(string path)
	{
		var entries = new List<PositionEntry>();
		using var stream = File.OpenRead(path);
		using var reader = new BinaryReader(stream);

		long fileSize = stream.Length;
		if (fileSize % EntrySize != 0)
			throw new InvalidDataException(
				$"File size ({fileSize} bytes) is not a multiple of entry size ({EntrySize} bytes). " +
				$"Remainder: {fileSize % EntrySize} bytes.");

		int numEntries = (int)(fileSize / EntrySize);
		entries.Capacity = numEntries;

		for (int i = 0; i < numEntries; i++)
		{
			var entry = new PositionEntry();
			for (int f = 0; f < NumFeatures; f++)
				entry.Features[f] = reader.ReadUInt64();
			entry.SearchScore = reader.ReadInt32();
			entry.EncodedMove = reader.ReadUInt32();
			entry.Result = reader.ReadUInt32();
			entry.SideToMove = reader.ReadUInt32();
			entries.Add(entry);
		}

		return entries;
	}
}
