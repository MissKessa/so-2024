#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "ComputerSystem.h"
#include "OperatingSystem.h"
#include "ComputerSystemBase.h"
#include "Processor.h"
#include "Messages.h"
#include "Asserts.h"
#include "Wrappers.h"

extern int COLOURED;
extern char *debugLevel;

// Functions prototypes
void ComputerSystem_PrintProgramList();
int OperatingSystem_GetNumberOfNotTerminatedUserProcess();
void OperatingSystem_TerminateExecutingProcess();

// Powers on of the Computer System.
void ComputerSystem_PowerOn(int argc, char *argv[], int paramIndex) {
	unsigned int i;
	// To remember the simulator sections to be message-debugged
	for (i=0; i< strlen(debugLevel);i++) {
		if (isupper(debugLevel[i])){
		COLOURED = 1;
		debugLevel[i]=tolower(debugLevel[i]);
		}
	}

	// Load debug messages
	int nm=0;
	nm=Messages_Load_Messages(nm,TEACHER_MESSAGES_FILE);
	if (nm<0) {
		ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,64,SHUTDOWN,TEACHER_MESSAGES_FILE);
		exit(2);
	}
	nm=Messages_Load_Messages(nm,STUDENT_MESSAGES_FILE);

	// Prepare if necesary the assert system
	Asserts_LoadAsserts();

	ComputerSystem_DebugMessage(TIMED_MESSAGE, 99, POWERON, "STARTING simulation\n");
	// Obtain a list of programs in the command line
	int programsFromFilesBaseIndex = ComputerSystem_ObtainProgramList(argc, argv, paramIndex);


	ComputerSystem_PrintProgramList(); //V1-2
	// Request the OS to do the initial set of tasks. The last one will be
	// the processor allocation to the process with the highest priority

	OperatingSystem_Initialize(programsFromFilesBaseIndex);
	
	// Tell the processor to begin its instruction cycle 

	//int j;

	//for(j=1; programList[j]!=NULL && j<PROGRAMSMAXNUMBER ; j++) {    
		//Processor_DeactivatePSW_Bit(POWEROFF_BIT);
		//Processor_InstructionCycleLoop();
		//int selectedProcess=OperatingSystem_ShortTermScheduler();
		//if(selectedProcess==0)
			//break;
		//ComputerSystem_DebugMessage(TIMED_MESSAGE, 102, INIT, "selected", selectedProcess);
		//OperatingSystem_Dispatch(selectedProcess);
		//Processor_SetPC(0);
		//Processor_SetAccumulator(0);
	//}
	int numberOfNotTerminatedUserProcesses=OperatingSystem_GetNumberOfNotTerminatedUserProcess();
	while(numberOfNotTerminatedUserProcesses!=0){
		Processor_DeactivatePSW_Bit(POWEROFF_BIT);
		Processor_InstructionCycleLoop();
		OperatingSystem_TerminateExecutingProcess();

		//int PID=OperatingSystem_GetExecutingProcessID();
		//ComputerSystem_DebugMessage(TIMED_MESSAGE, 102, INIT, "selected", PID);
		//if (PID==NOPROCESS)
		//	break;
		//Processor_SetPC(0);
		//Processor_SetAccumulator(0);
		numberOfNotTerminatedUserProcesses=OperatingSystem_GetNumberOfNotTerminatedUserProcess();
	}


}

// Powers off the CS (the C program ends)
void ComputerSystem_PowerOff() {
	// Show message in red colour: "END of the simulation\n" 
	ComputerSystem_DebugMessage(TIMED_MESSAGE,99,SHUTDOWN,"END of the simulation\n");  
	exit(0);
}

/////////////////////////////////////////////////////////
//  New functions below this line  //////////////////////
void ComputerSystem_PrintProgramList(){ //V1-1
	ComputerSystem_DebugMessage(TIMED_MESSAGE,101,INIT);
	int i;
	for (i=1; programList[i]!=NULL && i<PROGRAMSMAXNUMBER ; i++){
		PROGRAMS_DATA *executableProgram=programList[i];
		int arrival_time= executableProgram->arrivalTime;
		ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,102,INIT,executableProgram->executableName,arrival_time);
	}
}