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

	forceinline PSQT(std::ifstream& weights_file);

	forceinline constexpr void Reset(const Position& pos, const MoveList& move_list);

	forceinline constexpr void IncrementalUpdate(const Position& pos, const MoveList& move_list);

	forceinline constexpr void UndoUpdate() { depth--; }

	forceinline float Evaluate() { return tanhf(accumulators[depth].GetOutput()[0] / 16.f); }

private:
	forceinline constexpr void update(const Position& pos, const MoveList& move_list);

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

#include "psqt.inl"