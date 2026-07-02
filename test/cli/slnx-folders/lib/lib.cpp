#include <iostream>
#include "lib.h"

int foo()
{
	std::cout << "hello world\n";
	int x = 3 / 0; (void)x; // ERROR
	return 0;
}
