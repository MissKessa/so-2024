#include "Simulator.h"
#include "OperatingSystem.h"
#include "OperatingSystemBase.h"
#include "MMU.h"
#include "Processor.h"
#include "Buses.h"
#include "Heap.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

// Functions prototypes
void OperatingSystem_PCBInitialization(int, int, int, int, int);
void OperatingSystem_MoveToTheREADYState(int);
void OperatingSystem_Dispatch(int);
void OperatingSystem_RestoreContext(int);
void OperatingSystem_SaveContext(int);
void OperatingSystem_TerminateExecutingProcess();
int OperatingSystem_LongTermScheduler();
int OperatingSystem_CreateProcess(int);
int OperatingSystem_ObtainMainMemory(int,int, int);
int OperatingSystem_ShortTermScheduler();
int OperatingSystem_ExtractFromReadyToRun();
void OperatingSystem_HandleException();
void OperatingSystem_HandleSystemCall();
void OperatingSystem_PrintReadyToRunQueue();
int OperatingSystem_GetNumberOfNotTerminatedUserProcess();
void OperatingSystem_HandleClockInterrupt();
void OperatingSystem_SleepRunningProcess();
void OperatingSystem_MoveToTheBLOCKEDState(int PID);
int Processor_GetRegisterD();
void Processor_SetRegisterD(int reg);
int Clock_GetNumberOfClockInterrupts();
void OperatingSystem_AwakeProcess(int PID);
void OperatingSystem_MoveToTheReadyStateDAEMON(int PID);
void OperatingSystem_ReleaseMainMemory(int PID);
void Clock_IncreaseNumberOfClockInterrupts(); //ERROR 9/05


// The process table
// PCB processTable[PROCESSTABLEMAXSIZE];
PCB * processTable;

// Size of the memory occupied for the OS
int OS_MEMORY_SIZE=32;

// Address base for OS code in this version
int OS_address_base; 

// Identifier of the current executing process
int executingProcessID=NOPROCESS;

// Identifier of the System Idle Process
int sipID;

// Initial PID for assignation (Not assigned)
int initialPID=-1;

// Begin indes for daemons in programList
// int baseDaemonsInProgramList; 

// Array that contains the identifiers of the READY processes
// heapItem readyToRunQueue[NUMBEROFQUEUES][PROCESSTABLEMAXSIZE];
heapItem *readyToRunQueue[NUMBEROFQUEUES];
//int numberOfReadyToRunProcesses[NUMBEROFQUEUES]={0};
int numberOfReadyToRunProcesses[NUMBEROFQUEUES]={0,0}; //V1-11a
char * queueNames [NUMBEROFQUEUES]={"USER","DAEMONS"}; //V1-11a

// Variable containing the number of not terminated user processes
int numberOfNotTerminatedUserProcesses=0;

// char DAEMONS_PROGRAMS_FILE[MAXFILENAMELENGTH]="teachersDaemons";

int MAINMEMORYSECTIONSIZE = 60;

extern int MAINMEMORYSIZE;

int PROCESSTABLEMAXSIZE = 4;

char * statesNames[5]={"NEW","READY","EXECUTING","BLOCKED","EXIT"}; //V1-10a
char * exceptionsCauses[4] ={"division by zero", "invalid processor mode", "invalid address", "invalid instruction"};//V4-2


// In OperatingSystem.c Exercise 5-b of V2
// Heap with blocked processes sorted by when to wakeup
heapItem *sleepingProcessesQueue; //V2-5b
int numberOfSleepingProcesses=0; //V2-5b


int OperatingSystem_GetNumberOfNotTerminatedUserProcess(){
  return numberOfNotTerminatedUserProcesses;
}

