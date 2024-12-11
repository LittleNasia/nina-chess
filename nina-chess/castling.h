#pragma once
#include "color.h"
#include "utils.h"

struct Castling
{
public:
	forceinline constexpr Castling();
	forceinline constexpr Castling(const uint32_t castlingPermissionsBitmask);
	forceinline constexpr Castling(const bool whiteCanCastleKingside, const bool whiteCanCastleQueenside, const bool blackCanCastleKingside, const bool blackCanCastleQueenside);

	uint32_t CurrentCastlingPermissions;

	template<Color color>
	forceinline constexpr bool CanCastleKingside() const;
	template<Color color>
	forceinline constexpr bool CanCastleQueenside() const;
	template<Color color>
	forceinline constexpr uint32_t GetCastlingOfSide() const;
	template<Color color>
	forceinline constexpr void RemoveCastling();

	forceinline constexpr uint32_t GetCastlingOfSide(Color color) const;
	forceinline constexpr void UpdateCastling(const Bitboard whiteRooks, const Bitboard blackRooks);

	template<Color color>
	static consteval uint32_t CastlingPermissionsBitmask();
	template<Color color>
	static consteval Bitboard KingsideCastlingKingDestination();
	template<Color color>
	static consteval Bitboard KingsideCastlingKingPath();
	template<Color color>
	static consteval uint32_t KingsideCastlingPermissionsBitmask();
	template<Color color>
	static consteval Bitboard KingsideCastlingRookBitmask();
	template<Color color>
	static consteval Bitboard KingsideCastlingRookDestination();
	template<Color color>
	static consteval Bitboard KingsideCastlingRookPath();
	template<Color color>
	static consteval Bitboard KingStartposBitmask();
	template<Color color>
	static consteval Bitboard QueensideCastlingKingDestination();
	template<Color color>
	static consteval Bitboard QueensideCastlingKingPath();
	template<Color color>
	static consteval uint32_t QueensideCastlingPermissionsBitmask();
	template<Color color>
	static consteval Bitboard QueensideCastlingRookBitmask();
	template<Color color>
	static consteval Bitboard QueensideCastlingRookDestination();
	template<Color color>
	static consteval Bitboard QueensideCastlingRookPath();

	static consteval Bitboard AllCastlingRooksBitmask();
};


template<Color color>
consteval Bitboard Castling::KingsideCastlingRookBitmask()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0x1;
	}
	else
	{
		return 0x100000000000000;
	}
}

template<Color color>
consteval Bitboard Castling::QueensideCastlingRookBitmask()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0x80;
	}
	else
	{
		return 0x8000000000000000;
	}
}

consteval Bitboard Castling::AllCastlingRooksBitmask()
{
	return KingsideCastlingRookBitmask<WHITE>() | QueensideCastlingRookBitmask<WHITE>() | KingsideCastlingRookBitmask<BLACK>() | QueensideCastlingRookBitmask<BLACK>();
}

forceinline constexpr Castling::Castling(const bool whiteCanCastleKingside, const bool whiteCanCastleQueenside, const bool blackCanCastleKingside, const bool blackCanCastleQueenside) :
	CurrentCastlingPermissions(
		(static_cast<uint32_t>(whiteCanCastleKingside) << 0) |
		(static_cast<uint32_t>(whiteCanCastleQueenside) << 1) |
		(static_cast<uint32_t>(blackCanCastleKingside) << 2) |
		(static_cast<uint32_t>(blackCanCastleQueenside) << 3)
	)
{}

forceinline constexpr Castling::Castling(const uint32_t castlingPermissionsBitmask) :
	CurrentCastlingPermissions(castlingPermissionsBitmask)
{}

forceinline constexpr Castling::Castling() :
	CurrentCastlingPermissions(0b1111)
{}

template<Color color>
forceinline constexpr void Castling::RemoveCastling()
{
	CurrentCastlingPermissions &= ~(0b0011 << (color * 2));
}

forceinline constexpr void Castling::UpdateCastling(const Bitboard whiteRooks, const Bitboard blackRooks)
{
	uint32_t castlingPermsFromRooksBitmask = 0b1111;
	if (!(KingsideCastlingRookBitmask<WHITE>() & whiteRooks))
	{
		castlingPermsFromRooksBitmask &= ~0b0001;
	}
	if (!(QueensideCastlingRookBitmask<WHITE>() & whiteRooks))
	{
		castlingPermsFromRooksBitmask &= ~0b0010;
	}
	if (!(KingsideCastlingRookBitmask<BLACK>() & blackRooks))
	{
		castlingPermsFromRooksBitmask &= ~0b0100;
	}
	if (!(QueensideCastlingRookBitmask<BLACK>() & blackRooks))
	{
		castlingPermsFromRooksBitmask &= ~0b1000;
	}

	CurrentCastlingPermissions &= castlingPermsFromRooksBitmask;
}

template<Color color>
forceinline constexpr uint32_t Castling::GetCastlingOfSide() const
{
	if constexpr (color == WHITE)
	{
		return CurrentCastlingPermissions & 0b0011;
	}
	else if constexpr (color == BLACK)
	{
		return (CurrentCastlingPermissions & 0b1100) >> 2;
	}
}

forceinline constexpr uint32_t Castling::GetCastlingOfSide(Color color) const
{
	return CurrentCastlingPermissions & (0b0011 << (color * 2));
}

template<Color color>
forceinline constexpr bool Castling::CanCastleKingside() const
{
	return GetCastlingOfSide<color>() & 0b01;
}

template<Color color>
forceinline constexpr bool Castling::CanCastleQueenside() const
{
	return GetCastlingOfSide<color>() & 0b10;
}

template<Color color>
consteval Bitboard Castling::KingStartposBitmask()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0x8;
	}
	else
	{
		return 0x800000000000000;
	}
}

template<Color color>
consteval Bitboard Castling::KingsideCastlingKingPath()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0xe;
	}
	else
	{
		return 0xe00000000000000;
	}
}

template<Color color>
consteval Bitboard Castling::KingsideCastlingRookPath()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0x6;
	}
	else
	{
		return 0x600000000000000;
	}
}

template<Color color>
consteval Bitboard Castling::QueensideCastlingKingPath()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0x38;
	}
	else
	{
		return 0x3800000000000000;
	}
}

template<Color color>
consteval Bitboard Castling::QueensideCastlingRookPath()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0x78;
	}
	else
	{
		return 0x7800000000000000;
	}
}

template<Color color>
consteval Bitboard Castling::KingsideCastlingKingDestination()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0x2;
	}
	else
	{
		return 0x200000000000000;
	}
}

template<Color color>
consteval Bitboard Castling::QueensideCastlingKingDestination()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0x20;
	}
	else
	{
		return 0x2000000000000000;
	}
}

template<Color color>
consteval Bitboard Castling::QueensideCastlingRookDestination()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0x10;
	}
	else
	{
		return 0x1000000000000000;
	}
}

template<Color color>
consteval Bitboard Castling::KingsideCastlingRookDestination()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0x4;
	}
	else
	{
		return 0x400000000000000;
	}
}

template<Color color>
consteval uint32_t Castling::CastlingPermissionsBitmask()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0b11;
	}
	else
	{
		return 0b1100;
	}
}

template<Color color>
consteval uint32_t Castling::KingsideCastlingPermissionsBitmask()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0b1;
	}
	else
	{
		return 0b100;
	}
}

template<Color color>
consteval uint32_t Castling::QueensideCastlingPermissionsBitmask()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0b10;
	}
	else
	{
		return 0b1000;
	}
}
