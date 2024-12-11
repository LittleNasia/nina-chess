#pragma once
#include "move_gen.h"
#include "position.h"
#include "search_core.h"
#include "utils.h"

struct PositionStack
{
public:
	forceinline PositionStack();

	template<Color sideToMove>
	forceinline constexpr MoveList& GetMoveListSkippingHashCheck();
	template<Color sideToMove>
	forceinline constexpr MoveList& GetMoveList();

	forceinline constexpr void DecrementDepth() { --m_Depth; }
	forceinline constexpr int64_t GetDepth() const { return m_Depth; }
	forceinline constexpr const Position& GetCurrentPosition() const;
	forceinline constexpr Position& GetPositionAt(const int64_t depth);
	forceinline constexpr MoveList& GetMoveListAt(const int64_t depth);
	forceinline constexpr MoveList& GetMoveList();
	forceinline constexpr Position& GetNextPosition();
	forceinline constexpr void IncrementDepth() { ++m_Depth; }
	forceinline constexpr bool IsThreefoldRepetition() const;
	forceinline void Reset();
	forceinline void Reset(const Position& position);
	forceinline constexpr void SetCurrentPosition(const Position& position);

private:
	forceinline constexpr uint64_t GetHashAtPly(const int64_t ply) const { return m_PositionStack[ply].Hash; }

	MoveList m_MoveListStack[MAX_PLY];
	Position m_PositionStack[MAX_PLY];
	int64_t m_Depth;
};


forceinline PositionStack::PositionStack() :
	m_MoveListStack{},
	m_PositionStack{},
	m_Depth{ 0 }
{
}

forceinline void PositionStack::Reset(const Position& pos)
{
	m_PositionStack[0] = pos;
	GenerateMoves(pos, m_MoveListStack[0]);
	m_Depth = 0;
}

forceinline constexpr bool PositionStack::IsThreefoldRepetition() const
{
	const Position& currentPosition = GetCurrentPosition();
	const int64_t plyToSearchTo = 0;

	for (int64_t ply = m_Depth - 2; ply >= plyToSearchTo; ply -= 2)
	{
		if (currentPosition.Hash == GetHashAtPly(ply))
		{
			return true;
		}
	}
	return false;
}

forceinline void PositionStack::Reset()
{
	Reset(Position());
}

forceinline constexpr void PositionStack::SetCurrentPosition(const Position& position)
{
	m_PositionStack[m_Depth] = position;
}

forceinline constexpr Position& PositionStack::GetPositionAt(const int64_t depthOfPosition)
{
	return m_PositionStack[depthOfPosition];
}

forceinline constexpr const Position& PositionStack::GetCurrentPosition() const
{
	return m_PositionStack[m_Depth];
}

forceinline constexpr Position& PositionStack::GetNextPosition()
{
	return m_PositionStack[m_Depth + 1];
}

forceinline constexpr MoveList& PositionStack::GetMoveListAt(const int64_t depthOfPosition)
{
	if (m_MoveListStack[depthOfPosition].GetHashOfPosition() != m_PositionStack[depthOfPosition].Hash)
		GenerateMoves(m_PositionStack[depthOfPosition], m_MoveListStack[depthOfPosition]);
	return m_MoveListStack[depthOfPosition];
}

forceinline constexpr MoveList& PositionStack::GetMoveList()
{
	const Position& currentPosition = GetCurrentPosition();
	if (currentPosition.SideToMove == WHITE)
		return GetMoveList<WHITE>();
	else
		return GetMoveList<BLACK>();
}

template<Color sideToMove>
forceinline constexpr MoveList& PositionStack::GetMoveList()
{
	const Position& currentPosition = GetCurrentPosition();
	MoveList& currentMoveList = m_MoveListStack[m_Depth];
	if (currentMoveList.GetHashOfPosition() != currentPosition.Hash)
	{
		GenerateMoves<sideToMove>(currentPosition, currentMoveList);
	}
	return currentMoveList;
}

template<Color sideToMove>
forceinline constexpr MoveList& PositionStack::GetMoveListSkippingHashCheck()
{
	const Position& currentPosition = GetCurrentPosition();
	MoveList& currentMoveList = m_MoveListStack[m_Depth];
	GenerateMoves<sideToMove>(currentPosition, currentMoveList);
	return currentMoveList;
}
