// To test:
// ../../cppcheck --suppressions-list=suppressions.txt --dump misra-suppressions*-test.c && python ../misra.py misra-suppressions*-test.c.dump
// There should be no violations reported

// This needs to stay at line number 7 to make the test pass
// If it is changed update suppressions.txt with the new line number
// cppcheck-suppress-file misra-c2012-5.2
#include <stdio.h> //21.6

extern int misra_5_2_var_hides_var______31x;//8.4
static int misra_5_2_var_hides_var______31y;//5.2
static int misra_5_2_function_hides_var_31x;
static void misra_5_2_function_hides_var_31y(void) {}//5.2
static void foo(void)
{
  int i;
  // cppcheck-suppress-begin misra-c2012-16.4
  // cppcheck-suppress misra-c2012-16.6
  switch(misra_5_2_func1()) //16.4 16.6
  {
    case 1:
    {
      do
      {
        for(i = 0; i < 10; i++)
        {
          if(misra_5_2_func3()) //17.3
          {
            int misra_5_2_var_hides_var_1____31x;
            int misra_5_2_var_hides_var_1____31y;//5.2
          }
        }
      } while(misra_5_2_func2()); //17.3
    }
  }
  // cppcheck-suppress-end misra-c2012-16.4
}
