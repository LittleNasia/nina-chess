#include "perft.h"
#include "targets.h"

#ifdef _TEST

int main()
{
	if (!test_perft(false, 8031647685ULL - 1))
		return 1;
}

#endif