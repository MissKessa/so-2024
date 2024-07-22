#include "EN10States.h"

char * stateName[]={"READY", "EXECUTING", "BLOCKED","EXIT"};

char * transitionName[]={"DISPATCH", "BLOCK", "WAKEUP", "RELEASE"};


int changeState(int currentState, int ocurredEvent) {
  
  int nextState=currentState;
  
  switch (currentState) {
    
    case READY: if (ocurredEvent==DISPATCH) nextState=EXECUTING;
      break;
      
    case EXECUTING: if (ocurredEvent==RELEASE) nextState=EXIT;
      else
	if (ocurredEvent==BLOCK) nextState=BLOCKED;
      break;
      
    case BLOCKED: if (ocurredEvent==WAKEUP) nextState=READY;
      break;
  }
  
  if (currentState==RELEASE) nextState=EXIT;
  if(currentState==nextState) printf("False!");
  
  return(nextState);
}

char * getStateName(int state) {
  
      return stateName[state];
}


char * getTransitionName(int transition) {
  
      return transitionName[transition];
}
