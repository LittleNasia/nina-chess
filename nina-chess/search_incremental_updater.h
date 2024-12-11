#pragma once
#include "common_incremental_updater.h"
#include "utils.h"

class SearchIncrementalUpdater: public CommonIncrementalUpdater
{
public:
	SearchIncrementalUpdater(CommonIncrementalUpdater& commonIncrementalUpdater, int64_t depthToSearchTo);

	template<Color sideToMove>
	forceinline constexpr void MakeMoveUpdate(const Move& move);

	forceinline constexpr int64_t GetRemainingDepth() const { return m_RemainingDepth; }
	forceinline constexpr int64_t GetSearchDepth() const { return m_SearchDepth;  }
	forceinline constexpr void UndoMoveUpdate();
	
private:
	int64_t m_RemainingDepth;
	int64_t m_SearchDepth;
};

SearchIncrementalUpdater::SearchIncrementalUpdater(CommonIncrementalUpdater& commonIncrementalUpdater, int64_t depthToSearchTo) :
	CommonIncrementalUpdater(commonIncrementalUpdater),
	m_RemainingDepth(depthToSearchTo),
	m_SearchDepth(0)
{
}

template<Color sideToMove>
forceinline constexpr void SearchIncrementalUpdater::MakeMoveUpdate(const Move& move)
{
	m_RemainingDepth--;
	m_SearchDepth++;
	CommonIncrementalUpdater::MakeMoveUpdate<sideToMove>(move);
}

forceinline constexpr void SearchIncrementalUpdater::UndoMoveUpdate()
{
	m_RemainingDepth++;
	m_SearchDepth--;
	CommonIncrementalUpdater::UndoMoveUpdate();
}