// Initial set of tasks of the OS
void OperatingSystem_Initialize(int programsFromFileIndex) {
	
	int i, selectedProcess;
	FILE *programFile; // For load Operating System Code
	
// In this version, with original configuration of memory size (300) and number of processes (4)
// every process occupies a 60 positions main memory chunk 
// and OS code and the system stack occupies 60 positions 

	OS_address_base = MAINMEMORYSIZE - OS_MEMORY_SIZE;

	MAINMEMORYSECTIONSIZE = OS_address_base / PROCESSTABLEMAXSIZE;

	if (initialPID<0) // if not assigned in options...
		initialPID=PROCESSTABLEMAXSIZE-1; //V1-8
	
	// Space for the processTable
	processTable = (PCB *) malloc(PROCESSTABLEMAXSIZE*sizeof(PCB));
	
	// Space for the ready to run queues (one queue initially...)
	//readyToRunQueue[ALLPROCESSESQUEUE] = Heap_create(PROCESSTABLEMAXSIZE);
	readyToRunQueue[USERPROCESSQUEUE] = Heap_create(PROCESSTABLEMAXSIZE); //V1-11c
	readyToRunQueue[DAEMONSQUEUE] = Heap_create(PROCESSTABLEMAXSIZE); //V1-11c
	
	
	sleepingProcessesQueue= Heap_create(PROCESSTABLEMAXSIZE); ////V2-5b

	programFile=fopen("OperatingSystemCode", "r");
	if (programFile==NULL){
		// Show red message "FATAL ERROR: Missing Operating System!\n"
		ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,99,SHUTDOWN,"FATAL ERROR: Missing Operating System!\n");
		exit(1);		
	}

	OperatingSystem_InitializePartitionTable(); //V4-5

	// Obtain the memory requirements of the program
	int processSize=OperatingSystem_ObtainProgramSize(programFile);

	// Load Operating System Code
	OperatingSystem_LoadProgram(programFile, OS_address_base, processSize);
	
	// Process table initialization (all entries are free)
	for (i=0; i<PROCESSTABLEMAXSIZE;i++){
		processTable[i].busy=0;
		processTable[i].programListIndex=-1;
		processTable[i].initialPhysicalAddress=-1;
		processTable[i].processSize=-1;
		processTable[i].copyOfSPRegister=-1;
		processTable[i].state=-1;
		processTable[i].priority=-1;
		processTable[i].queueID=-1;
		processTable[i].copyOfPCRegister=-1;
		processTable[i].copyOfPSWRegister=0;

		processTable[i].copyOfRegisterA=-1; //V1-13
		processTable[i].copyOfRegisterB=-1; //V1-13
		processTable[i].copyOfAccumulator=-1; //V1-13
	}
	// Initialization of the interrupt vector table of the processor
	Processor_InitializeInterruptVectorTable(OS_address_base+2);
		
	// Include in program list all user or system daemon processes
	OperatingSystem_PrepareDaemons(programsFromFileIndex);
	
	ComputerSystem_FillInArrivalTimeQueue(); //V3-1c
	OperatingSystem_PrintStatus(); //V3-1d

	// Create all user processes from the information given in the command line
	OperatingSystem_LongTermScheduler(); //int numberOfSuccessfullyCreatedProcesses=OperatingSystem_LongTermScheduler();
	
	
	if (strcmp(programList[processTable[sipID].programListIndex]->executableName,"SystemIdleProcess")
		&& processTable[sipID].state==READY) {
		// Show red message "FATAL ERROR: Missing SIP program!\n"
		ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,99,SHUTDOWN,"FATAL ERROR: Missing SIP program!\n");
		exit(1);		
	}

	//if (numberOfSuccessfullyCreatedProcesses==1)
	if(numberOfNotTerminatedUserProcesses==0 && numberOfProgramsInArrivalTimeQueue==0){ //V1-15   V3-4a numberOfProgramsInArrivalTimeQueue==0
		OperatingSystem_Dispatch(sipID);
		//executingProcessID=sipID;
		OperatingSystem_TerminateExecutingProcess();
	}

	// At least, one process has been created
	// Select the first process that is going to use the processor
	selectedProcess=OperatingSystem_ShortTermScheduler();

	Processor_SetSSP(MAINMEMORYSIZE-1);

	// Assign the processor to the selected process
	OperatingSystem_Dispatch(selectedProcess);

	// Initial operation for Operating System
	Processor_SetPC(OS_address_base);

	OperatingSystem_PrintStatus(); //V2-3a
}

