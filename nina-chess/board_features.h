#pragma once
#include "castling.h"
#include "side.h"
#include "utils.h"

struct BoardFeatures
{
	const Side* WhitePieces;
	const Side* BlackPieces;
	Bitboard EnPassantSquare;
	Castling Castling;
};