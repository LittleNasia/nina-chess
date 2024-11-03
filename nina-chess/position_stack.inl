#include "position_stack.h"

forceinline void PositionStack::Reset(const Position& pos)
{
	position_stack[0] = pos;
	generate_moves(pos, move_list_stack[0]);
	depth = 0;
}

inline constexpr bool PositionStack::IsThreefoldRepetition() const
{
	const Position& current_position = GetCurrentPosition();
	const int64_t ply_to_search_to = 0;

	for (int64_t ply = depth - 2; ply >= ply_to_search_to; ply -= 2)
	{
		if (current_position.hash == GetHashAtPly(ply))
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
	position_stack[depth] = position;
}

forceinline constexpr Position& PositionStack::GetPositionAt(const int64_t depth_of_position)
{
	return position_stack[depth_of_position];
}

forceinline constexpr const Position& PositionStack::GetCurrentPosition() const
{
	return position_stack[depth];
}

forceinline constexpr Position& PositionStack::GetNextPosition()
{
	return position_stack[depth + 1];
}

forceinline constexpr MoveList& PositionStack::GetMoveListAt(const int64_t depth_of_position)
{
	if (move_list_stack[depth_of_position].GetHashOfPosition() != position_stack[depth_of_position].hash)
		generate_moves(position_stack[depth_of_position], move_list_stack[depth_of_position]);
	return move_list_stack[depth_of_position];
}

forceinline constexpr MoveList& PositionStack::GetMoveList()
{
	const Position& curr_pos = GetCurrentPosition();
	if (curr_pos.side_to_move == WHITE)
		return GetMoveList<WHITE>();
	else
		return GetMoveList<BLACK>();
}

template<Color side_to_move>
forceinline constexpr MoveList& PositionStack::GetMoveList()
{
	const Position& curr_pos = GetCurrentPosition();
	MoveList& curr_move_list = move_list_stack[depth];
	if (curr_move_list.GetHashOfPosition() != curr_pos.hash)
	{
		generate_moves<side_to_move>(curr_pos, curr_move_list);
	}
	return curr_move_list;
}

template<Color side_to_move>
inline constexpr MoveList& PositionStack::GetMoveListSkippingHashCheck()
{
	const Position& curr_pos = GetCurrentPosition();
	MoveList& curr_move_list = move_list_stack[depth];
	generate_moves<side_to_move>(curr_pos, curr_move_list);
	return curr_move_list;
}
