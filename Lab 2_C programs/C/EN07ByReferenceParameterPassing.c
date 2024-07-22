#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_STRING "Fernando"

#define DEFAULT_COMMAND_LINE_POSITION 1

main (int argc, char *argv[]) {
  
  int length=strlen(DEFAULT_STRING);
  
  if (argc==2)
    length=strlen(argv[DEFAULT_COMMAND_LINE_POSITION]);
  
  substituteAnIntegerByItsSquare(length);
  
  printf("The string length squared is %d\n",length);
  exit(1);
}

void substituteAnIntegerByItsSquare(int length) { 

  length *= length; 

} 

// 1. Compile using "gcc EN07ByReferenceParameterPassing.c"
// 2. Execute using "./a.out"
// 3. Implement a similar function passing the argument "length" by value; study how different the program behaves

