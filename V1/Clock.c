// V1
#include "Clock.h"
#include "Processor.h"
// #include "ComputerSystemBase.h"

int tics=0;

void Clock_Update() {
	tics++;
    // ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,97,CLOCK,tics);
}


int Clock_GetTime() {

	return tics;
}
