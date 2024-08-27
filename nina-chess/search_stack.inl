#include "search_stack.h"

forceinline void SearchStack::Reset(const Position& pos)
{
	depth = 0;
	remaining_depth = 0;
	nodes = 0;
	search_aborted.clear();

	position_stack[0] = pos;
	generate_moves(pos, move_list_stack[0]);
}

forceinline void SearchStack::Reset()
{
	Reset(Position());
}


forceinline constexpr void SearchStack::SetCurrentPosition(const Position& position)
{
	position_stack[depth] = position;
}

forceinline constexpr Position& SearchStack::GetPositionAt(const int depth)
{
	return position_stack[depth];
}

forceinline constexpr const Position& SearchStack::GetCurrentPosition() const
{
	return position_stack[depth];
}

forceinline constexpr Position& SearchStack::GetNextPosition()
{
	return position_stack[depth + 1];
}

forceinline constexpr MoveList& SearchStack::GetMoveListAt(const int depth)
{
	if (move_list_stack[depth].GetHashOfPosition() != position_stack[depth].hash)
		generate_moves(position_stack[depth], move_list_stack[depth]);
	return move_list_stack[depth];
}

forceinline constexpr MoveList& SearchStack::GetMoveList()
{
	const Position& curr_pos = GetCurrentPosition();
	if (curr_pos.side_to_move == WHITE)
		return GetMoveList<WHITE>();
	else
		return GetMoveList<BLACK>();
}

forceinline void SearchStack::StartSearch()
{
	start_time = std::chrono::high_resolution_clock::now();
	search_aborted.clear();
	nodes = 0;
}

forceinline int64_t SearchStack::GetSearchDurationInMs() const
{
	const auto end_time = std::chrono::high_resolution_clock::now();
	return (int)std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
}

template<Color side_to_move>
forceinline constexpr MoveList& SearchStack::GetMoveList()
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
inline constexpr MoveList& SearchStack::GetMoveListSkippingHashCheck()
{
	const Position& curr_pos = GetCurrentPosition();
	MoveList& curr_move_list = move_list_stack[depth];
	generate_moves<side_to_move>(curr_pos, curr_move_list);
	return curr_move_list;
}
