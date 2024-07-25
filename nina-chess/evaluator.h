#pragma once
#include "utils.h"

#include "move_list.h"
#include "position.h"
#include "search_stack.h"

struct BoardFeatures
{
	const Side* white_pieces;
	const Side* black_pieces;
	Bitboard EP_square;
	CastlingType castling;

};

class Evaluator
{
public:
	Evaluator();
	
	void Reset(SearchStack& search_stack);

	template<Color side_to_move>
	constexpr Score Evaluate(const Position& position, const MoveList& move_list);

	template<Color side_to_move>
	constexpr void IncrementalUpdate(const Position& new_pos, const MoveList& move_list);

	constexpr void UndoUpdate() { depth--; }

	template<Color side_to_move>
	constexpr void Update(const Position& position, const MoveList& move_list);

private:
	int depth;
	alignas(cache_line_size) const MoveListMisc* moves_misc[max_ply];
	alignas(cache_line_size) BoardFeatures board_features[max_ply];
};

forceinline Score get_score(const float wdl_chances)
{
	DEBUG_IF(wdl_chances < -1.0f || wdl_chances > 1.0f)
	{
		throw std::runtime_error("wdl_chances must be in the range [-1.0, 1.0]");
	}

	constexpr int32_t win_score = static_cast<int32_t>(Score::WIN);
	return static_cast<Score>(wdl_chances * win_score);
}

forceinline Score get_mated_score(const int32_t mate_in)
{
	DEBUG_IF(mate_in < 0)
	{
		throw std::runtime_error("mate_in must be non-negative");
	}

	return Score(int32_t(Score::LOSS) + mate_in);
}

#include "evaluator.inl"