// The LTS is responsible of the admission of new processes in the system.
// Initially, it creates a process from each program specified in the 
// 			command line and daemons programs
int OperatingSystem_LongTermScheduler() {
	int PID,
		numberOfSuccessfullyCreatedProcesses=0, numberOfFailedfullyCreatedProcesses=0;
	int i;
	
	//for (i=0; OperatingSystem_IsThereANewProgram()!=YES && programList[i]!=NULL && i<PROGRAMSMAXNUMBER ; i++) 	 
	while(OperatingSystem_IsThereANewProgram()==YES) { //V3-2
		i=Heap_poll(arrivalTimeQueue,QUEUE_ARRIVAL,&(numberOfProgramsInArrivalTimeQueue));
		
			
		PID=OperatingSystem_CreateProcess(i);

		if (PID==NOFREEENTRY){ //No more free entries in table for the program  V1-4b
			numberOfFailedfullyCreatedProcesses++;

			ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,103,ERROR,programList[i]->executableName);
		}
		else if(PID==PROGRAMDOESNOTEXIST){ //The program doesn't exist   V1-5b
			numberOfFailedfullyCreatedProcesses++;
			ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,104,ERROR,programList[i]->executableName, "it does not exist");
		}
		else if (PID==PROGRAMNOTVALID){ //The program has an invalid size or priority  V1-5c
			numberOfFailedfullyCreatedProcesses++;
			ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,104,ERROR,programList[i]->executableName, "invalid priority or size");
		}
		else if (PID==TOOBIGPROCESS){ //The program has a bigger size   V1-6b
			numberOfFailedfullyCreatedProcesses++;
			ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,105,ERROR,programList[i]->executableName);
		} else if (PID==MEMORYFULL) { //V4-6d
			numberOfFailedfullyCreatedProcesses++;
		}
		else {
			numberOfSuccessfullyCreatedProcesses++;
			if (programList[i]->type==USERPROGRAM) 
				numberOfNotTerminatedUserProcesses++;

			// Move process to the ready state
			OperatingSystem_MoveToTheREADYState(PID);
			
		}
		
	}

	if (OperatingSystem_IsThereANewProgram()==EMPTYQUEUE && numberOfNotTerminatedUserProcesses==0){ //ERROR V3 27/04/2024
        OperatingSystem_Dispatch(sipID);
        OperatingSystem_TerminateExecutingProcess();
    }


	if (numberOfSuccessfullyCreatedProcesses>=1)
		OperatingSystem_PrintStatus(); //V2-3e
 
	// Return the number of succesfully created processes
	return numberOfSuccessfullyCreatedProcesses;
}


// This function creates a process from an executable program
int OperatingSystem_CreateProcess(int indexOfExecutableProgram) {
	int PID;
	int processSize;
	int loadingPhysicalAddress;
	int priority;
	FILE *programFile;
	PROGRAMS_DATA *executableProgram=programList[indexOfExecutableProgram];

	// Obtain a process ID
	PID=OperatingSystem_ObtainAnEntryInTheProcessTable();
	if (PID==NOFREEENTRY){ //V1-4a
		return NOFREEENTRY;
	}

	// Check if programFile exists
	programFile=fopen(executableProgram->executableName, "r");

	if (programFile==NULL){ //V1-5b
		return PROGRAMDOESNOTEXIST;
	}
	// Obtain the memory requirements of the program
	processSize=OperatingSystem_ObtainProgramSize(programFile);	

	if (processSize==PROGRAMNOTVALID){ //V1-5c
		return PROGRAMNOTVALID;
	}
	// Obtain the priority for the process
	priority=OperatingSystem_ObtainPriority(programFile);
	
	if (priority==PROGRAMNOTVALID){ //V1-5c
		return PROGRAMNOTVALID;
	}

	// Obtain enough memory space
	int partitionID=OperatingSystem_ObtainMainMemory(indexOfExecutableProgram, processSize, PID);
	if (partitionID==TOOBIGPROCESS) //V1-7a
		return TOOBIGPROCESS;
	else if(partitionID==MEMORYFULL) //V4-6d
		return MEMORYFULL;

	loadingPhysicalAddress=0;
	if(partitionID!=0) {// not first partition V4-6c
		loadingPhysicalAddress=partitionsTable[partitionID-1].initAddress+partitionsTable[partitionID-1].size;
	}

	// Load program in the allocated memory
	int loadProgramSucess=OperatingSystem_LoadProgram(programFile, loadingPhysicalAddress, processSize);
	if (loadProgramSucess==TOOBIGPROCESS) //V1-6a
		return TOOBIGPROCESS;
	
	OperatingSystem_ShowPartitionTable("before allocating memory"); //V4-7
	//V4-6c
	ComputerSystem_DebugMessage(TIMED_MESSAGE,143,SYSMEM,partitionID,partitionsTable[partitionID].initAddress,partitionsTable[partitionID].size,PID,programList[indexOfExecutableProgram]->executableName);
	partitionsTable[partitionID].PID=PID;
	// PCB initialization
	OperatingSystem_PCBInitialization(PID, loadingPhysicalAddress, processSize, priority, indexOfExecutableProgram);
	OperatingSystem_ShowPartitionTable("after allocating memory"); //V4-7
	// Show message "Process [PID] created from program [executableName]\n"
	//ComputerSystem_DebugMessage(TIMED_MESSAGE,70,SYSPROC,PID,executableProgram->executableName); //V3-0
	//ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,111,SYSPROC,PID,executableProgram->executableName);
	
	return PID;
}

