#include "../lib/lib.h"

int main(int argc, char *argv[])
{
	int x = 3 / 0; (void)x; // ERROR
	return foo();
}
