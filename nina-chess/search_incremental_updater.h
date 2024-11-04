#pragma once
#include "utils.h"

#include "common_incremental_updater.h"

class SearchIncrementalUpdater: public CommonIncrementalUpdater
{
public:
	SearchIncrementalUpdater(CommonIncrementalUpdater& commonIncrementalUpdater, int64_t depthToSearchTo) :
		CommonIncrementalUpdater(commonIncrementalUpdater),
		m_RemainingDepth(depthToSearchTo),
		m_SearchDepth(0)
	{
	}

	template<Color sideToMove>
	forceinline constexpr void MakeMoveUpdate(const Move& move);
	forceinline constexpr void UndoMoveUpdate();

	forceinline constexpr int64_t GetRemainingDepth() const { return m_RemainingDepth; }
	forceinline constexpr int64_t GetSearchDepth() const { return m_SearchDepth;  }
	
private:
	int64_t m_RemainingDepth;
	int64_t m_SearchDepth;
};


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