#pragma once
#include "search_constraints.h"
#include "utils.h"

class SearchNodesCancellationPolicy
{
public:
	SearchNodesCancellationPolicy(const SearchConstraints& searchConstraints) : m_NodeLimit(getNodeLimit(searchConstraints)) {}

	forceinline constexpr uint64_t GetNodeLimit() const { return m_NodeLimit; }
	forceinline constexpr bool ShouldAbort(uint64_t nodes) const { return nodes >= m_NodeLimit; }

private:
	forceinline constexpr uint64_t getNodeLimit(const SearchConstraints& search_constraints);
	
	uint64_t m_NodeLimit;
};

forceinline constexpr uint64_t SearchNodesCancellationPolicy::getNodeLimit(const SearchConstraints& searchConstraints)
{
	uint64_t nodeLimit = searchConstraints.Nodes == -1
		? std::numeric_limits<uint64_t>::max()
		: searchConstraints.Nodes;

	return nodeLimit;
}
