#pragma once
#include "utils.h"

#include <string_view>

#include "attacks.h"
#include "castling.h"
#include "move.h"
#include "side.h"
#include "zobrist.h"

struct Position
{
	forceinline Position();

	forceinline Position(
		const Side& whitePieces,
		const Side& blackPieces,
		const Bitboard enPassantSquare,
		const Castling castling,
		const Color sideToMove,
		const uint32_t fiftyMoveRule);

	forceinline Position(
		const Side& whitePieces,
		const Side& blackPieces,
		const Bitboard enPassantSquare,
		const Castling castling,
		const Color sideToMove,
		const uint32_t fiftyMoveRule,
		const uint64_t hash);

	template<Color color>
	constexpr const Side& GetSide() const;

	template<Color color>
	constexpr Side& GetSide();

	forceinline constexpr Castling GetCurrentCastling() const;
	template<Color color>
	forceinline constexpr Castling GetCurrentCastling() const;

	forceinline uint64_t CalculateHash() const;

	forceinline constexpr bool IsDrawn() const;
	forceinline constexpr bool IsFiftyMoveRule() const;
	forceinline constexpr bool IsInsufficientMaterial() const;

	forceinline constexpr void UpdateOccupiedBitboard();

	template<Color color>
	forceinline constexpr PieceType RemoveCapturedPieces(const Move& move);

	Side WhitePieces;
	Side BlackPieces;

	Bitboard OccupiedBitmask;
	Bitboard EnPassantSquare;
	Castling CastlingPermissions;
	Color SideToMove;
	uint64_t Hash;
	uint32_t FiftyMoveRule;

	template<Color sideToMove, MoveType moveType>
	forceinline static Position& MakeMove(const Position& pos, Position& newPos, const Move& move);

	template<Color sideToMove>
	forceinline static Position& MakeMove(const Position& pos, Position& newPos, const Move& move);

	forceinline static Position& MakeMove(const Position& pos, Position& newPos, const Move& move);

	forceinline static void PrintBoard(const Position& currPos);

	forceinline static Position ParseFen(const std::string_view fen);
};


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

template<Color sideToMove, size_t numPieces>
forceinline constexpr uint64_t UpdateHash(uint64_t hash, const PieceType movingPieceType, Bitboard move)
{
	ValidateColor<sideToMove>();

	const uint32_t firstSquareIndex = PopBitAndGetIndex(move);
	hash ^= ZOBRIST_PIECE_KEYS[firstSquareIndex][static_cast<uint32_t>(sideToMove) * static_cast<uint32_t>(PIECE_TYPE_NONE) + static_cast<uint32_t>(movingPieceType)];
	if constexpr (numPieces > 1)
	{
		const uint32_t secondSquareIndex = PopBitAndGetIndex(move);
		hash ^= ZOBRIST_PIECE_KEYS[secondSquareIndex][static_cast<uint32_t>(sideToMove) * static_cast<uint32_t>(PIECE_TYPE_NONE) + static_cast<uint32_t>(movingPieceType)];
	}

	return hash;
}

forceinline constexpr Castling Position::GetCurrentCastling() const
{
	return CastlingPermissions.GetCastlingOfSide(SideToMove);
}

template<Color color>
forceinline constexpr Castling Position::GetCurrentCastling() const
{
	ValidateColor<color>();
	return CastlingPermissions.GetCastlingOfSide<color>();
}

