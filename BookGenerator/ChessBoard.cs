namespace BookGen;

/// <summary>
/// Minimal chess board for replaying PGN games.
/// Internal square indexing: 0=a1, 1=b1, ..., 7=h1, 8=a2, ..., 63=h8.
/// </summary>
public class ChessBoard
{
	// Piece encoding
	private const int Empty = 0;
	private const int WhitePawn = 1, WhiteKnight = 2, WhiteBishop = 3;
	private const int WhiteRook = 4, WhiteQueen = 5, WhiteKing = 6;
	private const int BlackPawn = 7, BlackKnight = 8, BlackBishop = 9;
	private const int BlackRook = 10, BlackQueen = 11, BlackKing = 12;

	// Piece type indices (shared between white/black)
	private const int PawnType = 0, KnightType = 1, BishopType = 2;
	private const int RookType = 3, QueenType = 4, KingType = 5;

	// Notable square indices
	private const int WhiteKingStartSquare = 4;   // e1
	private const int BlackKingStartSquare = 60;   // e8
	private const int WhiteKingsideRookSquare = 7;  // h1
	private const int WhiteQueensideRookSquare = 0; // a1
	private const int BlackKingsideRookSquare = 63; // h8
	private const int BlackQueensideRookSquare = 56; // a8

	private readonly int[] _squares = new int[64];
	private bool _whiteToMove = true;
	private bool _whiteKingsideCastle = true, _whiteQueensideCastle = true;
	private bool _blackKingsideCastle = true, _blackQueensideCastle = true;
	private int _enPassantSquare = -1; // en passant target square, or -1

	public ChessBoard()
	{
		SetStartingPosition();
	}

	private void SetStartingPosition()
	{
		Array.Clear(_squares);
		_squares[0] = WhiteRook; _squares[1] = WhiteKnight; _squares[2] = WhiteBishop; _squares[3] = WhiteQueen;
		_squares[4] = WhiteKing; _squares[5] = WhiteBishop; _squares[6] = WhiteKnight; _squares[7] = WhiteRook;
		for (int squareIndex = 8; squareIndex < 16; squareIndex++) _squares[squareIndex] = WhitePawn;
		for (int squareIndex = 48; squareIndex < 56; squareIndex++) _squares[squareIndex] = BlackPawn;
		_squares[56] = BlackRook; _squares[57] = BlackKnight; _squares[58] = BlackBishop; _squares[59] = BlackQueen;
		_squares[60] = BlackKing; _squares[61] = BlackBishop; _squares[62] = BlackKnight; _squares[63] = BlackRook;
	}

	public void MakeMoveSan(string rawSan)
	{
		string san = rawSan.TrimEnd('+', '#', '!', '?');
		while (san.Length > 0 && (san[^1] == '!' || san[^1] == '?'))
			san = san[..^1];

		if (san is "O-O" or "0-0") { CastleKingside(); return; }
		if (san is "O-O-O" or "0-0-0") { CastleQueenside(); return; }

		// Handle promotion (e.g. "e8=Q", "exd1=N")
		int promotionPieceType = -1;
		int equalsIndex = san.IndexOf('=');
		if (equalsIndex >= 0)
		{
			promotionPieceType = CharToPieceType(san[equalsIndex + 1]);
			san = san[..equalsIndex];
		}

		// Target square is always the last 2 characters
		int targetFile = san[^2] - 'a';
		int targetRank = san[^1] - '1';
		int targetSquare = targetRank * 8 + targetFile;
		san = san[..^2];

		// Remove capture marker
		san = san.Replace("x", "");

		// Piece type
		int pieceType = PawnType;
		if (san.Length > 0 && char.IsUpper(san[0]))
		{
			pieceType = CharToPieceType(san[0]);
			san = san[1..];
		}

		// Remaining characters are disambiguation (file letter and/or rank digit)
		int disambiguationFile = -1, disambiguationRank = -1;
		foreach (char character in san)
		{
			if (character >= 'a' && character <= 'h') disambiguationFile = character - 'a';
			else if (character >= '1' && character <= '8') disambiguationRank = character - '1';
		}

		int fromSquare = FindFromSquare(pieceType, targetSquare, disambiguationFile, disambiguationRank);
		ExecuteMove(fromSquare, targetSquare, promotionPieceType);
	}

