#include "targets.h"
#ifdef _TEST
#include "perft.h"

int main()
{
	if (!TestPerft(false, _PERFTNODES))
		return 1;
	if (!TestSearch(false))
		return 1;
}

#endif