forceinline uint64_t Position::CalculateHash() const
{
	uint64_t calculatedHash = 0ULL;
	calculatedHash ^= (ZOBRIST_SIDE_TO_MOVE_KEY * SideToMove);
	calculatedHash ^= ZOBRIST_CASTLING_KEYS[CastlingPermissions.CastlingPermissionsBitmask];
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
forceinline constexpr Side& Position::GetSide()
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

template<Color sideToMove>
forceinline constexpr PieceType Position::RemoveCapturedPieces(const Move& move)
{
	constexpr Color oppositeColor = GetOppositeColor<sideToMove>();
	FiftyMoveRule = 0;

	Side& enemyPieces(GetSide<oppositeColor>());

	PieceType capturedPieceType = PIECE_TYPE_NONE;

	if (move.ToBitmask() & enemyPieces.Pawns)
	{
		Hash = UpdateHash<oppositeColor, 1>(Hash, PAWN, move.ToBitmask());
		capturedPieceType = PAWN;
	}
	else if (move.ToBitmask() & enemyPieces.Knights)
	{
		Hash = UpdateHash<oppositeColor, 1>(Hash, KNIGHT, move.ToBitmask());
		capturedPieceType = KNIGHT;
	}
	else if (move.ToBitmask() & enemyPieces.Bishops)
	{
		Hash = UpdateHash<oppositeColor, 1>(Hash, BISHOP, move.ToBitmask());
		capturedPieceType = BISHOP;
	}
	else if (move.ToBitmask() & enemyPieces.Rooks)
	{
		Hash = UpdateHash<oppositeColor, 1>(Hash, ROOK, move.ToBitmask());
		capturedPieceType = ROOK;
	}
	else if (move.ToBitmask() & enemyPieces.Queens)
	{
		Hash = UpdateHash<oppositeColor, 1>(Hash, QUEEN, move.ToBitmask());
		capturedPieceType = QUEEN;
	}
	enemyPieces.RemovePieces(move.ToBitmask());

	DEBUG_ASSERT(capturedPieceType != PIECE_TYPE_NONE);

	return capturedPieceType;
}

template<Color sideToMove, MoveType moveType>
forceinline Position& Position::MakeMove(const Position& pos, Position& newPos, const Move& move)
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

	if constexpr (moveType == MoveType::KINGSIDE_CASTLING)
	{
		newPos.Hash ^= ZOBRIST_CASTLING_KEYS[newPos.CastlingPermissions.CastlingPermissionsBitmask];
		newPos.CastlingPermissions.RemoveCastling<sideToMove>();
		newPos.Hash ^= ZOBRIST_CASTLING_KEYS[newPos.CastlingPermissions.CastlingPermissionsBitmask];

		constexpr Bitboard rookMoveBitmask = (KingsideCastlingRookBitmask<sideToMove>() | KingsideCastlingRookDestination<sideToMove>());
		constexpr Bitboard kingMoveBitmask = (GetKingStartposBitmask<sideToMove>() | KingsideCastlingKingDestination<sideToMove>());

		constexpr size_t rookStartIndex = BitIndex<KingsideCastlingRookBitmask<sideToMove>()>();
		constexpr size_t rookEndIndex = BitIndex<KingsideCastlingRookDestination<sideToMove>()>();
		constexpr size_t kingStartIndex = BitIndex<GetKingStartposBitmask<sideToMove>()>();
		constexpr size_t kingEndIndex = BitIndex<KingsideCastlingKingDestination<sideToMove>()>();

		constexpr Piece rookPiece = GetPieceFromPieceType<ROOK, sideToMove>();
		constexpr Piece kingPiece = GetPieceFromPieceType<KING, sideToMove>();

		constexpr uint64_t hashUpdate = ZOBRIST_PIECE_KEYS[rookStartIndex][rookPiece] ^
			ZOBRIST_PIECE_KEYS[rookEndIndex][rookPiece] ^
			ZOBRIST_PIECE_KEYS[kingStartIndex][kingPiece] ^
			ZOBRIST_PIECE_KEYS[kingEndIndex][kingPiece];

		newPos.Hash ^= hashUpdate;
		newPos.Hash ^= ZOBRIST_EN_PASSANT_KEYS[BitIndex<0>()];

		ownPieces.Rooks ^= rookMoveBitmask;
		ownPieces.King ^= kingMoveBitmask;
	}
	if constexpr (moveType == MoveType::QUEENSIDE_CASTLING)
	{
		newPos.Hash ^= ZOBRIST_CASTLING_KEYS[newPos.CastlingPermissions.CastlingPermissionsBitmask];
		newPos.CastlingPermissions.RemoveCastling<sideToMove>();
		newPos.Hash ^= ZOBRIST_CASTLING_KEYS[newPos.CastlingPermissions.CastlingPermissionsBitmask];

		constexpr Bitboard rookMoveBitmask = (QueensideCastlingRookBitmask<sideToMove>() | QueensideCastlingRookDestination<sideToMove>());
		constexpr Bitboard kingMoveBitmask = (GetKingStartposBitmask<sideToMove>() | QueensideCastlingKingDestination<sideToMove>());

		constexpr size_t rookStartIndex = BitIndex<QueensideCastlingRookBitmask<sideToMove>()>();
		constexpr size_t rookEndIndex = BitIndex<QueensideCastlingRookDestination<sideToMove>()>();
		constexpr size_t kingStartIndex = BitIndex<GetKingStartposBitmask<sideToMove>()>();
		constexpr size_t kingEndIndex = BitIndex<QueensideCastlingKingDestination<sideToMove>()>();

		constexpr Piece rookPiece = GetPieceFromPieceType<ROOK, sideToMove>();
		constexpr Piece kingPiece = GetPieceFromPieceType<KING, sideToMove>();

		constexpr uint64_t hashUpdate = ZOBRIST_PIECE_KEYS[rookStartIndex][rookPiece] ^
			ZOBRIST_PIECE_KEYS[rookEndIndex][rookPiece] ^
			ZOBRIST_PIECE_KEYS[kingStartIndex][kingPiece] ^
			ZOBRIST_PIECE_KEYS[kingEndIndex][kingPiece];

		newPos.Hash ^= hashUpdate;
		newPos.Hash ^= ZOBRIST_EN_PASSANT_KEYS[BitIndex<0>()];

		ownPieces.Rooks ^= rookMoveBitmask;
		ownPieces.King ^= kingMoveBitmask;
	}
	if constexpr (moveType == MoveType::EN_PASSANT)
	{
		newPos.FiftyMoveRule = 0;
		const auto enPassantSquareIndex = BitIndex(pos.EnPassantSquare);
		const auto enPassantVictimBitmask = EN_PASSANT_VICTIM_BITMASK_LOOKUP[enPassantSquareIndex];
		enemyPieces.Pawns ^= enPassantVictimBitmask;
		ownPieces.Pawns ^= (move.FromBitmask() | pos.EnPassantSquare);
		newPos.Hash = UpdateHash<sideToMove, 2>(newPos.Hash, PAWN, (move.FromBitmask() | pos.EnPassantSquare));
		newPos.Hash = UpdateHash<oppositeColor, 1>(newPos.Hash, PAWN, enPassantVictimBitmask);
		newPos.Hash ^= ZOBRIST_EN_PASSANT_KEYS[BitIndex<0>()];
	}
	if constexpr (moveType == MoveType::DOUBLE_PAWN_ADVANCE)
	{
		newPos.FiftyMoveRule = 0;

		newPos.EnPassantSquare = GetPawnAdvances<sideToMove>(move.FromBitmask());
		newPos.Hash ^= ZOBRIST_EN_PASSANT_KEYS[BitIndex(newPos.EnPassantSquare)];

		const Bitboard moveBitmask = move.FromBitmask() | move.ToBitmask();

		ownPieces.Pawns ^= moveBitmask;
		newPos.Hash = UpdateHash<sideToMove, 2>(newPos.Hash, PAWN, moveBitmask);
	}
	constexpr bool isPromotion = IsMoveTypePromotion<moveType>();
	constexpr bool isCapture = IsMoveTypeCapture<moveType>();

	if constexpr (isPromotion)
	{
		newPos.FiftyMoveRule = 0;

		constexpr PieceType promotionPieceType = GetPromotionPieceFromMoveType<moveType>();

		ownPieces.GetPieceBitboard<promotionPieceType>() |= move.ToBitmask();
		ownPieces.GetPieceBitboard<PAWN>() ^= move.FromBitmask();
		newPos.Hash = UpdateHash<sideToMove, 1>(newPos.Hash, promotionPieceType, move.ToBitmask());
		newPos.Hash = UpdateHash<sideToMove, 1>(newPos.Hash, PAWN, move.FromBitmask());
		newPos.Hash ^= ZOBRIST_EN_PASSANT_KEYS[BitIndex<0>()];

		if constexpr (isCapture)
		{
			const PieceType capturedPieceType = newPos.RemoveCapturedPieces<sideToMove>(move);
			if (capturedPieceType == ROOK)
			{
				newPos.Hash ^= ZOBRIST_CASTLING_KEYS[newPos.CastlingPermissions.CastlingPermissionsBitmask];
				newPos.CastlingPermissions.UpdateCastling(whitePieces.Rooks, blackPieces.Rooks);
				newPos.Hash ^= ZOBRIST_CASTLING_KEYS[newPos.CastlingPermissions.CastlingPermissionsBitmask];
			}
		}
	}

	if constexpr (moveType == MoveType::CAPTURE)
	{
		newPos.FiftyMoveRule = 0;

		const Bitboard moveBitmask = move.FromBitmask() | move.ToBitmask();
		const PieceType movingPieceType = move.MovingPieceType();

		ownPieces.GetPieceBitboard(movingPieceType) ^= moveBitmask;
		newPos.Hash = UpdateHash<sideToMove, 2>(newPos.Hash, movingPieceType, moveBitmask);
		newPos.Hash ^= ZOBRIST_EN_PASSANT_KEYS[BitIndex<0>()];

		const PieceType capturedPieceType = newPos.RemoveCapturedPieces<sideToMove>(move);
		if (movingPieceType == ROOK || capturedPieceType == ROOK)
		{
			newPos.Hash ^= ZOBRIST_CASTLING_KEYS[newPos.CastlingPermissions.CastlingPermissionsBitmask];
			newPos.CastlingPermissions.UpdateCastling(whitePieces.Rooks, blackPieces.Rooks);
			newPos.Hash ^= ZOBRIST_CASTLING_KEYS[newPos.CastlingPermissions.CastlingPermissionsBitmask];
		}
		if (movingPieceType == KING)
		{
			newPos.Hash ^= ZOBRIST_CASTLING_KEYS[newPos.CastlingPermissions.CastlingPermissionsBitmask];
			newPos.CastlingPermissions.RemoveCastling<sideToMove>();
			newPos.Hash ^= ZOBRIST_CASTLING_KEYS[newPos.CastlingPermissions.CastlingPermissionsBitmask];
		}
	}

	if constexpr (moveType == MoveType::NORMAL)
	{
		const Bitboard moveBitmask = move.FromBitmask() | move.ToBitmask();
		const PieceType movingPieceType = move.MovingPieceType();

		ownPieces.GetPieceBitboard(movingPieceType) ^= moveBitmask;
		newPos.Hash = UpdateHash<sideToMove, 2>(newPos.Hash, movingPieceType, moveBitmask);
		newPos.Hash ^= ZOBRIST_EN_PASSANT_KEYS[BitIndex<0>()];

		if (movingPieceType == ROOK)
		{
			newPos.Hash ^= ZOBRIST_CASTLING_KEYS[newPos.CastlingPermissions.CastlingPermissionsBitmask];
			newPos.CastlingPermissions.UpdateCastling(whitePieces.Rooks, blackPieces.Rooks);
			newPos.Hash ^= ZOBRIST_CASTLING_KEYS[newPos.CastlingPermissions.CastlingPermissionsBitmask];
		}
		else if (movingPieceType == KING)
		{
			newPos.Hash ^= ZOBRIST_CASTLING_KEYS[newPos.CastlingPermissions.CastlingPermissionsBitmask];
			newPos.CastlingPermissions.RemoveCastling<sideToMove>();
			newPos.Hash ^= ZOBRIST_CASTLING_KEYS[newPos.CastlingPermissions.CastlingPermissionsBitmask];
		}
	}

	newPos.UpdateOccupiedBitboard();
	DEBUG_ASSERT(newPos.Hash == newPos.CalculateHash());

	return newPos;
}

