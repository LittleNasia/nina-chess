#include "position.h"
#include "zobrist.h"

forceinline Position::Position() :
	WhitePieces(65280ULL, 66ULL, 36ULL, 129ULL, 16ULL, 8ULL),
	BlackPieces(71776119061217280ULL, 4755801206503243776ULL, 2594073385365405696ULL,
		9295429630892703744ULL, 1152921504606846976ULL, 576460752303423488ULL),
	OccupiedBitmask(18446462598732906495ULL), EnPassantSquare(0ULL), CastlingPermissions(0b1111), SideToMove(WHITE),
	Hash(CalculateHash()),
	FiftyMoveRule(0)
{
}

forceinline Position::Position(const Side& whitePieces, const Side& blackPieces,
	const Bitboard EnPassantSquare, const Castling castling, const Color sideToMove,
	const uint32_t fiftyMoveRule) :
	WhitePieces(whitePieces.Pawns, whitePieces.Knights, whitePieces.Bishops, whitePieces.Rooks, whitePieces.Queens, whitePieces.King),
	BlackPieces(blackPieces.Pawns, blackPieces.Knights, blackPieces.Bishops, blackPieces.Rooks, blackPieces.Queens, blackPieces.King),
	OccupiedBitmask(WhitePieces.Pieces | BlackPieces.Pieces),
	EnPassantSquare(EnPassantSquare),
	CastlingPermissions(castling),
	SideToMove(sideToMove),
	Hash(CalculateHash()),
	FiftyMoveRule(fiftyMoveRule)
{
	ValidateColor(sideToMove);
}

forceinline Position::Position(const Side& whitePieces, const Side& blackPieces,
	const Bitboard enPassantSquare, const Castling castling, const Color sideToMove,
	const uint32_t fiftyMoveRule, const uint64_t hash) :
	WhitePieces(whitePieces.Pawns, whitePieces.Knights, whitePieces.Bishops, whitePieces.Rooks, whitePieces.Queens, whitePieces.King),
	BlackPieces(blackPieces.Pawns, blackPieces.Knights, blackPieces.Bishops, blackPieces.Rooks, blackPieces.Queens, blackPieces.King),
	OccupiedBitmask(WhitePieces.Pieces | BlackPieces.Pieces),
	EnPassantSquare(enPassantSquare),
	CastlingPermissions(castling),
	SideToMove(sideToMove),
	Hash(hash),
	FiftyMoveRule(fiftyMoveRule)
{
	ValidateColor(sideToMove);
}

template<Color sideToMove>
forceinline constexpr uint64_t UpdateHash(uint64_t hash, const PieceType movingPieceType, Bitboard move)
{
	ValidateColor<sideToMove>();
	const Bitboard firstSquare = PopBit(move);
	const Bitboard secondSquare = PopBit(move);
	hash ^= ZOBRIST_PIECE_KEYS[BitIndex(firstSquare)][static_cast<uint32_t>(sideToMove) * static_cast<uint32_t>(PIECE_TYPE_NONE) + static_cast<uint32_t>(movingPieceType)];
	if (secondSquare)
		hash ^= ZOBRIST_PIECE_KEYS[BitIndex(secondSquare)][static_cast<uint32_t>(sideToMove) * static_cast<uint32_t>(PIECE_TYPE_NONE) + static_cast<uint32_t>(movingPieceType)];
	return hash;
}

forceinline constexpr Castling Position::GetCurrentCastling() const
{
	return CastlingPermissions >> (2 * SideToMove) & 0b11;
}

forceinline uint64_t Position::CalculateHash() const
{
	uint64_t calculatedHash = 0ULL;
	calculatedHash ^= (ZOBRIST_SIDE_TO_MOVE_KEY * SideToMove);
	calculatedHash ^= ZOBRIST_CASTLING_KEYS[CastlingPermissions];
	calculatedHash ^= ZOBRIST_EN_PASSANT_KEYS[BitIndex(EnPassantSquare)];

	for (Color color = WHITE; auto & side : { WhitePieces, BlackPieces })
	{
		for (PieceType pieceType = PAWN; pieceType < PIECE_TYPE_NONE; pieceType++)
		{
			Bitboard pieceBitboard = side.GetPieceBitboard(pieceType);
			while (pieceBitboard)
			{
				const size_t index = PopBitAndGetIndex(pieceBitboard);
				calculatedHash ^= ZOBRIST_PIECE_KEYS[index][static_cast<uint32_t>(color) * static_cast<uint32_t>(PIECE_TYPE_NONE) + static_cast<uint32_t>(pieceType)];
			}
		}
		color = BLACK;
	}
	return calculatedHash;
}

