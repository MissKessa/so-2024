#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EN10States.h"

main (int argc, char *argv[]) {
  
  int state;
  int transition;
  int i;
  
  if (argc<2) {
    printf("Syntax: %s StateNumber TransitionNumber\n",argv[0]);
    for (i=0;i < NUMBER_OF_STATES;i++)
      printf("%s (%d),",getStateName(i),i);
    printf("\n");
    for (i=0;i < NUMBER_OF_TRANSITIONS;i++)
      printf("%s (%d),",getTransitionName(i),i);
    printf("\n");
  }    
  else {
    state=atoi(argv[1]);
    transition=atoi(argv[2]);
    printf("Transition %s moves the process from the %s state to the %s state\n", getTransitionName(transition),getStateName(state),getStateName(changeState(state,transition)));
  }
  exit(1);
}

// 1. Compile using "gcc EN10States.c EN10MainStates.c"
// 2. Execute using "./a.out"
// 3. Modify the changeState function to add a RELEASE transition that always moves a process to the EXIT state
// 4. Modify the program so that it detects fake transitions, that is, transitions that not change the process state. Show a special message when detected.