template<Color sideToMove>
forceinline Position& Position::MakeMove(const Position& pos, Position& newPos, const Move& move)
{
	ValidateColor<sideToMove>();
	const MoveType moveType = move.GetMoveType();

	switch (moveType)
	{
	case MoveType::NORMAL:
		return MakeMove<sideToMove, MoveType::NORMAL>(pos, newPos, move);
	case MoveType::CAPTURE:
		return MakeMove<sideToMove, MoveType::CAPTURE>(pos, newPos, move);
	case MoveType::DOUBLE_PAWN_ADVANCE:
		return MakeMove<sideToMove, MoveType::DOUBLE_PAWN_ADVANCE>(pos, newPos, move);
	case MoveType::KINGSIDE_CASTLING:
		return MakeMove<sideToMove, MoveType::KINGSIDE_CASTLING>(pos, newPos, move);
	case MoveType::QUEENSIDE_CASTLING:
		return MakeMove<sideToMove, MoveType::QUEENSIDE_CASTLING>(pos, newPos, move);
	case MoveType::EN_PASSANT:
		return MakeMove<sideToMove, MoveType::EN_PASSANT>(pos, newPos, move);
	case MoveType::PROMOTION_TO_QUEEN:
		return MakeMove<sideToMove, MoveType::PROMOTION_TO_QUEEN>(pos, newPos, move);
	case MoveType::PROMOTION_TO_ROOK:
		return MakeMove<sideToMove, MoveType::PROMOTION_TO_ROOK>(pos, newPos, move);
	case MoveType::PROMOTION_TO_BISHOP:
		return MakeMove<sideToMove, MoveType::PROMOTION_TO_BISHOP>(pos, newPos, move);
	case MoveType::PROMOTION_TO_KNIGHT:
		return MakeMove<sideToMove, MoveType::PROMOTION_TO_KNIGHT>(pos, newPos, move);
	case MoveType::PROMOTION_TO_QUEEN_AND_CAPTURE:
		return MakeMove<sideToMove, MoveType::PROMOTION_TO_QUEEN_AND_CAPTURE>(pos, newPos, move);
	case MoveType::PROMOTION_TO_ROOK_AND_CAPTURE:
		return MakeMove<sideToMove, MoveType::PROMOTION_TO_ROOK_AND_CAPTURE>(pos, newPos, move);
	case MoveType::PROMOTION_TO_BISHOP_AND_CAPTURE:
		return MakeMove<sideToMove, MoveType::PROMOTION_TO_BISHOP_AND_CAPTURE>(pos, newPos, move);
	case MoveType::PROMOTION_TO_KNIGHT_AND_CAPTURE:
		return MakeMove<sideToMove, MoveType::PROMOTION_TO_KNIGHT_AND_CAPTURE>(pos, newPos, move);
#ifdef _DEBUG
	default:
		DEBUG_ASSERT(false);
#endif
	}
}