	/// <summary>
	/// Convert board state to the engine's binary BookPosition format.
	/// Engine bit mapping: bit index = rank * 8 + (7 - file).
	/// </summary>
	public BookPosition ToBookPosition()
	{
		var bookPosition = new BookPosition();

		for (int squareIndex = 0; squareIndex < 64; squareIndex++)
		{
			int piece = _squares[squareIndex];
			if (piece == Empty) continue;

			int rank = squareIndex / 8;
			int file = squareIndex % 8;
			int engineBitIndex = rank * 8 + (7 - file);
			ulong bitMask = 1UL << engineBitIndex;

			int pieceArrayIndex = piece <= WhiteKing ? piece - 1 : piece - BlackPawn + 6;
			bookPosition.Pieces[pieceArrayIndex] |= bitMask;
		}

		if (_enPassantSquare >= 0)
		{
			int rank = _enPassantSquare / 8;
			int file = _enPassantSquare % 8;
			bookPosition.EnPassantSquare = 1UL << (rank * 8 + (7 - file));
		}

		uint castlingPermissions = 0;
		if (_whiteKingsideCastle) castlingPermissions |= 1;
		if (_whiteQueensideCastle) castlingPermissions |= 2;
		if (_blackKingsideCastle) castlingPermissions |= 4;
		if (_blackQueensideCastle) castlingPermissions |= 8;
		bookPosition.CastlingPermissions = castlingPermissions;

		bookPosition.SideToMove = _whiteToMove ? 0u : 1u;
		return bookPosition;
	}

	private int FindFromSquare(int pieceType, int targetSquare, int disambiguationFile, int disambiguationRank)
	{
		int expectedPiece = _whiteToMove ? pieceType + 1 : pieceType + BlackPawn;

		int foundSquare = -1;
		for (int squareIndex = 0; squareIndex < 64; squareIndex++)
		{
			if (_squares[squareIndex] != expectedPiece) continue;
			if (disambiguationFile >= 0 && squareIndex % 8 != disambiguationFile) continue;
			if (disambiguationRank >= 0 && squareIndex / 8 != disambiguationRank) continue;
			if (!CanPieceReach(pieceType, squareIndex, targetSquare)) continue;

			if (foundSquare >= 0)
				throw new InvalidOperationException(
					$"Ambiguous move: multiple {PieceTypeName(pieceType)}s can reach {SquareName(targetSquare)} " +
					$"(from {SquareName(foundSquare)} and {SquareName(squareIndex)})");
			foundSquare = squareIndex;
		}

		if (foundSquare < 0)
			throw new InvalidOperationException(
				$"No {PieceTypeName(pieceType)} found that can reach {SquareName(targetSquare)}");

		return foundSquare;
	}

	private bool CanPieceReach(int pieceType, int fromSquare, int toSquare)
	{
		return pieceType switch
		{
			PawnType => CanPawnReach(fromSquare, toSquare),
			KnightType => CanKnightReach(fromSquare, toSquare),
			BishopType => IsDiagonalClear(fromSquare, toSquare),
			RookType => IsStraightClear(fromSquare, toSquare),
			QueenType => IsDiagonalClear(fromSquare, toSquare) || IsStraightClear(fromSquare, toSquare),
			KingType => CanKingReach(fromSquare, toSquare),
			_ => false
		};
	}

