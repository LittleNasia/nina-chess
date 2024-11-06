#pragma once
#include "utils.h"

#include "color.h"

template<Color color>
forceinline consteval Bitboard KingsideCastlingRookBitmask()
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
forceinline consteval Bitboard QueensideCastlingRookBitmask()
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

forceinline consteval Bitboard AllCastlingRooksBitmask()
{
	return KingsideCastlingRookBitmask<WHITE>() | QueensideCastlingRookBitmask<WHITE>() | KingsideCastlingRookBitmask<BLACK>() | QueensideCastlingRookBitmask<BLACK>();
}



struct Castling
{
public:
	forceinline constexpr Castling(const bool whiteCanCastleKingside, const bool whiteCanCastleQueenside, const bool blackCanCastleKingside, const bool blackCanCastleQueenside):
		CastlingPermissionsBitmask(
			(static_cast<uint32_t>(whiteCanCastleKingside ) << 0) | 
			(static_cast<uint32_t>(whiteCanCastleQueenside) << 1) | 
			(static_cast<uint32_t>(blackCanCastleKingside ) << 2) |
			(static_cast<uint32_t>(blackCanCastleQueenside) << 3)
		)
	{

	}

	forceinline constexpr Castling(const uint32_t castlingPermissionsBitmask) :
		CastlingPermissionsBitmask(castlingPermissionsBitmask)
	{

	}

	forceinline constexpr Castling():
		CastlingPermissionsBitmask(0b1111)
	{

	}

	template<Color color>
	forceinline constexpr void RemoveCastling()
	{
		CastlingPermissionsBitmask &= ~(0b0011 << (color * 2));
	}

	forceinline constexpr void UpdateCastling(const Bitboard whiteRooks, const Bitboard blackRooks)
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
		
		CastlingPermissionsBitmask &= castlingPermsFromRooksBitmask;
	}

	template<Color color>
	forceinline constexpr uint32_t GetCastlingOfSide() const
	{
		if constexpr (color == WHITE)
		{
			return CastlingPermissionsBitmask & 0b0011;
		}
		else if constexpr (color == BLACK)
		{
			return (CastlingPermissionsBitmask & 0b1100) >> 2;
		}
	}

	forceinline constexpr uint32_t GetCastlingOfSide(Color color) const
	{
		return CastlingPermissionsBitmask & (0b0011 << (color * 2));
	}

	template<Color color> 
	forceinline constexpr bool CanCastleKingside() const
	{
		return GetCastlingOfSide<color>() & 0b01;
	}

	template<Color color>
	forceinline constexpr bool CanCastleQueenside() const
	{
		return GetCastlingOfSide<color>() & 0b10;
	}

	uint32_t CastlingPermissionsBitmask;
};


// bitmask of the path of king during castling
template<Color color>
forceinline constexpr Bitboard KingsideCastlingKingPath()
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

// bitmask of the path of king during castling
template<Color color>
forceinline constexpr Bitboard KingsideCastlingRookPath()
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

// bitmask of the path of king during castling
template<Color color>
forceinline constexpr Bitboard QueensideCastlingKingPath()
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

// bitmask of the path of king during castling
template<Color color>
forceinline constexpr Bitboard QueensideCastlingRookPath()
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
forceinline constexpr Bitboard KingsideCastlingKingDestination()
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
forceinline constexpr Bitboard QueensideCastlingKingDestination()
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
forceinline constexpr Bitboard QueensideCastlingRookDestination()
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
forceinline constexpr Bitboard KingsideCastlingRookDestination()
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
forceinline constexpr uint32_t CastlingPermissionsBitmask()
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
forceinline constexpr uint32_t KingsideCastlingPermissionsBitmask()
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
forceinline constexpr uint32_t QueensideCastlingPermissionsBitmask()
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