// Main memory is assigned in chunks. All chunks are the same size. A process
// always obtains the chunk whose position in memory is equal to the processor identifier
int OperatingSystem_ObtainMainMemory(int indexOfExecutableProgram, int processSize, int PID) {
	// if (processSize>MAINMEMORYSECTIONSIZE)
	// 	return TOOBIGPROCESS;
	
	// return PID*MAINMEMORYSECTIONSIZE;
	ComputerSystem_DebugMessage(TIMED_MESSAGE,142,SYSMEM,PID,programList[indexOfExecutableProgram]->executableName, processSize); //V4-6b
	//V4-6a
	int i;
	int bestFit=-1;
	int numberPart=0;
	for (i=0; i<PARTITIONTABLEMAXSIZE;i++){
		if(partitionsTable[i].size >= processSize && bestFit==-1){ //it fits, it's the first best fit
			if(partitionsTable[i].PID==NOPROCESS)
				bestFit=i;
			numberPart++;
		} else if(bestFit!=-1 && partitionsTable[i].size >= processSize && partitionsTable[i].size<partitionsTable[bestFit].size){
			if(partitionsTable[i].PID==NOPROCESS)
				bestFit=i;
			numberPart++;
		}
	}

	if(bestFit!=-1){
		return bestFit; //V4-6c
	}
	
	ComputerSystem_DebugMessage(TIMED_MESSAGE,144,ERROR,programList[indexOfExecutableProgram]->executableName); //V4-6d
	if(numberPart==0){ //V4-6d-i
		return TOOBIGPROCESS;
	} else { //V4-6d-ii
		return MEMORYFULL;
	}

}

void OperatingSystem_ReleaseMainMemory(int PID) {//V4-8
	int i;
	int partition=-1;
	for (i=0; i<PARTITIONTABLEMAXSIZE;i++){
		if(partitionsTable[i].PID == PID){ //it fits, it's the first best fit
			partition=i;
			break;
		}
	}

	ComputerSystem_DebugMessage(TIMED_MESSAGE,145,SYSMEM,partition,partitionsTable[partition].initAddress,partitionsTable[partition].size,PID,programList[processTable[PID].programListIndex]->executableName);
	OperatingSystem_ShowPartitionTable("before allocating memory");
	partitionsTable[partition].PID=NOPROCESS;
	OperatingSystem_ShowPartitionTable("after allocating memory");
}

// Assign initial values to all fields inside the PCB
void OperatingSystem_PCBInitialization(int PID, int initialPhysicalAddress, int processSize, int priority, int processPLIndex) {

	processTable[PID].busy=1;
	processTable[PID].initialPhysicalAddress=initialPhysicalAddress;
	processTable[PID].processSize=processSize;
	processTable[PID].copyOfSPRegister=initialPhysicalAddress+processSize;
	processTable[PID].state=NEW;
	processTable[PID].queueID=USERPROCESSQUEUE; //FIXED ERROR
	processTable[PID].priority=priority;
	processTable[PID].programListIndex=processPLIndex;

	processTable[PID].copyOfRegisterA=0; //V1-13
	processTable[PID].copyOfRegisterB=0; //V1-13
	processTable[PID].copyOfAccumulator=0; //V1-13
	// Daemons run in protected mode and MMU use real address
	if (programList[processPLIndex]->type == DAEMONPROGRAM) {
		processTable[PID].copyOfPCRegister=initialPhysicalAddress;
		processTable[PID].queueID=DAEMONSQUEUE; //FIXED ERROR
		processTable[PID].copyOfPSWRegister= ((unsigned int) 1) << EXECUTION_MODE_BIT;
	} 
	else {
		processTable[PID].copyOfPCRegister=0;
		processTable[PID].copyOfPSWRegister=0;
	}

}


// Move a process to the READY state: it will be inserted, depending on its priority, in
// a queue of identifiers of READY processes
void OperatingSystem_MoveToTheREADYState(int PID) {	
	if(processTable[PID].queueID==DAEMONSQUEUE || PID==sipID){ //ERROR 27/04/24
        OperatingSystem_MoveToTheReadyStateDAEMON(PID);
        return;
    }

	if(processTable[PID].state==NEW){
		if (Heap_add(PID, readyToRunQueue[USERPROCESSQUEUE],QUEUE_PRIORITY ,&(numberOfReadyToRunProcesses[USERPROCESSQUEUE]))>=0) { //V1-11c
			processTable[PID].state=READY;
			ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,111,SYSPROC,PID,statesNames[0],programList[processTable[PID].programListIndex]->executableName); //V1-10b
			//OperatingSystem_PrintReadyToRunQueue(); V1-9b  Commented in V2-4
		} 
	}
	else if (processTable[PID].state==EXECUTING){
		if (Heap_add(PID, readyToRunQueue[USERPROCESSQUEUE],QUEUE_PRIORITY ,&(numberOfReadyToRunProcesses[USERPROCESSQUEUE]))>=0) {
			processTable[PID].state=READY;
			ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,110,SYSPROC,PID,programList[processTable[PID].programListIndex]->executableName,statesNames[2],statesNames[1]);
			//OperatingSystem_PrintReadyToRunQueue();
		} 
	} else if(processTable[PID].state==BLOCKED){
		if (Heap_add(PID, readyToRunQueue[USERPROCESSQUEUE],QUEUE_PRIORITY ,&(numberOfReadyToRunProcesses[USERPROCESSQUEUE]))>=0) {
			processTable[PID].state=READY;
			ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,110,SYSPROC,PID,programList[processTable[PID].programListIndex]->executableName,statesNames[3],statesNames[1]);
		} 
	}
}