	private bool CanPawnReach(int fromSquare, int toSquare)
	{
		int fromRank = fromSquare / 8, fromFile = fromSquare % 8;
		int toRank = toSquare / 8, toFile = toSquare % 8;
		int direction = _whiteToMove ? 1 : -1;
		int startingRank = _whiteToMove ? 1 : 6;

		// Forward push
		if (fromFile == toFile && _squares[toSquare] == Empty)
		{
			if (toRank == fromRank + direction)
				return true;
			if (fromRank == startingRank && toRank == fromRank + 2 * direction
				&& _squares[(fromRank + direction) * 8 + fromFile] == Empty)
				return true;
		}

		// Diagonal capture (normal or en passant)
		if (Math.Abs(toFile - fromFile) == 1 && toRank == fromRank + direction)
		{
			if (_squares[toSquare] != Empty && IsOpponentPiece(_squares[toSquare]))
				return true;
			if (toSquare == _enPassantSquare)
				return true;
		}

		return false;
	}

	private static bool CanKnightReach(int fromSquare, int toSquare)
	{
		int rankDelta = Math.Abs(toSquare / 8 - fromSquare / 8);
		int fileDelta = Math.Abs(toSquare % 8 - fromSquare % 8);
		return (rankDelta == 2 && fileDelta == 1) || (rankDelta == 1 && fileDelta == 2);
	}

	private static bool CanKingReach(int fromSquare, int toSquare)
	{
		int rankDelta = Math.Abs(toSquare / 8 - fromSquare / 8);
		int fileDelta = Math.Abs(toSquare % 8 - fromSquare % 8);
		return rankDelta <= 1 && fileDelta <= 1 && (rankDelta + fileDelta > 0);
	}

	private bool IsStraightClear(int fromSquare, int toSquare)
	{
		int fromRank = fromSquare / 8, fromFile = fromSquare % 8;
		int toRank = toSquare / 8, toFile = toSquare % 8;

		if (fromRank != toRank && fromFile != toFile) return false;
		if (fromSquare == toSquare) return false;

		int rankStep = Math.Sign(toRank - fromRank);
		int fileStep = Math.Sign(toFile - fromFile);

		int currentRank = fromRank + rankStep, currentFile = fromFile + fileStep;
		while (currentRank != toRank || currentFile != toFile)
		{
			if (_squares[currentRank * 8 + currentFile] != Empty) return false;
			currentRank += rankStep;
			currentFile += fileStep;
		}
		return true;
	}

	private bool IsDiagonalClear(int fromSquare, int toSquare)
	{
		int fromRank = fromSquare / 8, fromFile = fromSquare % 8;
		int toRank = toSquare / 8, toFile = toSquare % 8;

		if (Math.Abs(toRank - fromRank) != Math.Abs(toFile - fromFile)) return false;
		if (fromSquare == toSquare) return false;

		int rankStep = Math.Sign(toRank - fromRank);
		int fileStep = Math.Sign(toFile - fromFile);

		int currentRank = fromRank + rankStep, currentFile = fromFile + fileStep;
		while (currentRank != toRank || currentFile != toFile)
		{
			if (_squares[currentRank * 8 + currentFile] != Empty) return false;
			currentRank += rankStep;
			currentFile += fileStep;
		}
		return true;
	}

	private void ExecuteMove(int fromSquare, int toSquare, int promotionPieceType)
	{
		int movingPiece = _squares[fromSquare];
		int pieceType = movingPiece <= WhiteKing ? movingPiece - 1 : movingPiece - BlackPawn;
		bool isPawn = pieceType == PawnType;

		// En passant capture: remove the captured pawn
		if (isPawn && toSquare == _enPassantSquare && _enPassantSquare >= 0)
		{
			int capturedPawnSquare = _whiteToMove ? toSquare - 8 : toSquare + 8;
			_squares[capturedPawnSquare] = Empty;
		}

		// Move the piece
		_squares[toSquare] = movingPiece;
		_squares[fromSquare] = Empty;

		// Promotion
		if (promotionPieceType >= 0)
			_squares[toSquare] = _whiteToMove ? promotionPieceType + 1 : promotionPieceType + BlackPawn;

		// Update en passant square
		_enPassantSquare = -1;
		if (isPawn && Math.Abs(toSquare / 8 - fromSquare / 8) == 2)
			_enPassantSquare = (fromSquare + toSquare) / 2; // square between from and to

		// Update castling rights
		UpdateCastlingRights(fromSquare, toSquare);

		_whiteToMove = !_whiteToMove;
	}

