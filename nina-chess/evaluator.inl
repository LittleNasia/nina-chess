#include "evaluator.h"

template<Color side_to_move>
constexpr Score Evaluator::Evaluate(const Position& position, const MoveList& move_list)
{
	if (move_list.GetNumMoves() == 0)
	{
		return move_list.move_list_misc.checkers ? get_mated_score(depth) : get_score(0.0f);
	}

	float score = (float)(int(popcnt(position.GetSide<WHITE>().pieces)) - int(popcnt(position.GetSide<BLACK>().pieces))) / 16.f;
	score *= (side_to_move == WHITE ? 1 : -1);

	return get_score(score);
}

template<Color side_to_move>
constexpr void Evaluator::IncrementalUpdate(const Position& new_pos, const MoveList& move_list)
{
	depth++;
	Update<side_to_move>(new_pos, move_list);
}

template<Color side_to_move>
constexpr void Evaluator::Update(const Position& position, const MoveList& move_list)
{
	auto& curr_board_features = board_features[depth];
	curr_board_features.white_pieces = &position.white_pieces;
	curr_board_features.black_pieces = &position.black_pieces;
	curr_board_features.EP_square = position.EP_square;
	curr_board_features.castling = position.castling;

	moves_misc[depth] = &move_list.move_list_misc;
}