void OperatingSystem_MoveToTheReadyStateDAEMON(int PID){
    if(processTable[PID].state==NEW){
        if (Heap_add(PID, readyToRunQueue[DAEMONPROGRAM],QUEUE_PRIORITY ,&(numberOfReadyToRunProcesses[DAEMONPROGRAM]))>=0) { //V1-11c
            processTable[PID].state=READY;
            ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,111,SYSPROC,PID,statesNames[0],programList[processTable[PID].programListIndex]->executableName); //V1-10b
            //OperatingSystem_PrintReadyToRunQueue(); V1-9b  Commented in V2-4
        }
    }
    else if (processTable[PID].state==EXECUTING){
        if (Heap_add(PID, readyToRunQueue[DAEMONPROGRAM],QUEUE_PRIORITY ,&(numberOfReadyToRunProcesses[DAEMONPROGRAM]))>=0) {
            processTable[PID].state=READY;
            ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,110,SYSPROC,PID,programList[processTable[PID].programListIndex]->executableName,statesNames[2],statesNames[1]);
            //OperatingSystem_PrintReadyToRunQueue();
        }
    } else if(processTable[PID].state==BLOCKED){
        if (Heap_add(PID, readyToRunQueue[DAEMONPROGRAM],QUEUE_PRIORITY ,&(numberOfReadyToRunProcesses[DAEMONPROGRAM]))>=0) {
            processTable[PID].state=READY;
            ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,110,SYSPROC,PID,programList[processTable[PID].programListIndex]->executableName,statesNames[3],statesNames[1]);
        }
    }
}


// The STS is responsible of deciding which process to execute when specific events occur.
// It uses processes priorities to make the decission. Given that the READY queue is ordered
// depending on processes priority, the STS just selects the process in front of the READY queue
int OperatingSystem_ShortTermScheduler() {
	
	int selectedProcess;

	selectedProcess=OperatingSystem_ExtractFromReadyToRun();
	if(selectedProcess==NOPROCESS){ //Error 27/04/24
        return sipID;
    }

	return selectedProcess;
}


// Return PID of more priority process in the READY queue
int OperatingSystem_ExtractFromReadyToRun() {

	int selectedProcess=NOPROCESS;

	selectedProcess=Heap_poll(readyToRunQueue[USERPROCESSQUEUE],QUEUE_PRIORITY ,&(numberOfReadyToRunProcesses[USERPROCESSQUEUE])); //V1-11c
	
	// Return most priority process or NOPROCESS if empty queue
	return selectedProcess; 
}


// Function that assigns the processor to a process
void OperatingSystem_Dispatch(int PID) {
	if(PID==-1 || PID==NOPROCESS) {
		PID=NOPROCESS;
		return;
	}
	// The process identified by PID becomes the current executing process
	executingProcessID=PID;
	// Change the process' state
	processTable[PID].state=EXECUTING;
	ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,110,SYSPROC,PID,programList[processTable[PID].programListIndex]->executableName,statesNames[1],statesNames[2]); //V1-10b
	// Modify hardware registers with appropriate values for the process identified by PID
	OperatingSystem_RestoreContext(PID);
}


// Modify hardware registers with appropriate values for the process identified by PID
void OperatingSystem_RestoreContext(int PID) {

	// New values for the CPU registers are obtained from the PCB
	Processor_PushInSystemStack(processTable[PID].copyOfPCRegister);
	if(PID!=sipID){
		Processor_SetPC(processTable[PID].copyOfPCRegister-1); //V2-6
	}
	Processor_PushInSystemStack(processTable[PID].copyOfPSWRegister);
	Processor_UpdatePSW(); //NEW
	
	Processor_SetRegisterSP(processTable[PID].copyOfSPRegister);

	Processor_SetAccumulator(processTable[PID].copyOfAccumulator); //V1-13
	Processor_SetRegisterA(processTable[PID].copyOfRegisterA); //V1-13
	Processor_SetRegisterB(processTable[PID].copyOfRegisterB); //V1-13	

	// Same thing for the MMU registers
	MMU_SetBase(processTable[PID].initialPhysicalAddress);
	MMU_SetLimit(processTable[PID].processSize);
}


