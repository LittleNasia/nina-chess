#pragma once
#include "utils.h"

#include "serializable.h"
#include "side.h"

struct BoardFeatures
{
	const Side* white_pieces;
	const Side* black_pieces;
	Bitboard EP_square;
	CastlingType castling;
};