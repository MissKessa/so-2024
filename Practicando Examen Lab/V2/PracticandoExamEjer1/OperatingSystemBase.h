// V2-studentsCode
#ifndef OPERATINGSYSTEMBASE_H
#define OPERATINGSYSTEMBASE_H

#include "ComputerSystem.h"
#include "OperatingSystem.h"
#include "Heap.h"
#include <stdio.h>

// Prototypes of OS functions that students should not change
int OperatingSystem_ObtainAnEntryInTheProcessTable();
int OperatingSystem_ObtainProgramSize(FILE *);
int OperatingSystem_ObtainPriority(FILE *);
int OperatingSystem_LoadProgram(FILE *, int, int);
void OperatingSystem_ReadyToShutdown();
void OperatingSystem_PrepareDaemons(int);
int OperatingSystem_GetExecutingProcessID();
void OperatingSystem_PrintStatus();  // V2-studentsCode
void OperatingSystem_PrintReadyToRunQueue();  // V2-studentsCode

// These "extern" declaration enables other source code files to gain access
// to the variable listed
extern PCB * processTable;
extern int OS_address_base;
extern int sipID;
extern char DAEMONS_PROGRAMS_FILE[];
extern char USER_PROGRAMS_FILE[];

#ifdef SLEEPINGQUEUE
extern heapItem *sleepingProcessesQueue;  // V2-studentsCode
extern int numberOfSleepingProcesses;   // V2-studentsCode
#endif

#endif
