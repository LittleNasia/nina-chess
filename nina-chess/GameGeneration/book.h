#pragma once
#include "Chess/castling.h"
#include "Chess/color.h"
#include "Chess/piece.h"
#include "Chess/position.h"
#include "Chess/side.h"
#include "Core/Engine/rng.h"
#include <Core/Engine/utils.h>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

struct BookPosition
{
	uint64_t Pieces[PIECE_NONE];
	uint64_t EnPassantSquare;
	uint32_t CastlingPermissions;
	uint32_t SideToMove;
};

class Book
{
public:
	static constexpr int NUM_SLOTS = 17;
	static constexpr int MAX_REGULAR_PLY = 16;
	static constexpr int SPECIAL_PLY = 17;
	static constexpr int HEADER_SIZE = NUM_SLOTS * sizeof(uint64_t);

	forceinline Book(const std::string& path);

	forceinline Position GetRandomPosition(const int ply, Xorshift64& rng) const;
	forceinline bool HasPositionsAtPly(const int ply) const;
	forceinline int GetNumPositionsAtPly(const int ply) const;
	forceinline std::vector<int> GetAvailablePlies() const;

private:
	forceinline Position convertToPosition(const BookPosition& bookPos) const;
	forceinline int getStartIndex(const int ply) const;
	forceinline int getEndIndex(const int ply) const;

	std::vector<BookPosition> m_Positions;
	uint64_t m_SlotOffsets[NUM_SLOTS];
	uint64_t m_FileSize;
};


forceinline Book::Book(const std::string& path)
{
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open())
		throw std::runtime_error("Failed to open book file: " + path);

	// Read header
	file.read(reinterpret_cast<char*>(m_SlotOffsets), sizeof(m_SlotOffsets));

	// Get file size
	file.seekg(0, std::ios::end);
	m_FileSize = static_cast<uint64_t>(file.tellg());

	// Read all positions
	const uint64_t dataSize = m_FileSize - HEADER_SIZE;
	const uint64_t numPositions = dataSize / sizeof(BookPosition);

	m_Positions.resize(numPositions);
	file.seekg(HEADER_SIZE);
	file.read(reinterpret_cast<char*>(m_Positions.data()),
		static_cast<std::streamsize>(dataSize));

	std::cout << "Book loaded: " << path << std::endl;
	std::cout << "  Total positions: " << numPositions << std::endl;
	for (int ply = 1; ply <= MAX_REGULAR_PLY; ply++)
	{
		const int count = GetNumPositionsAtPly(ply);
		if (count > 0)
			std::cout << "  Ply " << ply << ": " << count << " positions" << std::endl;
	}
	const int specialCount = GetNumPositionsAtPly(SPECIAL_PLY);
	if (specialCount > 0)
		std::cout << "  Special: " << specialCount << " positions" << std::endl;
}

forceinline int Book::getStartIndex(const int ply) const
{
	const uint64_t startByte = m_SlotOffsets[ply - 1];
	return static_cast<int>((startByte - HEADER_SIZE) / sizeof(BookPosition));
}

forceinline int Book::getEndIndex(const int ply) const
{
	const uint64_t endByte = (ply < NUM_SLOTS) ? m_SlotOffsets[ply] : m_FileSize;
	return static_cast<int>((endByte - HEADER_SIZE) / sizeof(BookPosition));
}

forceinline int Book::GetNumPositionsAtPly(const int ply) const
{
	if (ply < 1 || ply > NUM_SLOTS) return 0;
	return getEndIndex(ply) - getStartIndex(ply);
}

forceinline bool Book::HasPositionsAtPly(const int ply) const
{
	return GetNumPositionsAtPly(ply) > 0;
}

forceinline std::vector<int> Book::GetAvailablePlies() const
{
	std::vector<int> available;
	for (int ply = 1; ply <= NUM_SLOTS; ply++)
	{
		if (HasPositionsAtPly(ply))
			available.push_back(ply);
	}
	return available;
}

forceinline Position Book::GetRandomPosition(const int ply, Xorshift64& rng) const
{
	const int numPositions = GetNumPositionsAtPly(ply);
	if (numPositions == 0)
		throw std::runtime_error("No positions available at ply " + std::to_string(ply));

	std::uniform_int_distribution<size_t> dist(0, static_cast<size_t>(numPositions) - 1);
	const size_t index = static_cast<size_t>(getStartIndex(ply)) + dist(rng);
	return convertToPosition(m_Positions[index]);
}

forceinline Position Book::convertToPosition(const BookPosition& bookPos) const
{
	const Side white(bookPos.Pieces[0], bookPos.Pieces[1], bookPos.Pieces[2],
		bookPos.Pieces[3], bookPos.Pieces[4], bookPos.Pieces[5]);
	const Side black(bookPos.Pieces[6], bookPos.Pieces[7], bookPos.Pieces[8],
		bookPos.Pieces[9], bookPos.Pieces[10], bookPos.Pieces[11]);
	const Castling castling(bookPos.CastlingPermissions);
	const Color sideToMove = static_cast<Color>(bookPos.SideToMove);

	return Position(white, black, bookPos.EnPassantSquare, castling, sideToMove, 0);
}
