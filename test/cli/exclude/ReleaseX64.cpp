#include <iostream>

int foo()
{
	std::cout << "ReleaseX64\n";
	int x = 3 / 0; (void)x; // ERROR
	return 0;
}
