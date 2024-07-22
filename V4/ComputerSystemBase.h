// V3-studentsCode
#ifndef COMPUTERSYSTEMBASE_H
#define COMPUTERSYSTEMBASE_H

#include "Heap.h"	// V3-studentsCode

enum TimedMessages{ NO_TIMED_MESSAGE,TIMED_MESSAGE };

// Functions prototypes
int ComputerSystem_ObtainProgramList(int , char *[], int);
void ComputerSystem_DebugMessage(int , int, char , ...);
int ComputerSystem_PrepareAditionalPrograms(int , int , char *);
void ComputerSystem_FillInArrivalTimeQueue(); // V3-studentsCode
void ComputerSystem_PrintArrivalTimeQueue(); // V3-studentsCode

// Sections in which we divide our simulator in terms of
// debugging messages that show its internal working details
// See "DebugSections.def"

enum DebugSection {
NONEXISTING_DEBUG_SECTION=0,
#undef DEBUGSECTION
#define DEBUGSECTION(name,value) name = value , 
#include "DebugSections.def"
#undef DEBUGSECTION
LAST_DEBUG_SECTION=255
};

// Basic data to collect about every program to be created
// User programs specified in the command line: name of the file, the time of its arrival time
// 												to the system (0, by default), and type USERPROGRAM
// Daemon programs of type DAEMONPROGRAM
typedef struct ProgramData {
    char *executableName;
    unsigned int arrivalTime;
    unsigned int type;
} PROGRAMS_DATA;

// This "extern" declarations enables other source code files to gain access to the variables 
extern char defaultDebugLevel[];
extern int intervalBetweenInterrupts; // V2-studentsCode

#define DEFAULT_INTERVAL_BETWEEN_INTERRUPTS 5 // V2-studentsCode

#ifdef ARRIVALQUEUE
extern int numberOfProgramsInArrivalTimeQueue;	// V3-studentsCode
extern heapItem *arrivalTimeQueue;			 	// V3-studentsCode
#endif

#endif
