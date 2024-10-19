#pragma once
#include "utils.h"

#include "common_incremental_updater.h"

class SearchIncrementalUpdater: public CommonIncrementalUpdater
{
public:
	SearchIncrementalUpdater(CommonIncrementalUpdater& common_incremental_updater, int64_t depth_to_search_to) :
		CommonIncrementalUpdater(common_incremental_updater),
		remaining_depth(depth_to_search_to),
		search_depth(0)
	{
	}

	template<Color side_to_move>
	forceinline constexpr void MakeMoveUpdate(const Move& move);
	forceinline constexpr void UndoMoveUpdate();

	forceinline constexpr int64_t GetRemainingDepth() const { return remaining_depth; }
	forceinline constexpr int64_t GetSearchDepth() const { return search_depth;  }
	
private:
	int64_t remaining_depth;
	int64_t search_depth;
};


template<Color side_to_move>
forceinline constexpr void SearchIncrementalUpdater::MakeMoveUpdate(const Move& move)
{
	remaining_depth--;
	search_depth++;
	CommonIncrementalUpdater::MakeMoveUpdate<side_to_move>(move);
}

forceinline constexpr void SearchIncrementalUpdater::UndoMoveUpdate()
{
	remaining_depth++;
	search_depth--;
	CommonIncrementalUpdater::UndoMoveUpdate();
}