forceinline Position& Position::MakeMove(const Position& pos, Position& newPos, const Move& move)
{
	const auto& sideToMove = pos.SideToMove;

	switch (sideToMove)
	{
	case WHITE:
		return MakeMove<WHITE>(pos, newPos, move);
	case BLACK:
		return MakeMove<BLACK>(pos, newPos, move);
#ifdef _DEBUG
	default:
		DEBUG_ASSERT(false);
#endif
	}
}

template<Color sideToMove>
forceinline constexpr char GetPieceChar(const Side& side, const Bitboard bit)
{
	ValidateColor<sideToMove>();
	if (side.Pawns & bit)
		return sideToMove == WHITE ? 'P' : 'p';
	if (side.Knights & bit)
		return sideToMove == WHITE ? 'N' : 'n';
	if (side.Bishops & bit)
		return sideToMove == WHITE ? 'B' : 'b';
	if (side.Rooks & bit)
		return sideToMove == WHITE ? 'R' : 'r';
	if (side.Queens & bit)
		return sideToMove == WHITE ? 'Q' : 'q';
	if (side.King & bit)
		return sideToMove == WHITE ? 'K' : 'k';
	return ' ';
}

forceinline constexpr char GetPieceChar(const Position& pos, const Bitboard bit)
{
	if (bit & ~pos.OccupiedBitmask)
		return ' ';
	else if (bit & pos.WhitePieces.Pieces)
		return GetPieceChar<WHITE>(pos.WhitePieces, bit);
	else
		return GetPieceChar<BLACK>(pos.BlackPieces, bit);
}

