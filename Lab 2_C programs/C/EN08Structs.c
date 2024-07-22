#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXIMUMSIZE 20

struct person {
  char name[MAXIMUMSIZE];
  int age;
  float income;
};

main (int argc, char *argv[]) {
  if (argc < 3)
    return 0;
  char *endptr;
  
  struct person teacher = {"", atoi(argv[2]), strtof(argv[3], &endptr)};
  strcpy(teacher.name,argv[1]);
  struct person student;
  
  strcpy(student.name,"Juan");
  student.age=19;
  student.income=11.777;

  printf("Teacher: %s Age: %d Income: %f\n",teacher.name, teacher.age, teacher.income);
  printf("The overall income of the teacher and the student is %f\n",student.income+teacher.income);
  exit(1);
}

// 1. Compile using "gcc EN08Structs.c"
// 2. Execute using "./a.out"
// 3. Modify the programa so that the teacher's name, age and income are obtained from the command line 