// Function invoked when the executing process leaves the CPU 
void OperatingSystem_PreemptRunningProcess() {

	// Save in the process' PCB essential values stored in hardware registers and the system stack
	OperatingSystem_SaveContext(executingProcessID);
	// Change the process' state
	OperatingSystem_MoveToTheREADYState(executingProcessID);
	// The processor is not assigned until the OS selects another process
	executingProcessID=NOPROCESS;
}


// Save in the process' PCB essential values stored in hardware registers and the system stack
void OperatingSystem_SaveContext(int PID) {
	if(PID==sipID){ //NEW
		return;
	}
	// Load PSW saved for interrupt manager
	processTable[PID].copyOfPSWRegister=Processor_PopFromSystemStack();
	
	// Load PC saved for interrupt manager
	processTable[PID].copyOfPCRegister=Processor_PopFromSystemStack();
	
	// Save RegisterSP 
	processTable[PID].copyOfSPRegister=Processor_GetRegisterSP();


	processTable[PID].copyOfAccumulator=Processor_GetAccumulator(); //V1-13
	processTable[PID].copyOfRegisterA=Processor_GetRegisterA(); //V1-13
	processTable[PID].copyOfRegisterB=Processor_GetRegisterB(); //V1-13
}


// Exception management routine
void OperatingSystem_HandleException() {

	// Show message "Process [executingProcessID] has generated an exception and is terminating\n"
	//ComputerSystem_DebugMessage(TIMED_MESSAGE,71,INTERRUPT,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName); V4-2
	ComputerSystem_DebugMessage(TIMED_MESSAGE,140,INTERRUPT,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName, exceptionsCauses[Processor_GetRegisterD()]); //V4-2
	
	OperatingSystem_TerminateExecutingProcess();
	OperatingSystem_PrintStatus(); //V2-3d
}

// All tasks regarding the removal of the executing process
void OperatingSystem_TerminateExecutingProcess() {

	processTable[executingProcessID].state=EXIT;
	ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,110,SYSPROC,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName,statesNames[2],statesNames[4]); //V1-10b
	OperatingSystem_ReleaseMainMemory(executingProcessID); //V4-8
	
	if (executingProcessID==sipID && numberOfNotTerminatedUserProcesses==0 && numberOfSleepingProcesses==0 && numberOfProgramsInArrivalTimeQueue==0) { //NEW condition V3-4a numberOfProgramsInArrivalTimeQueue==0
		// finishing sipID, change PC to address of OS HALT instruction
		Processor_SetSSP(MAINMEMORYSIZE-1);
		Processor_PushInSystemStack(OS_address_base+1);
		Processor_PushInSystemStack(Processor_GetPSW());
		executingProcessID=NOPROCESS;
		ComputerSystem_DebugMessage(TIMED_MESSAGE,99,SHUTDOWN,"The system will shut down now...\n");
		ComputerSystem_PowerOff();
		return; // Don't dispatch any process
	}

	Processor_SetSSP(Processor_GetSSP()+2); // unstack PC and PSW stacked

	if (programList[processTable[executingProcessID].programListIndex]->type==USERPROGRAM) {
		// One more user process that has terminated
		numberOfNotTerminatedUserProcesses--;
	}
	
	if (numberOfNotTerminatedUserProcesses==0 && numberOfProgramsInArrivalTimeQueue==0) { //V3-4a numberOfProgramsInArrivalTimeQueue==0
		// Simulation must finish, telling sipID to finish
		OperatingSystem_ReadyToShutdown();
		OperatingSystem_Dispatch(sipID);
		return;

	} else if (numberOfNotTerminatedUserProcesses==numberOfSleepingProcesses){ //NEW
		OperatingSystem_Dispatch(sipID);
		Processor_ActivatePSW_Bit(EXECUTION_MODE_BIT);
		return;
	}

	// Select the next process to execute (sipID if no more user processes)
	int selectedProcess=OperatingSystem_ShortTermScheduler();
	while(selectedProcess==executingProcessID){ //IMPORTANT
		selectedProcess=OperatingSystem_ShortTermScheduler();
	}

	OperatingSystem_Dispatch(selectedProcess);
	if(selectedProcess!=sipID){ //IMPORTANT
		Processor_DeactivatePSW_Bit(EXECUTION_MODE_BIT);
		Processor_DeactivatePSW_Bit(INTERRUPT_MASKED_BIT);
	}
}

