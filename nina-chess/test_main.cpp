#include "perft.h"
#include "targets.h"

#ifdef _TEST

int main()
{
	if (!test_perft(false, _PERFTNODES))
		return 1;
}

#endif