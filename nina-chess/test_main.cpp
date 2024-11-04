#include "perft.h"
#include "targets.h"

#ifdef _TEST

int main()
{
	if (!TestPerft(false, _PERFTNODES))
		return 1;
}

#endif