// System call management routine
void OperatingSystem_HandleSystemCall() {
  
	int systemCallID;

	// Register A contains the identifier of the issued system call
	systemCallID=Processor_GetRegisterC();
	
	switch (systemCallID) {
		case SYSCALL_PRINTEXECPID:
			// Show message: "Process [executingProcessID] has the processor assigned\n"
			ComputerSystem_DebugMessage(TIMED_MESSAGE,72,SYSPROC,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName,Processor_GetRegisterA(),Processor_GetRegisterB());
			break;

		case SYSCALL_END:
			// Show message: "Process [executingProcessID] has requested to terminate\n"
			ComputerSystem_DebugMessage(TIMED_MESSAGE,73,SYSPROC,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName);
			OperatingSystem_TerminateExecutingProcess();
			OperatingSystem_PrintStatus(); //V2-3c
			break;

		case SYSCALL_YIELD: { //V1-12
			int priority= processTable[executingProcessID].priority;
			int queueID= processTable[executingProcessID].queueID;
			int nextProcessToRunPID=-1;
			int i;
			for (i=0; i<PROCESSTABLEMAXSIZE;i++){
				if (i!=executingProcessID && processTable[i].queueID==queueID && processTable[i].priority==priority && processTable[i].state==READY ){
					nextProcessToRunPID=i;
					break;
				}
			}

			if (nextProcessToRunPID==-1){
				ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,117,SHORTTERMSCHEDULE,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName);
			} else{
				ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,116,SHORTTERMSCHEDULE,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName, nextProcessToRunPID,programList[processTable[nextProcessToRunPID].programListIndex]->executableName);
				OperatingSystem_PreemptRunningProcess();
				OperatingSystem_Dispatch(nextProcessToRunPID);
				OperatingSystem_PrintStatus(); //V2-3b
				Processor_DeactivatePSW_Bit(INTERRUPT_MASKED_BIT); //NEW
				Processor_DeactivatePSW_Bit(EXECUTION_MODE_BIT); //NEW
			}
			break;
		}

		case SYSCALL_SLEEP: {//V2-5f
			int interrupts= Clock_GetNumberOfClockInterrupts();
			int delay;
			if(Processor_GetRegisterD()>0){
				delay=Processor_GetRegisterD();
			} else{
				delay=Processor_GetAccumulator();
				if(delay<0) {
					delay=delay*-1;
				}
			}
			processTable[executingProcessID].whenToWakeUp=delay+interrupts+1;
			//move to blocked state in pos according to whenToWakeUp in the sleepingProcessesQueue
			OperatingSystem_SleepRunningProcess();
			OperatingSystem_Dispatch(OperatingSystem_ShortTermScheduler());
			OperatingSystem_PrintStatus(); //V2-5g
			Processor_DeactivatePSW_Bit(INTERRUPT_MASKED_BIT); //NEW
			if(processTable[executingProcessID].queueID==USERPROCESSQUEUE){ //FIXED ERROR
				Processor_DeactivatePSW_Bit(EXECUTION_MODE_BIT); //NEW
			}
			break;
		}
		
		default: {//V4-4
			ComputerSystem_DebugMessage(TIMED_MESSAGE,141,INTERRUPT,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName, systemCallID); //V4-4a
			OperatingSystem_TerminateExecutingProcess();//V4-4b
			OperatingSystem_PrintStatus();//V4-4c
			break;
		}
	}
}

void OperatingSystem_SleepRunningProcess() { //V2-5f
	// Save in the process' PCB essential values stored in hardware registers and the system stack
	OperatingSystem_SaveContext(executingProcessID);
	// Change the process' state
	OperatingSystem_MoveToTheBLOCKEDState(executingProcessID);
	// The processor is not assigned until the OS selects another process
	executingProcessID=NOPROCESS;
}

void OperatingSystem_MoveToTheBLOCKEDState(int PID) { //V2-5f	
	//Example_ Heap_add(PID, readyToRunQueue[USERPROCESSQUEUE],QUEUE_PRIORITY ,&(numberOfReadyToRunProcesses[USERPROCESSQUEUE]
	if (Heap_add(PID,sleepingProcessesQueue,QUEUE_WAKEUP, &(numberOfSleepingProcesses))>=0) { //Changes que type
		processTable[PID].state=BLOCKED;
		//numberOfSleepingProcesses++;
		ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,110,SYSPROC,PID,programList[processTable[PID].programListIndex]->executableName,statesNames[2],statesNames[3]);
	}	
}
	
//	Implement interrupt logic calling appropriate interrupt handle
void OperatingSystem_InterruptLogic(int entryPoint){
	switch (entryPoint){
		case SYSCALL_BIT: // SYSCALL_BIT=2
			OperatingSystem_HandleSystemCall();
			break;
		case EXCEPTION_BIT: // EXCEPTION_BIT=6
			OperatingSystem_HandleException();
			break;
		case CLOCKINT_BIT:
			OperatingSystem_HandleClockInterrupt();
			break;
	}
}

