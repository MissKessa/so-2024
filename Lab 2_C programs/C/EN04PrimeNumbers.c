#include <stdio.h>
#include <stdlib.h>

main (int argc, char *argv[]) {

  int primeNumbers[]={1,2,3,5,7,11,13,17,19,23,29};
  int position;

  if (argc==1)
    printf("The list contains the %d first prime numbers\n",sizeof(primeNumbers)/sizeof(int));
  else {
    position=atoi(argv[1]);
    printf("The prime number located in position number %d is %d\n",position, primeNumbers[position]);
    int sum;
    int i;
    if(position<=sizeof(primeNumbers)/sizeof(int)){
      for(i=0; i<position; i+=1){
        sum+=primeNumbers[i];
      }
      printf("The sum until that position %d\n",sum);
    }
  }
  
  exit(1);
}


// 1. Compile using "gcc EN04PrimeNumbers.c"
// 2. Execute using "./a.out"
// 3. Get help for atoi: "man atoi"
// 4. Modify the program so that it calculates and shows the sum of all the prime numbers in the list but the one in the position given in the command line
