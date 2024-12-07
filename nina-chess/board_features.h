#pragma once
#include "utils.h"

#include "side.h"

struct BoardFeatures
{
	const Side* WhitePieces;
	const Side* BlackPieces;
	Bitboard EnPassantSquare;
	Castling Castling;
};