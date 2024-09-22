#pragma once
#include "utils.h"

#include "search_constraints.h"

class SearchNodesCancellationPolicy
{
public:
	SearchNodesCancellationPolicy(const SearchConstraints& search_constraints) :
		node_limit(getNodeLimit(search_constraints))
	{}

	forceinline constexpr bool ShouldAbort(uint64_t nodes) const { return nodes >= node_limit; }
	forceinline constexpr uint64_t GetNodeLimit() const { return node_limit; }

private:
	forceinline constexpr uint64_t getNodeLimit(const SearchConstraints& search_constraints);
	
	uint64_t node_limit;
};

inline constexpr uint64_t SearchNodesCancellationPolicy::getNodeLimit(const SearchConstraints& search_constraints)
{
	uint64_t nodeLimit = search_constraints.nodes == -1
		? std::numeric_limits<uint64_t>::max()
		: search_constraints.nodes;

	return nodeLimit;
}