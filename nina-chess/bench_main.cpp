#include "perft.h"
#include "targets.h"

#ifdef _BENCH
int main()
{
	constexpr bool hidePerftOutput = true;
	constexpr size_t benchNodeLimit = 10000000;

	constexpr uint32_t numWarmupRuns = 2;
	for (uint32_t warmupRunIndex = 0; warmupRunIndex < numWarmupRuns; warmupRunIndex++)
	{
		size_t nps = test_perft(hidePerftOutput, benchNodeLimit);
	}

	size_t nps{ 0 };
	constexpr uint32_t numBenchRuns = 3;
	for(uint32_t runIndex = 0; runIndex < numBenchRuns; runIndex++)
	{
		size_t currRunNps = test_perft(hidePerftOutput, benchNodeLimit);
		
		nps += currRunNps;
	}

	std::cout << nps / numBenchRuns << std::endl;
}
#endif