forceinline void Position::PrintBoard(const Position& curr_pos)
{
	// rank names and pieces
	int rank = 8;
	for (int square = 63; square >= 0; square--)
	{
		const Bitboard curr_bit = 1ULL << square;

		if ((square + 1) % BOARD_COLUMNS == 0)
		{
			std::cout << "\n";
			std::cout << rank-- << ' ';
		}

		std::cout << '[';
		std::cout << GetPieceChar(curr_pos, curr_bit);
		std::cout << ']';
	}

	// file names
	std::cout << "\n  ";
	for (int file = 0; file < 8; file++)
	{
		std::cout << ' ' << char('a' + file) << ' ';
	}

	// occupied bitboard
	std::cout << "\n\n";
	for (int square = 63; square >= 0; square--)
	{
		if ((square + 1) % 8 == 0)
		{
			std::cout << "\n";
		}

		std::cout << "[";
		std::cout << (curr_pos.OccupiedBitmask & (1ULL << square) ? "X" : " ");
		std::cout << "]";
	}

	// misc info
	std::cout << "\n";
	std::cout << "castling: BLACK: ";
	std::cout <<
		bool(curr_pos.CastlingPermissions.CastlingPermissionsBitmask & 0b1000) <<
		bool(curr_pos.CastlingPermissions.CastlingPermissionsBitmask & 0b0100) <<
		" WHITE: " <<
		bool(curr_pos.CastlingPermissions.CastlingPermissionsBitmask & 0b0010) <<
		bool(curr_pos.CastlingPermissions.CastlingPermissionsBitmask & 0b0001) <<
		"\n";
	std::cout << "side_to_move " << (curr_pos.SideToMove == WHITE ? "WHITE" : "BLACK");
}

