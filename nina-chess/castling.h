#pragma once
#include "utils.h"

#include "color.h"

using Castling = uint32_t;

template<Color color, Castling castling>
forceinline constexpr Castling get_castling()
{
	static_assert(castling <= 0b1111);
	validate_color<color>();
	if constexpr (color == WHITE)
	{
		return castling & 0b11;
	}
	else
	{
		return (castling & 0b1100) >> 2;
	}
}

// bitmask of the path of king during castling
template<Color color>
forceinline constexpr Bitboard kingside_castling_castling_king_path()
{
	validate_color<color>();
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
forceinline constexpr Bitboard kingside_castling_castling_rook_path()
{
	validate_color<color>();
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
forceinline constexpr Bitboard queenside_castling_king_path()
{
	validate_color<color>();
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
forceinline constexpr Bitboard queenside_castling_rook_path()
{
	validate_color<color>();
	if constexpr (color == WHITE)
	{
		return 0x70;
	}
	else
	{
		return 0x7000000000000000;
	}
}

template<Color color>
forceinline constexpr Bitboard kingside_castling_rook()
{
	validate_color<color>();
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
forceinline constexpr Bitboard queenside_castling_rook()
{
	validate_color<color>();
	if constexpr (color == WHITE)
	{
		return 0x80;
	}
	else
	{
		return 0x8000000000000000;
	}
}

template<Color color>
forceinline constexpr Bitboard kingside_castling_king_dest()
{
	validate_color<color>();
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
forceinline constexpr Bitboard queenside_castling_king_dest()
{
	validate_color<color>();
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
forceinline constexpr Bitboard queenside_castling_rook_dest()
{
	validate_color<color>();
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
forceinline constexpr Bitboard kingside_castling_rook_dest()
{
	validate_color<color>();
	if constexpr (color == WHITE)
	{
		return 0x4;
	}
	else
	{
		return 0x400000000000000;
	}
}

forceinline constexpr Castling update_castling_rights(const Bitboard white_rooks, const Bitboard black_rooks)
{
	Castling castling_perms = 0b1111;
	if (!(kingside_castling_rook<WHITE>() & white_rooks))
	{
		castling_perms &= ~0b0001;
	}
	if (!(queenside_castling_rook<WHITE>() & white_rooks))
	{
		castling_perms &= ~0b0010;
	}
	if (!(kingside_castling_rook<BLACK>() & black_rooks))
	{
		castling_perms &= ~0b0100;
	}
	if (!(queenside_castling_rook<BLACK>() & black_rooks))
	{
		castling_perms &= ~0b1000;
	}
	return castling_perms;
}

template<Color color>
forceinline constexpr Castling castling_perms()
{
	validate_color<color>();
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
forceinline constexpr Castling kingside_castling_perms()
{
	validate_color<color>();
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
forceinline constexpr Castling queenside_castling_perms()
{
	validate_color<color>();
	if constexpr (color == WHITE)
	{
		return 0b10;
	}
	else
	{
		return 0b1000;
	}
}

