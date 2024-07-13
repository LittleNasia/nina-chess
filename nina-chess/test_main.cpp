#include "perft.h"
#include "targets.h"

#ifdef _TEST

int main()
{
	std::cout << "doing nodes " << _PERFTNODES << "\n";
	if (!test_perft(false, _PERFTNODES))
		return 1;
}

#endif