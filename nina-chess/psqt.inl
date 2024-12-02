#include "psqt.h"

forceinline PSQT::PSQT(std::ifstream&& weightsFile):
	PSQT(weightsFile)
{
}

PSQT::PSQT(std::ifstream& weightsFile) :
	m_MovesMiscellaneous{},
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
	const auto& currentMoveListMiscellaneous = *m_MovesMiscellaneous[m_Depth];
	const auto& featuresIterator = ChessBitboardFeatureIterator(currentFeatures, currentMoveListMiscellaneous);

	m_Accumulators[m_Depth].Reset(featuresIterator);
}

forceinline constexpr void PSQT::IncrementalUpdate(const Position& position, const MoveList& moveList)
{
	m_Depth++;
	update(position, moveList);

	const auto& oldBoardFeatures = m_BoardFeatures[m_Depth - 1];
	const auto& newBoardFeatures = m_BoardFeatures[m_Depth];

	const auto& oldMoveListMiscellaneous = *m_MovesMiscellaneous[m_Depth - 1];
	const auto& newMoveListMiscellaneous = *m_MovesMiscellaneous[m_Depth];

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

	m_MovesMiscellaneous[m_Depth] = &moveList.MoveListMisc;
}