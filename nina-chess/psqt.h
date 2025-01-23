#pragma once
#include "accumulator.h"
#include "chess_bitboard_feature_iterator.h"
#include "move_list.h"
#include "position.h"
#include "utils.h"

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
	forceinline float Evaluate() { return std::tanh(m_Accumulators[m_Depth].GetOutput()[0] / 16.f); }

private:
	forceinline constexpr void update(const Position& position, const MoveList& moveList);

	struct AccumulatorContext
	{
		AccumulatorType::Weights AccumulatorWeights;
	};

	alignas(CACHE_LINE_SIZE) const MoveListMiscellaneous* m_MovesMiscellaneousBitmasks[MAX_PLY];
	alignas(CACHE_LINE_SIZE) BoardFeatures m_BoardFeatures[MAX_PLY];
	alignas(CACHE_LINE_SIZE) AccumulatorContext m_AccumulatorContext;
	alignas(CACHE_LINE_SIZE) AccumulatorType m_Accumulators[MAX_PLY];
	int m_Depth;
};

#include "psqt.h"

forceinline PSQT::PSQT(std::ifstream&& weightsFile) :
	PSQT(weightsFile)
{
}

PSQT::PSQT(std::ifstream& weightsFile) :
	m_MovesMiscellaneousBitmasks{},
	m_BoardFeatures{},
	m_AccumulatorContext{},
	m_Accumulators{},
	m_Depth{ 0 }
{
	m_AccumulatorContext.AccumulatorWeights.SetWeights(weightsFile);

	for (auto& accumulator : m_Accumulators)
	{
		accumulator.SetWeights(m_AccumulatorContext.AccumulatorWeights);
	}
}

forceinline constexpr void PSQT::Reset(const Position& position, const MoveList& moveList)
{
	m_Depth = 0;
	update(position, moveList);

	const auto& currentFeatures = m_BoardFeatures[m_Depth];
	const auto& currentMoveListMiscellaneous = *m_MovesMiscellaneousBitmasks[m_Depth];
	const auto& featuresIterator = ChessBitboardFeatureIterator(currentFeatures, currentMoveListMiscellaneous);

	m_Accumulators[m_Depth].Reset(featuresIterator);
}

forceinline constexpr void PSQT::IncrementalUpdate(const Position& position, const MoveList& moveList)
{
	m_Depth++;
	update(position, moveList);

	const auto& oldBoardFeatures = m_BoardFeatures[m_Depth - 1];
	const auto& newBoardFeatures = m_BoardFeatures[m_Depth];

	const auto& oldMoveListMiscellaneous = *m_MovesMiscellaneousBitmasks[m_Depth - 1];
	const auto& newMoveListMiscellaneous = *m_MovesMiscellaneousBitmasks[m_Depth];

	const auto oldFeaturesIterator = ChessBitboardFeatureIterator(oldBoardFeatures, oldMoveListMiscellaneous);
	const auto newFeaturesIterator = ChessBitboardFeatureIterator(newBoardFeatures, newMoveListMiscellaneous);

	const auto& oldAccumulator = m_Accumulators[m_Depth - 1];

	m_Accumulators[m_Depth].AccumulateFeatures(newFeaturesIterator, oldFeaturesIterator, oldAccumulator.GetOutput());
}

forceinline constexpr void PSQT::update(const Position& position, const MoveList& moveList)
{
	auto& currentBoardFeatures = m_BoardFeatures[m_Depth];
	currentBoardFeatures.WhitePieces = &position.WhitePieces;
	currentBoardFeatures.BlackPieces = &position.BlackPieces;
	currentBoardFeatures.EnPassantSquare = position.EnPassantSquare;
	currentBoardFeatures.Castling = position.CastlingPermissions;

	m_MovesMiscellaneousBitmasks[m_Depth] = &moveList.MoveListMisc;
}
