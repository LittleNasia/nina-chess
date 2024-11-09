#include "perft.h"
#include "targets.h"

#include <thread>

#ifdef _BENCH
int main()
{
	constexpr bool hidePerftOutput = true;
	constexpr size_t benchNodeLimit = _PERFTNODES;

	constexpr uint32_t numWarmupRuns = 10;
	for (uint32_t warmupRunIndex = 0; warmupRunIndex < numWarmupRuns; warmupRunIndex++)
	{
		TestPerft(hidePerftOutput, benchNodeLimit);
	}

	size_t nps{ 0 };
	constexpr uint32_t numPerftBenchRuns = 30;
	for(uint32_t runIndex = 0; runIndex < numPerftBenchRuns; runIndex++)
	{
		size_t currRunNps = TestPerft(hidePerftOutput, benchNodeLimit);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		
		nps += currRunNps;
	}

	std::cout << nps / numPerftBenchRuns << std::endl;

	constexpr bool hideSearchTestOutput = true;
	constexpr size_t benchDepthLimit = 5;
	for (uint32_t warmupRunIndex = 0; warmupRunIndex < numWarmupRuns; warmupRunIndex++)
	{
		TestSearch(hideSearchTestOutput, benchDepthLimit);
	}

	nps = 0;
	constexpr uint32_t numSearchBenchRuns = 15;
	for (uint32_t runIndex = 0; runIndex < numSearchBenchRuns; runIndex++)
	{
		size_t currRunNps = TestSearch(hideSearchTestOutput, benchDepthLimit);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		nps += currRunNps;
	}

	std::cout << nps / numSearchBenchRuns << std::endl;
}
#endif