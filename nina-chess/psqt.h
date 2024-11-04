#pragma once
#include "utils.h"

#include "accumulator.h"
#include "chess_bitboard_feature_iterator.h"
#include "move_list.h"
#include "position.h"

class PSQT
{
public:
	inline static constexpr size_t ACCUMULATOR_OUTPUT_SIZE = 1;
	using AccumulatorType = BitboardFeatureAccumulator<ChessBitboardFeatureIterator, ACCUMULATOR_OUTPUT_SIZE>;

	forceinline PSQT(std::ifstream&& weightsFile);
	forceinline PSQT(std::ifstream& weightsFile);

	forceinline constexpr void Reset(const Position& position, const MoveList& moveList);

	forceinline constexpr void IncrementalUpdate(const Position& position, const MoveList& moveList);

	forceinline constexpr void UndoUpdate() { m_Depth--; }

	forceinline float Evaluate() { return std::tanhf(m_Accumulators[m_Depth].GetOutput()[0] / 16.f); }

private:
	forceinline constexpr void update(const Position& position, const MoveList& moveList);

	struct AccumulatorContext
	{
		AccumulatorType::Weights AccumulatorWeights;
	};

	alignas(CACHE_LINE_SIZE) const MoveListMiscellaneous* m_MovesMiscellaneous[MAX_PLY];
	alignas(CACHE_LINE_SIZE) BoardFeatures m_BoardFeatures[MAX_PLY];
	alignas(CACHE_LINE_SIZE) AccumulatorContext m_AccumulatorContext;
	alignas(CACHE_LINE_SIZE) AccumulatorType m_Accumulators[MAX_PLY];
	int m_Depth;
};

#include "psqt.inl"