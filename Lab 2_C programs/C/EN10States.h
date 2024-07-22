#define NUMBER_OF_STATES 4

#define NUMBER_OF_TRANSITIONS 4

enum states {READY, EXECUTING, BLOCKED,EXIT};

enum events {DISPATCH, BLOCK, WAKEUP, RELEASE};


int changeState(int currentState, int ocurredEvent);

char * getStateName(int);

char * getTransitionName(int);