forceinline constexpr bool Position::IsDrawn() const
{
	return IsFiftyMoveRule() || IsInsufficientMaterial();
}

forceinline constexpr bool Position::IsFiftyMoveRule() const
{
	return FiftyMoveRule >= 100;
}

forceinline constexpr bool Position::IsInsufficientMaterial() const
{
	// always can mate with horizontal sliders or potential horizontal sliders
	if (
		(WhitePieces.Pawns | BlackPieces.Pawns) ||
		(WhitePieces.Rooks | BlackPieces.Rooks) ||
		(WhitePieces.Queens | BlackPieces.Queens)
		)
	{
		return false;
	}

	/* only bishops and knights beyond this point */

	const uint32_t occupiedCount = Popcnt(OccupiedBitmask);

	// king bishop/knight vs king is always a draw
	if (occupiedCount <= 3)
		return true;

	if (occupiedCount >= 4)
	{
		// there always can be mate with two knights on the board
		if (Popcnt(WhitePieces.Knights | BlackPieces.Knights) >= 2)
			return false;

		// bishops on the same color are always a draw
		const Bitboard lightSquaredBishops = LIGHT_SQUARES & (WhitePieces.Bishops | BlackPieces.Bishops);
		const Bitboard darkSquaredBishops = DARK_SQUARES & (WhitePieces.Bishops | BlackPieces.Bishops);
		const uint32_t lightSquaredBishopsCount = Popcnt(lightSquaredBishops);
		const uint32_t darkSquaredBishopsCount = Popcnt(darkSquaredBishops);

		if (!(lightSquaredBishopsCount == 0 || darkSquaredBishopsCount == 0))
			return false;

		return true;
	}

	return false;
}

forceinline constexpr void Position::UpdateOccupiedBitboard()
{
	WhitePieces.Pieces = WhitePieces.Pawns | WhitePieces.Knights | WhitePieces.Bishops | WhitePieces.Rooks | WhitePieces.Queens | WhitePieces.King;
	BlackPieces.Pieces = BlackPieces.Pawns | BlackPieces.Knights | BlackPieces.Bishops | BlackPieces.Rooks | BlackPieces.Queens | BlackPieces.King;
	OccupiedBitmask = WhitePieces.Pieces | BlackPieces.Pieces;
}

template<Color color>
forceinline constexpr const Side& Position::GetSide() const
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return WhitePieces;
	}
	else
	{
		return BlackPieces;
	}
}

template<Color color>
inline constexpr Side& Position::GetSide()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return WhitePieces;
	}
	else
	{
		return BlackPieces;
	}
}

