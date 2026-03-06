#include "targets.h"
#ifdef _TEST
#include "game_generation_test.h"
#include "perft.h"

int main()
{
	if (!TestPerft(false, _PERFTNODES))
		return 1;
	if (!TestSearch(false))
		return 1;
	if (!TestGameGeneration())
		return 1;
}

#endif
