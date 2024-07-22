#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEFAULT_STRING "Fernando Alvarez Garcia"

#define DEFAULT_COMMAND_LINE_POSITION 1

main (int argc, char *argv[]) {
  
  char string[]=DEFAULT_STRING;
  
  if (argc==2)
      strcpy(string, argv[DEFAULT_COMMAND_LINE_POSITION]);
  
  toUpperCase(string);
  printf("Now, the string is %s\n",string);
  
  exit(1);
}

void toUpperCase(char string[]) {
  
  int i;
  int length=strlen(string);
  
  for (i=0; i<length;i++)
    if(string[i]>= 'a' && string[i]<='j'){
      string[i]=toupper(string[i]);
    }
}


// 1. Compile using "gcc EN05ToUpperCase.c"
// 2. Execute using "./a.out"
// 3. Get help for strcpy, strlen, toupper: "man strcpy", "man strlen", "man toupper"
// 4. Modify the program so that it only capitalizes the first 10 letters of the alphabet
