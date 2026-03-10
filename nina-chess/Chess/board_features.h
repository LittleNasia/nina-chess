#pragma once
#include "Chess/castling.h"
#include "Chess/side.h"
#include "Core/Engine/utils.h"

struct BoardFeatures
{
	const Side* WhitePieces;
	const Side* BlackPieces;
	Bitboard EnPassantSquare;
	Castling Castling;
};
