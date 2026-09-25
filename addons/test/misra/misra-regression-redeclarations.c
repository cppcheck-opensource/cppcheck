#include "misra-regression-redeclarations.h"

int prior_extern = 1;
int tentative;
int tentative = 2;
int missing = 3;
static int internal;
extern int internal;
static int internal;
int incomplete[4];
extern int duplicate_decl;
extern int duplicate_decl;
int duplicate_decl = 5;
int two_definitions = 7;
int elsewhere = 8;
int local_only;
int local_only = 9;
extern int declarations_only;
extern int declarations_only;
extern int initialized_extern;
extern int initialized_extern = 11;

static int read_main(void)
{
    return local_only + local_only + prior_extern + incomplete[0] +
           duplicate_decl + two_definitions + elsewhere;
}
