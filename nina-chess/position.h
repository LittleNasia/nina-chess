#pragma once
#include "utils.h"

#include <string_view>

#include "attacks.h"
#include "castling.h"
#include "move.h"
#include "side.h"

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

	Side WhitePieces;
	Side BlackPieces;

	Bitboard OccupiedBitmask;
	Bitboard EnPassantSquare;
	Castling CastlingPermissions;
	Color SideToMove;
	uint64_t Hash;
	uint32_t FiftyMoveRule;
};

namespace position
{
	template<Color sideToMove, bool isCastling, bool isEnPassant>
	forceinline Position& MakeMove(const Position& pos, Position& newPos, const Move& move);

	template<Color sideToMove>
	forceinline Position& MakeMove(const Position& pos, Position& newPos, const Move& move);

	forceinline Position& MakeMove(const Position& pos, Position& newPos, const Move& move);

	void PrintBoard(const Position& currPos);

	Position ParseFen(const std::string_view fen);
}

#include "position.inl"