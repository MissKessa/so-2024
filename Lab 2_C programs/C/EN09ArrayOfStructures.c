#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXIMUMSIZE 20

#define NUMBER_OF_STUDENTS 3

struct person {
  char name[MAXIMUMSIZE];
  int age;
  float income;
};

float averageIncome(struct person []);

main (int argc, char *argv[]) {
  
  struct person students[NUMBER_OF_STUDENTS]={{"Juan", 23, 777.11}, {"Luis",19,111.1}, {"Pedro",56,9327.2}};
  
  printf("The average income of the students is %f\n", averageIncome(students));
  printf("Position of youngest is %d\n", youngestStudent(students));
  exit(1);
}

float averageIncome(struct person people[]) {
  
  int i;
  float accumulator=0;
    
  for (i=0;i<NUMBER_OF_STUDENTS;i++)
    accumulator += people[i].income;
  
  return accumulator/NUMBER_OF_STUDENTS;
}

int youngestStudent(struct person people[]) {
  int i;
  int posYoungest=0;
  char* nameYoungest= people[0].name;
  int ageYoungest= people[0].age;
  
  for (i=1;i<NUMBER_OF_STUDENTS;i++){
    if (people[i].age<ageYoungest){
      ageYoungest=people[i].age;
      nameYoungest=people[i].name;
      posYoungest=i;
    }
  }
  printf("Youngest: %s\n", nameYoungest);
  return posYoungest;
}
  
// 1. Compile using "gcc EN09ArrayOfStructures.c"
// 2. Execute using "./a.out"
// 3. Add a second function that shows the name of the youngest student and returns the position of that student in the array

