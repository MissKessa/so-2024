#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CHAR 'a'
#define DEFAULT_STRING "Fernando Alvarez Garcia"

#define STRING_COMMAND_LINE_POSITION 1
#define CHAR_COMMAND_LINE_POSITION 2
int searchAndCount(char character, char *string);

main (int argc, char *argv[]) {
  int counter=0;
  if (argc==1)
    counter=searchAndCount(DEFAULT_CHAR, DEFAULT_STRING);
  else
    if (argc==2)
      counter=searchAndCount(DEFAULT_CHAR, argv[STRING_COMMAND_LINE_POSITION]);
    else
      counter=searchAndCount(argv[CHAR_COMMAND_LINE_POSITION][0], argv[STRING_COMMAND_LINE_POSITION]);
      
  printf("Counter: %d\n",counter);
  exit(1);
}

int searchAndCount(char character, char *string) {
  int counter=0;
  int i;
  int length=strlen(string);
  
  printf("Searching in string %s\n",string);
  for (i=0;i<length;i++) {
    if (string[i]==character){
      printf("Position %d of the string contains character [%c]\n",i,character);
      counter++;
    }
  }
  return counter;
}
  
// 1. Compile using "gcc EN06LookForAChar.c"
// 2. Execute using "./a.out"
// 3. Modify the searchAndCount function so that it counts and returns the number of matching characters in the string

