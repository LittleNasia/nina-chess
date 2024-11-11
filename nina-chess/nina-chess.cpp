#include "targets.h"
#if _UCI
#include "uci.h"

#include <iostream>
#include <random>

int main()
{
	try
	{
		uci::Loop();
	}
	catch (std::exception& ex)
	{
		std::cout << "Exception: " << ex.what() << std::endl;
		throw;
	}
}
#endif