	private void CastleKingside()
	{
		if (_whiteToMove)
		{
			_squares[6] = _squares[WhiteKingStartSquare]; _squares[WhiteKingStartSquare] = Empty;
			_squares[5] = _squares[WhiteKingsideRookSquare]; _squares[WhiteKingsideRookSquare] = Empty;
			_whiteKingsideCastle = false; _whiteQueensideCastle = false;
		}
		else
		{
			_squares[62] = _squares[BlackKingStartSquare]; _squares[BlackKingStartSquare] = Empty;
			_squares[61] = _squares[BlackKingsideRookSquare]; _squares[BlackKingsideRookSquare] = Empty;
			_blackKingsideCastle = false; _blackQueensideCastle = false;
		}
		_enPassantSquare = -1;
		_whiteToMove = !_whiteToMove;
	}

	private void CastleQueenside()
	{
		if (_whiteToMove)
		{
			_squares[2] = _squares[WhiteKingStartSquare]; _squares[WhiteKingStartSquare] = Empty;
			_squares[3] = _squares[WhiteQueensideRookSquare]; _squares[WhiteQueensideRookSquare] = Empty;
			_whiteKingsideCastle = false; _whiteQueensideCastle = false;
		}
		else
		{
			_squares[58] = _squares[BlackKingStartSquare]; _squares[BlackKingStartSquare] = Empty;
			_squares[59] = _squares[BlackQueensideRookSquare]; _squares[BlackQueensideRookSquare] = Empty;
			_blackKingsideCastle = false; _blackQueensideCastle = false;
		}
		_enPassantSquare = -1;
		_whiteToMove = !_whiteToMove;
	}

	private void UpdateCastlingRights(int fromSquare, int toSquare)
	{
		// King moved
		if (fromSquare == WhiteKingStartSquare) { _whiteKingsideCastle = false; _whiteQueensideCastle = false; }
		if (fromSquare == BlackKingStartSquare) { _blackKingsideCastle = false; _blackQueensideCastle = false; }
		// Rook moved or captured
		if (fromSquare == WhiteKingsideRookSquare || toSquare == WhiteKingsideRookSquare) _whiteKingsideCastle = false;
		if (fromSquare == WhiteQueensideRookSquare || toSquare == WhiteQueensideRookSquare) _whiteQueensideCastle = false;
		if (fromSquare == BlackKingsideRookSquare || toSquare == BlackKingsideRookSquare) _blackKingsideCastle = false;
		if (fromSquare == BlackQueensideRookSquare || toSquare == BlackQueensideRookSquare) _blackQueensideCastle = false;
	}

	private bool IsOpponentPiece(int piece)
	{
		if (_whiteToMove) return piece >= BlackPawn && piece <= BlackKing;
		return piece >= WhitePawn && piece <= WhiteKing;
	}

	private static int CharToPieceType(char character) => character switch
	{
		'K' => KingType,
		'Q' => QueenType,
		'R' => RookType,
		'B' => BishopType,
		'N' => KnightType,
		_ => throw new ArgumentException($"Unknown piece character: {character}")
	};

	private static string PieceTypeName(int pieceType) => pieceType switch
	{
		PawnType => "Pawn", KnightType => "Knight", BishopType => "Bishop",
		RookType => "Rook", QueenType => "Queen", KingType => "King",
		_ => "Unknown"
	};

	private static string SquareName(int squareIndex) =>
		$"{(char)('a' + squareIndex % 8)}{(char)('1' + squareIndex / 8)}";
}
