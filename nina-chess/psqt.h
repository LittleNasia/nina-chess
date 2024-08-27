#pragma once
#include "utils.h"

#include "accumulator.h"
#include "chess_bitboard_feature_iterator.h"
#include "move_list.h"
#include "position.h"


class PSQT
{
public:
	inline static constexpr size_t accumulator_output_size = 1;
	using AccumulatorType = BitboardFeatureAccumulator<ChessBitboardFeatureIterator, accumulator_output_size>;

	PSQT(std::ifstream& weights_file);

	forceinline constexpr void Reset(const Position& pos, const MoveList& move_list)
	{
		depth = 0;
		Update(pos, move_list);

		const auto& current_features = board_features[depth];
		const auto& current_move_list_misc = *moves_misc[depth];
		const auto& features_iterator = ChessBitboardFeatureIterator(current_features, current_move_list_misc);

		accumulators[depth].Reset(features_iterator);
	}

	forceinline constexpr void IncrementalUpdate(const Position& pos, const MoveList& move_list)
	{
		depth++;
		Update(pos, move_list);

		const auto& old_board_features = board_features[depth - 1];
		const auto& new_board_features = board_features[depth];

		const auto& old_move_list_misc = *moves_misc[depth - 1];
		const auto& new_move_list_misc = *moves_misc[depth];

		const auto old_features_iterator = ChessBitboardFeatureIterator(old_board_features, old_move_list_misc);
		const auto new_features_iterator = ChessBitboardFeatureIterator(new_board_features, new_move_list_misc);
		
		const auto& old_accumulator = accumulators[depth];

		accumulators[depth + 1].AccumulateFeatures(new_features_iterator, old_features_iterator, old_accumulator.GetOutput());
	}

	forceinline constexpr void UndoUpdate()
	{
		depth--;
	}

	forceinline constexpr float Evaluate()
	{
		return accumulators[depth].GetOutput()[0] / 100.f;
	}

private:
	forceinline constexpr void Update(const Position& pos, const MoveList& move_list)
	{
		auto& curr_board_features = board_features[depth];
		curr_board_features.white_pieces = &pos.white_pieces;
		curr_board_features.black_pieces = &pos.black_pieces;
		curr_board_features.EP_square = pos.EP_square;
		curr_board_features.castling = pos.castling;

		moves_misc[depth] = &move_list.move_list_misc;
	}

	struct AccumulatorContext
	{
		AccumulatorType::Weights accumulator_weights;
	};

	alignas(cache_line_size) const MoveListMisc* moves_misc[max_ply];
	alignas(cache_line_size) BoardFeatures board_features[max_ply];
	alignas(cache_line_size) AccumulatorContext accumulator_context;
	alignas(cache_line_size) AccumulatorType accumulators[max_ply];
	int depth;
};