namespace BookGen;

/// <summary>
/// Binary format for a single book position (112 bytes).
/// Layout matches the C++ BookPosition struct in book.h.
/// </summary>
public struct BookPosition
{
	public const int Size = 12 * 8 + 8 + 4 + 4; // 112 bytes

	// 12 piece bitboards: White {P,N,B,R,Q,K} then Black {P,N,B,R,Q,K}
	public ulong[] Pieces; // length 12
	public ulong EnPassantSquare;
	public uint CastlingPermissions;
	public uint SideToMove; // 0 = white, 1 = black

	public BookPosition()
	{
		Pieces = new ulong[12];
		EnPassantSquare = 0;
		CastlingPermissions = 0;
		SideToMove = 0;
	}

	public void WriteTo(BinaryWriter writer)
	{
		for (int pieceIndex = 0; pieceIndex < 12; pieceIndex++)
			writer.Write(Pieces[pieceIndex]);
		writer.Write(EnPassantSquare);
		writer.Write(CastlingPermissions);
		writer.Write(SideToMove);
	}

	public static BookPosition ReadFrom(BinaryReader reader)
	{
		var position = new BookPosition();
		for (int pieceIndex = 0; pieceIndex < 12; pieceIndex++)
			position.Pieces[pieceIndex] = reader.ReadUInt64();
		position.EnPassantSquare = reader.ReadUInt64();
		position.CastlingPermissions = reader.ReadUInt32();
		position.SideToMove = reader.ReadUInt32();
		return position;
	}

	public PositionKey ToKey()
	{
		return new PositionKey(
			Pieces[0], Pieces[1], Pieces[2], Pieces[3], Pieces[4], Pieces[5],
			Pieces[6], Pieces[7], Pieces[8], Pieces[9], Pieces[10], Pieces[11],
			EnPassantSquare, CastlingPermissions, SideToMove);
	}
}

/// <summary>
/// Deduplication key for positions. Record struct auto-generates Equals/GetHashCode.
/// </summary>
public readonly record struct PositionKey(
	ulong WhitePawns, ulong WhiteKnights, ulong WhiteBishops,
	ulong WhiteRooks, ulong WhiteQueens, ulong WhiteKing,
	ulong BlackPawns, ulong BlackKnights, ulong BlackBishops,
	ulong BlackRooks, ulong BlackQueens, ulong BlackKing,
	ulong EnPassant, uint Castling, uint SideToMove);
