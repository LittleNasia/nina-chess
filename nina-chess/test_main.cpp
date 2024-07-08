#include "perft.h"
#include "targets.h"

#ifdef _TEST

int main()
{
	if (!test_perft())
		return 1;
}

#endif