template<Color sideToMove, bool isCastling, bool isEnPassant>
forceinline Position& position::MakeMove(const Position& pos, Position& newPos, const Move& move)
{
	ValidateColor<sideToMove>();
	constexpr Color oppositeColor = GetOppositeColor<sideToMove>();

	// copy the old position's info first
	newPos.WhitePieces = pos.WhitePieces;
	newPos.BlackPieces = pos.BlackPieces;
	newPos.OccupiedBitmask = pos.OccupiedBitmask;
	newPos.CastlingPermissions = pos.CastlingPermissions;
	newPos.Hash = pos.Hash;
	newPos.FiftyMoveRule = pos.FiftyMoveRule;

	//update all the things that are easy to update
	newPos.Hash ^= ZOBRIST_SIDE_TO_MOVE_KEY;
	newPos.Hash ^= ZOBRIST_EN_PASSANT_KEYS[BitIndex(pos.EnPassantSquare)];
	newPos.EnPassantSquare = 0ULL;
	newPos.FiftyMoveRule++;
	newPos.SideToMove = oppositeColor;

	// start making the move
	Side& ownPieces(newPos.GetSide<sideToMove>());
	Side& enemyPieces(newPos.GetSide<oppositeColor>());
	Side& whitePieces = (sideToMove == WHITE ? ownPieces : enemyPieces);
	Side& blackPieces = (sideToMove == BLACK ? ownPieces : enemyPieces);

	if constexpr (isCastling)
	{
		newPos.Hash ^= ZOBRIST_CASTLING_KEYS[newPos.CastlingPermissions];
		if (move.ToBitmask() == QueensideCastlingRookBitmask<sideToMove>())
		{
			const Bitboard rookMoveBitmask = (move.ToBitmask() | QueensideCastlingRookDestination<sideToMove>());
			const Bitboard kingMoveBitmask = (move.FromBitmask() | QueensideCastlingKingDestination<sideToMove>());
			ownPieces.Rooks ^= rookMoveBitmask;
			ownPieces.King ^= kingMoveBitmask;
			newPos.Hash = UpdateHash<sideToMove>(newPos.Hash, ROOK, rookMoveBitmask);
			newPos.Hash = UpdateHash<sideToMove>(newPos.Hash, KING, kingMoveBitmask);
		}
		else if (move.ToBitmask() == KingsideCastlingRookBitmask<sideToMove>())
		{
			const Bitboard rookMoveBitmask = (move.ToBitmask() | KingsideCastlingRookDestination<sideToMove>());
			const Bitboard kingMoveBitmask = (move.FromBitmask() | KingsideCastlingKingDestination<sideToMove>());
			ownPieces.Rooks ^= rookMoveBitmask;
			ownPieces.King ^= kingMoveBitmask;
			newPos.Hash = UpdateHash<sideToMove>(newPos.Hash, ROOK, rookMoveBitmask);
			newPos.Hash = UpdateHash<sideToMove>(newPos.Hash, KING, kingMoveBitmask);
		}
		newPos.CastlingPermissions &= ~CastlingPermissionsBitmask<sideToMove>();
		newPos.Hash ^= ZOBRIST_CASTLING_KEYS[newPos.CastlingPermissions];
		newPos.Hash ^= ZOBRIST_EN_PASSANT_KEYS[BitIndex(newPos.EnPassantSquare)];
	}
	else if constexpr (isEnPassant)
	{
		newPos.FiftyMoveRule = 0;
		const auto enPassantSquareIndex = BitIndex(pos.EnPassantSquare);
		const auto enPassantVictimBitmask = EN_PASSANT_VICTIM_BITMASK_LOOKUP[enPassantSquareIndex];
		enemyPieces.Pawns ^= enPassantVictimBitmask;
		ownPieces.Pawns ^= (move.FromBitmask() | pos.EnPassantSquare);
		newPos.Hash = UpdateHash<sideToMove>(newPos.Hash, PAWN, (move.FromBitmask() | pos.EnPassantSquare));
		newPos.Hash = UpdateHash<oppositeColor>(newPos.Hash, PAWN, enPassantVictimBitmask);
		newPos.Hash ^= ZOBRIST_EN_PASSANT_KEYS[BitIndex(newPos.EnPassantSquare)];
	}
	else
	{
		newPos.Hash ^= ZOBRIST_CASTLING_KEYS[newPos.CastlingPermissions];
		// normal move, maybe capture

		// remove castling perms because nobody wants them
		if (move.MovingPieceType() == KING)
		{
			newPos.CastlingPermissions &= ~CastlingPermissionsBitmask<sideToMove>();
		}

		if (move.ToBitmask() & enemyPieces.Pieces)
		{
			newPos.FiftyMoveRule = 0;

			if (move.ToBitmask() & enemyPieces.Pawns)
				newPos.Hash = UpdateHash<oppositeColor>(newPos.Hash, PAWN, move.ToBitmask());
			else if (move.ToBitmask() & enemyPieces.Knights)
				newPos.Hash = UpdateHash<oppositeColor>(newPos.Hash, KNIGHT, move.ToBitmask());
			else if (move.ToBitmask() & enemyPieces.Bishops)
				newPos.Hash = UpdateHash<oppositeColor>(newPos.Hash, BISHOP, move.ToBitmask());
			else if (move.ToBitmask() & enemyPieces.Rooks)
				newPos.Hash = UpdateHash<oppositeColor>(newPos.Hash, ROOK, move.ToBitmask());
			else if (move.ToBitmask() & enemyPieces.Queens)
				newPos.Hash = UpdateHash<oppositeColor>(newPos.Hash, QUEEN, move.ToBitmask());
			enemyPieces.RemovePieces(move.ToBitmask());
		}

		Bitboard& pieceBitmask = ownPieces.GetPieceBitboard(move.MovingPieceType());
		pieceBitmask ^= move.FromBitmask() | move.ToBitmask();
		newPos.Hash = UpdateHash<sideToMove>(newPos.Hash, move.MovingPieceType(), move.FromBitmask() | move.ToBitmask());

		if (move.MovingPieceType() == PAWN)
		{
			newPos.FiftyMoveRule = 0;
			if (std::abs((long long)move.FromIndex() - (long long)move.ToIndex()) == 16)
			{
				newPos.EnPassantSquare = GetPawnAdvances<sideToMove>(move.FromBitmask());
			}
			if constexpr (sideToMove == WHITE)
			{
				pieceBitmask &= 0xffffffffffffff;
				if (move.ToBitmask() & ~0xffffffffffffff)
				{
					newPos.Hash ^= ZOBRIST_PIECE_KEYS[move.ToIndex()][WHITE_PAWN];
				}
			}
			else
			{
				pieceBitmask &= 0xffffffffffffff00;
				if (move.ToBitmask() & ~0xffffffffffffff00)
				{
					newPos.Hash ^= ZOBRIST_PIECE_KEYS[move.ToIndex()][BLACK_PAWN];
				}
			}
			if (move.PromotionPieceType() != PIECE_TYPE_NONE)
			{
				ownPieces.GetPieceBitboard(move.PromotionPieceType()) |= move.ToBitmask();
				newPos.Hash = UpdateHash<sideToMove>(newPos.Hash, move.PromotionPieceType(), move.ToBitmask());
			}
		}
		newPos.CastlingPermissions &= UpdateCastlingRights(whitePieces.Rooks, blackPieces.Rooks);
		newPos.Hash ^= ZOBRIST_EN_PASSANT_KEYS[BitIndex(newPos.EnPassantSquare)];
		newPos.Hash ^= ZOBRIST_CASTLING_KEYS[newPos.CastlingPermissions];
	}

	newPos.UpdateOccupiedBitboard();

	DEBUG_ASSERT(newPos.Hash == newPos.CalculateHash());

	return newPos;
}

