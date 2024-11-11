#include "targets.h"
#ifdef _BENCH
#include "perft.h"

#include <thread>

int main()
{
	constexpr bool hidePerftOutput = true;
	constexpr size_t benchNodeLimit = _PERFTNODES;

	constexpr uint32_t numPerftWarmupRuns = 5;
	for (uint32_t warmupRunIndex = 0; warmupRunIndex < numPerftWarmupRuns; warmupRunIndex++)
	{
		TestPerft(hidePerftOutput, benchNodeLimit);
	}

	size_t nps{ 0 };
	constexpr uint32_t numPerftBenchRuns = 15;
	for(uint32_t runIndex = 0; runIndex < numPerftBenchRuns; runIndex++)
	{
		size_t currRunNps = TestPerft(hidePerftOutput, benchNodeLimit);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		
		nps += currRunNps;
	}

	std::cout << nps / numPerftBenchRuns << std::endl;

	constexpr bool hideSearchTestOutput = true;
	constexpr uint32_t numSearchWarmupRuns = 1;
	constexpr size_t benchDepthLimit = 7;
	for (uint32_t warmupRunIndex = 0; warmupRunIndex < numSearchWarmupRuns; warmupRunIndex++)
	{
		TestSearch(hideSearchTestOutput, benchDepthLimit);
	}

	nps = 0;
	constexpr uint32_t numSearchBenchRuns = 2;
	for (uint32_t runIndex = 0; runIndex < numSearchBenchRuns; runIndex++)
	{
		size_t currRunNps = TestSearch(hideSearchTestOutput, benchDepthLimit);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		nps += currRunNps;
	}

	std::cout << nps / numSearchBenchRuns << std::endl;
}
#endif