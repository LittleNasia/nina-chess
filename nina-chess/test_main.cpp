#include "Core/Build/targets.h"
#ifdef _TEST
#include "NN/dense_layer_test.h"
#include "GameGeneration/game_generation_test.h"
#include "Search/perft.h"

int main()
{
	if (!TestDenseLayer())
		return 1;
	if (!TestPerft(false, _PERFTNODES))
		return 1;
	if (!TestSearch(false))
		return 1;
	if (!TestGameGeneration())
		return 1;
}

#endif
