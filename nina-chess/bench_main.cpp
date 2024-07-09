#include "perft.h"
#include "targets.h"

#ifdef _BENCH
int main()
{
	constexpr bool hidePerftOutput = true;
	constexpr size_t benchNodeLimit = 8031647685ULL - 1;

	constexpr uint32_t numWarmupRuns = 0;
	for (uint32_t warmupRunIndex = 0; warmupRunIndex < numWarmupRuns; warmupRunIndex++)
	{
		size_t nps = test_perft(hidePerftOutput, benchNodeLimit);
	}

	size_t nps{ 0 };
	constexpr uint32_t numBenchRuns = 1;
	for(uint32_t runIndex = 0; runIndex < numBenchRuns; runIndex++)
	{
		size_t currRunNps = test_perft(hidePerftOutput, benchNodeLimit);
		
		nps += currRunNps;
	}

	std::cout << nps / numBenchRuns << std::endl;
}
#endif