void OperatingSystem_HandleClockInterrupt(){ //In OperatingSystem.c Exercise 1-b of V2   V2-1b
	Clock_IncreaseNumberOfClockInterrupts(); //ERROR 9/05

	ComputerSystem_DebugMessage(TIMED_MESSAGE,120,INTERRUPT,Clock_GetNumberOfClockInterrupts()); //V2-1c

	//V2-6a
	int interrupts=Clock_GetNumberOfClockInterrupts();
	int awakenProcesses=0;
	int i;
	for (i=0; i<PROCESSTABLEMAXSIZE;i++){
		if(processTable[i].state==BLOCKED && interrupts >= processTable[i].whenToWakeUp) { //V2 EWRROR 29/04
			OperatingSystem_AwakeProcess(i);
			awakenProcesses++;
			OperatingSystem_PrintStatus(); //V2-6b
		}
		
	}
	int created= OperatingSystem_LongTermScheduler(); //V3-3a
	if(created>0){
		OperatingSystem_PrintStatus();
	}

	//V2-6c
	int beforePID=executingProcessID;
	int newPID=beforePID;
	int maxPriority=processTable[beforePID].priority;
	
	for(i=0; i<PROCESSTABLEMAXSIZE && (awakenProcesses>0 || created>0);i++){ //V3-3b   created>0
		if(processTable[i].state==READY && maxPriority > processTable[i].priority){
			maxPriority=processTable[i].priority;
			newPID=i;
		}
	}
	
	if(beforePID!=newPID){
		ComputerSystem_DebugMessage(TIMED_MESSAGE,121,SHORTTERMSCHEDULE,beforePID,programList[processTable[beforePID].programListIndex]->executableName, newPID,programList[processTable[newPID].programListIndex]->executableName);
		OperatingSystem_PreemptRunningProcess();
		OperatingSystem_Dispatch(newPID);
		OperatingSystem_PrintStatus();
		if(newPID!=sipID) { //NEW
	 		//Processor_SetPC(processTable[newPID].copyOfPCRegister); 
			Processor_DeactivatePSW_Bit(EXECUTION_MODE_BIT);
			numberOfReadyToRunProcesses[USERPROCESSQUEUE]--; //ERROR 27/04/24
		}
	}
	Processor_DeactivatePSW_Bit(INTERRUPT_MASKED_BIT); //ERROR V2 29/04
	return; 
}

void OperatingSystem_AwakeProcess(int PID){ //V2-6a
	Heap_poll(sleepingProcessesQueue,QUEUE_WAKEUP ,&(numberOfSleepingProcesses)); //Changes queue type
	OperatingSystem_MoveToTheREADYState(PID);
}


void OperatingSystem_PrintReadyToRunQueue() { //V1-9a
	int i;
	ComputerSystem_DebugMessage(TIMED_MESSAGE,106,SHORTTERMSCHEDULE);
	ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,107,SHORTTERMSCHEDULE); 
	ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,112,SHORTTERMSCHEDULE); 
		//for (i=0; i<numberOfReadyToRunProcesses[NUMBEROFQUEUES];i++) //NUMBEROFQUEUES //ALLPROCESSESQUEUE PROCESSTABLEMAXSIZE
		for (i=0; i<PROCESSTABLEMAXSIZE;i++){
			if(processTable[i].state==READY && processTable[i].queueID==USERPROCESSQUEUE) { //V1-11b
				//ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,108,SHORTTERMSCHEDULE,readyToRunQueue[i]->info,processTable[readyToRunQueue[i]->info].priority);
				ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,108,SHORTTERMSCHEDULE,i,processTable[i].priority);	  
			}
		}
	ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,109,SHORTTERMSCHEDULE);

	ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,107,SHORTTERMSCHEDULE); 
	ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,113,SHORTTERMSCHEDULE); 
		//for (i=0; i<numberOfReadyToRunProcesses[NUMBEROFQUEUES];i++) //NUMBEROFQUEUES //ALLPROCESSESQUEUE PROCESSTABLEMAXSIZE
		for (i=0; i<PROCESSTABLEMAXSIZE;i++){ 
			if(processTable[i].state==READY && processTable[i].queueID==DAEMONSQUEUE) { //V1-11b
				//ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,108,SHORTTERMSCHEDULE,readyToRunQueue[i]->info,processTable[readyToRunQueue[i]->info].priority);
				ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,108,SHORTTERMSCHEDULE,i,processTable[i].priority);	  
			}
		}
	ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,109,SHORTTERMSCHEDULE);
}

