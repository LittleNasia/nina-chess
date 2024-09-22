#include "targets.h"
#include "uci.h"

#include <iostream>
#include <random>


#if _UCI
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
