#include "psqt.h"

inline PSQT::PSQT(std::ifstream&& weights_file):
	PSQT(weights_file)
{
}

PSQT::PSQT(std::ifstream& weights_file) :
	moves_misc{},
	board_features{},
	accumulator_context{},
	accumulators{},
	depth{ 0 }
{
	accumulator_context.accumulator_weights.SetWeights(weights_file);

	for (auto& accumulator : accumulators)
	{
		accumulator.SetWeights(accumulator_context.accumulator_weights);
	}
}

inline constexpr void PSQT::Reset(const Position& pos, const MoveList& move_list)
{
	depth = 0;
	update(pos, move_list);

	const auto& current_features = board_features[depth];
	const auto& current_move_list_misc = *moves_misc[depth];
	const auto& features_iterator = ChessBitboardFeatureIterator(current_features, current_move_list_misc);

	accumulators[depth].Reset(features_iterator);
}

inline constexpr void PSQT::IncrementalUpdate(const Position& pos, const MoveList& move_list)
{
	depth++;
	update(pos, move_list);

	const auto& old_board_features = board_features[depth - 1];
	const auto& new_board_features = board_features[depth];

	const auto& old_move_list_misc = *moves_misc[depth - 1];
	const auto& new_move_list_misc = *moves_misc[depth];

	const auto old_features_iterator = ChessBitboardFeatureIterator(old_board_features, old_move_list_misc);
	const auto new_features_iterator = ChessBitboardFeatureIterator(new_board_features, new_move_list_misc);

	const auto& old_accumulator = accumulators[depth - 1];

	accumulators[depth].AccumulateFeatures(new_features_iterator, old_features_iterator, old_accumulator.GetOutput());
}

inline constexpr void PSQT::update(const Position& pos, const MoveList& move_list)
{
	auto& curr_board_features = board_features[depth];
	curr_board_features.white_pieces = &pos.white_pieces;
	curr_board_features.black_pieces = &pos.black_pieces;
	curr_board_features.EP_square = pos.EP_square;
	curr_board_features.castling = pos.castling;

	moves_misc[depth] = &move_list.move_list_misc;
}