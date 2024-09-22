#pragma once
#include "utils.h"

#include "common_incremental_updater.h"
#include "search_incremental_updater.h"

class UciIncrementalUpdater : private CommonIncrementalUpdater
{
public:
	UciIncrementalUpdater(Evaluator* evaluator, PositionStack* position_stack, const Position& pos) :
		CommonIncrementalUpdater(evaluator, position_stack)
	{
		position_stack->Reset(pos);
		evaluator->Reset(*position_stack);
	}

	forceinline constexpr PositionStack& GetPositionStack() { return CommonIncrementalUpdater::GetPositionStack(); }
	template<Color side_to_move>
	forceinline constexpr void FullUpdate(const Move& move);
	forceinline SearchIncrementalUpdater CreateSearchIncrementalUpdater(int64_t depth_to_search_to);
};

template<Color side_to_move>
inline constexpr void UciIncrementalUpdater::FullUpdate(const Move& move)
{
	MakeMoveUpdate<side_to_move>(move);
	MoveGenerationUpdateWithoutGuard<get_opposite_color<side_to_move>()>();
}

inline SearchIncrementalUpdater UciIncrementalUpdater::CreateSearchIncrementalUpdater(int64_t depth_to_search_to)
{
	return SearchIncrementalUpdater(*this, depth_to_search_to);
}