template<Color sideToMove>
forceinline Position& position::MakeMove(const Position& pos, Position& newPos, const Move& move)
{
	ValidateColor<sideToMove>();
	const bool isCastling = move.IsCastling();
	const bool isEnPassant = move.IsEnPassant();

	if		(!isCastling && !isEnPassant) MakeMove<sideToMove, false, false>(pos, newPos, move);
	else if ( isCastling && !isEnPassant) MakeMove<sideToMove, true , false>(pos, newPos, move);
	else if (!isCastling &&  isEnPassant) MakeMove<sideToMove, false, true >(pos, newPos, move);

	return newPos;
}

forceinline Position& position::MakeMove(const Position& pos, Position& newPos, const Move& move)
{
	const auto& sideToMove = pos.SideToMove;
	const bool isCastling = move.IsCastling();
	const bool isEnPassant = move.IsEnPassant();

	if		(sideToMove == WHITE && !isCastling && !isEnPassant) MakeMove<WHITE, false, false>(pos, newPos, move);
	else if (sideToMove == BLACK && !isCastling && !isEnPassant) MakeMove<BLACK, false, false>(pos, newPos, move);
	else if (sideToMove == WHITE &&  isCastling && !isEnPassant) MakeMove<WHITE, true , false>(pos, newPos, move);
	else if (sideToMove == BLACK &&  isCastling && !isEnPassant) MakeMove<BLACK, true , false>(pos, newPos, move);
	else if (sideToMove == WHITE && !isCastling &&  isEnPassant) MakeMove<WHITE, false, true >(pos, newPos, move);
	else if (sideToMove == BLACK && !isCastling &&  isEnPassant) MakeMove<BLACK, false, true >(pos, newPos, move);

	return newPos;
}