// V1
#include "Clock.h"
#include "Processor.h"
#include "ComputerSystemBase.h"

int tics=0;
int numberOfClockInterrupts=0;
void OperatingSystem_InterruptLogic(int entryPoint);

void Clock_Update() { //V2-1d
	tics++;
	if(tics%intervalBetweenInterrupts==0){
		numberOfClockInterrupts++;
		int userMode=0;
		if(!Processor_PSW_BitState(EXECUTION_MODE_BIT)){
			userMode=1;
			Processor_ActivatePSW_Bit(EXECUTION_MODE_BIT);
		}
		// Processor_ActivatePSW_Bit(INTERRUPT_MASKED_BIT); 
		OperatingSystem_InterruptLogic(CLOCKINT_BIT);
		// Processor_DeactivatePSW_Bit(INTERRUPT_MASKED_BIT);
		if(userMode==1){
			Processor_DeactivatePSW_Bit(EXECUTION_MODE_BIT);
		}
	}
}


int Clock_GetTime() {
	return tics;
}

int Clock_GetNumberOfClockInterrupts() {
	return numberOfClockInterrupts;
}
