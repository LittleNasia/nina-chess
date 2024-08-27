#pragma once
#include "utils.h"

#include "side.h"


struct BoardFeatures
{
	const Side* white_pieces;
	const Side* black_pieces;
	Bitboard EP_square;
	CastlingType castling;
};