forceinline Position Position::ParseFen(const std::string_view fen)
{
	uint32_t row = 7;
	uint32_t col = 7;
	uint32_t index = 0;
	Side white_pieces;
	Side black_pieces;
	const uint32_t fifty_move_rule = 0;
	for (index = 0; index < fen.length(); index++)
	{
		// "col 0" is actually on the right of the board (where square 0 is)
		if (fen[index] == '/')
		{
			row--;
			col = 7;
			continue;
		}
		else if (fen[index] == ' ')
		{
			// we go parse side to move and other things afterwards
			break;
		}
		PieceType curr_piece_type = PIECE_TYPE_NONE;
		Color piece_color = COLOR_NONE;
		const size_t curr_index = TwoDimensionalIndexToOneDimensional(row, col);
		const size_t curr_piece = 1ULL << curr_index;
		// put pieces
		switch (fen[index])
		{
		case 'p':
			curr_piece_type = PAWN;
			piece_color = BLACK;
			black_pieces.Pawns ^= curr_piece;
			break;
		case 'r':
			curr_piece_type = ROOK;
			piece_color = BLACK;
			black_pieces.Rooks ^= curr_piece;
			break;
		case 'b':
			curr_piece_type = BISHOP;
			piece_color = BLACK;
			black_pieces.Bishops ^= curr_piece;
			break;
		case 'q':
			curr_piece_type = QUEEN;
			piece_color = BLACK;
			black_pieces.Queens ^= curr_piece;
			break;
		case 'k':
			curr_piece_type = KING;
			piece_color = BLACK;
			black_pieces.King ^= curr_piece;
			break;
		case 'n':
			curr_piece_type = KNIGHT;
			piece_color = BLACK;
			black_pieces.Knights ^= curr_piece;
			break;
		case 'P':
			curr_piece_type = PAWN;
			piece_color = WHITE;
			white_pieces.Pawns ^= curr_piece;
			break;
		case 'R':
			curr_piece_type = ROOK;
			piece_color = WHITE;
			white_pieces.Rooks ^= curr_piece;
			break;
		case 'B':
			curr_piece_type = BISHOP;
			piece_color = WHITE;
			white_pieces.Bishops ^= curr_piece;
			break;
		case 'Q':
			curr_piece_type = QUEEN;
			piece_color = WHITE;
			white_pieces.Queens ^= curr_piece;
			break;
		case 'K':
			curr_piece_type = KING;
			piece_color = WHITE;
			white_pieces.King ^= curr_piece;
			break;
		case 'N':
			curr_piece_type = KNIGHT;
			piece_color = WHITE;
			white_pieces.Knights ^= curr_piece;
			break;
			// if it's not a piece, it's a number (new rows and end of pieces was already handled)
		default:
		{
			int num_of_empty_squares = fen[index] - '0';
			for (int empty_square = 0; empty_square < num_of_empty_squares; empty_square++)
			{
				col--;
			}
			continue;
		}
		}
		col--;
	}

	std::stringstream ss(fen.substr(index).data());

	char fen_side_to_move;
	ss >> fen_side_to_move;
	Color side_to_move = WHITE;
	switch (fen_side_to_move)
	{
	case 'w':
		side_to_move = WHITE;
		break;
	case 'b':
		side_to_move = BLACK;
		break;
	default:
		std::cout << "wrong fen lmao\n";
		break;
	}

	std::string fen_castling_rights;
	ss >> fen_castling_rights;
	Castling castling_rights = 0;
	for (uint32_t castling_index = 0; castling_index < fen_castling_rights.length(); castling_index++)
	{
		if (fen_castling_rights[castling_index] == '-')
		{
			break;
		}
		switch (fen_castling_rights[castling_index])
		{
		case 'K':
			castling_rights.CastlingPermissionsBitmask |= KingsideCastlingPermissionsBitmask<WHITE>();
			break;
		case 'Q':
			castling_rights.CastlingPermissionsBitmask |= QueensideCastlingPermissionsBitmask<WHITE>();
			break;
		case 'k':
			castling_rights.CastlingPermissionsBitmask |= KingsideCastlingPermissionsBitmask<BLACK>();
			break;
		case 'q':
			castling_rights.CastlingPermissionsBitmask |= QueensideCastlingPermissionsBitmask<BLACK>();
			break;
		default:
			std::cout << "fen sucks\n";
			break;
		}
	}

	std::string fen_en_passant_square;
	ss >> fen_en_passant_square;
	Bitboard EP_square;
	if (fen_en_passant_square == "-")
	{
		EP_square = 0ULL;
	}
	else
	{
		const size_t EP_index = GetSquareIndexFromChessSquareName(fen_en_passant_square.c_str());
		EP_square = (1ULL << EP_index);
		// sometimes en_passant square is just wrongly set in some fens
		// this stops some fens from completely ruining my sweet little program
		// my sweet little ilusia will be okay too
		//if (b.curr_pos.occupied & EP_square)
		//{
		//	EP_square = 0ULL;
		//}
	}

	return Position(
		white_pieces,
		black_pieces,
		EP_square, castling_rights, side_to_move, fifty_move_rule);
}