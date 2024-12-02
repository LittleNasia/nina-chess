#pragma once
#include "utils.h"

#include "common_incremental_updater.h"
#include "search_incremental_updater.h"

class UciIncrementalUpdater : private CommonIncrementalUpdater
{
public:
	UciIncrementalUpdater(Evaluator* evaluator, PositionStack* positionStack, const Position& currentPos) :
		CommonIncrementalUpdater(evaluator, positionStack)
	{
		positionStack->Reset(currentPos);
		evaluator->Reset(*positionStack);
	}

	forceinline constexpr PositionStack& GetPositionStack() { return CommonIncrementalUpdater::GetPositionStack(); }
	template<Color sideToMove>
	forceinline constexpr void FullUpdate(const Move& move);
	forceinline SearchIncrementalUpdater CreateSearchIncrementalUpdater(int64_t depthToSearchTo);
};

template<Color sideToMove>
forceinline constexpr void UciIncrementalUpdater::FullUpdate(const Move& move)
{
	MakeMoveUpdate<sideToMove>(move);
	MoveGenerationUpdateWithoutGuard<GetOppositeColor<sideToMove>()>();
}

forceinline SearchIncrementalUpdater UciIncrementalUpdater::CreateSearchIncrementalUpdater(int64_t depthToSearchTo)
{
	return SearchIncrementalUpdater(*this, depthToSearchTo);
}