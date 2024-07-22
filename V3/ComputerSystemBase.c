// V3-studentsCode
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include "ComputerSystem.h"
#include "ComputerSystemBase.h"
#include "Processor.h"
#include "ProcessorBase.h"
#include "Heap.h"
#include "OperatingSystemBase.h"
#include "Messages.h"
#include "Asserts.h"
#include "Clock.h"

// Functions prototypes


extern int GEN_ASSERTS;

char defaultDebugLevel[]="A";
// String specified in the command line to tell the simulator which of its
// sections are interesting for the user so it must show debug messages
// related to them
char *debugLevel=defaultDebugLevel;

int endSimulationTime=-1; // For end simulation forced by time

int intervalBetweenInterrupts = DEFAULT_INTERVAL_BETWEEN_INTERRUPTS; // Default value V2-studentsCode

// Only one colour messages. Set to 1 for more colours checking uppercase in debugLevel
int COLOURED = 0 ;

// Array that contains basic data about all daemons
// and all user programs specified in the command line
PROGRAMS_DATA *programList[PROGRAMSMAXNUMBER];

// Students messages 
char STUDENT_MESSAGES_FILE[MAXFILENAMELENGTH]="messagesSTD.txt";

char USER_PROGRAMS_FILE[MAXFILENAMELENGTH]="";
#ifdef ARRIVALQUEUE
	char * typeProgramNames []={"USER-PROGRAM","DAEMON-PROGRAM"}; 
	extern heapItem * arrivalTimeQueue;
	extern int numberOfProgramsInArrivalTimeQueue;
#endif

// Fill in the array named userProgramsList with the information given
// by the user in the command line
// IT IS NOT NECESSARY TO COMPLETELY UNDERSTAND THIS FUNCTION
int ComputerSystem_ObtainProgramList(int argc, char *argv[], int paramIndex) {
	
	int i;
	int count=1;  // 0 reserved for sipid
	PROGRAMS_DATA *progData;

	// Initialization of the programList
	for (i=0; i<PROGRAMSMAXNUMBER;i++) {
	      programList[i]=NULL;
	}

	// Store the names of the programs
	for (i = paramIndex; i < argc && count<PROGRAMSMAXNUMBER;) { // check number of programs < PROGRAMSMAXNUMBER
		progData=(PROGRAMS_DATA *) malloc(sizeof(PROGRAMS_DATA));
		// Save file name
		progData->executableName = (char *) malloc((strlen(argv[i])+1)*sizeof(char));
		strcpy(progData->executableName,argv[i]);
		i++;
		// Default arrival time: 0  
		progData->arrivalTime = 0;
		// Try to store the arrival time if exists
		if ((i < argc)
			 && (sscanf(argv[i], "%d", &(progData->arrivalTime)) == 1)) {
				// An arrival time has been read. Increment i
				i++;
			 }
		// Defaul user programs
		progData->type=USERPROGRAM;

		// Store the structure in the list
		programList[count]=progData;
		
 		count++; // There is one more program
	}

	// Preparing aditionals user programs here
	// index for aditionals programs in programList
	count=ComputerSystem_PrepareAditionalPrograms(count,USERPROGRAM,USER_PROGRAMS_FILE);


	return count; // Next place for new programs
}

// Function used to show messages with details of the internal working of
// the simulator
// IT IS NOT NECESSARY TO UNDERSTAND ALL THE DETAILS OF THIS FUNCTION
void ComputerSystem_DebugMessage(int timed, int msgNo, char section, ...) {

	va_list lp;
	char c, *format;
	int count, youHaveToContinue, colour=0;
 
	int pos;
	
        pos=Messages_Get_Pos(msgNo);
        if (pos==-1) {
         printf("Debug Message %d not defined\n",msgNo);
         return;
        }
        format=DebugMessages[pos].format;
        	
	char notTabuled[]="hodxct";
	
	if ((strchr(debugLevel, ALL) != NULL) 		// Print the message because the user specified ALL
	  || section == ERROR  						//  Always print ERROR section
	  || (strchr(debugLevel,section)) != NULL){ //  or the section argument is included in the debugLevel string
		if (timed) {
			if ((strchr(notTabuled,section)) != NULL) // print tab if necessary
				// ComputerSystem_ShowTime(section);
				ComputerSystem_DebugMessage(NO_TIMED_MESSAGE, Processor_PSW_BitState(EXECUTION_MODE_BIT)?95:94,section,Clock_GetTime());
			else{
				// OperatingSystem_ShowTime(section);  
				ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,100,section,Processor_PSW_BitState(EXECUTION_MODE_BIT)?"\t":"");
				ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,Processor_PSW_BitState(EXECUTION_MODE_BIT)?95:94,section,Clock_GetTime());
			}
		}
		va_start(lp, section);

		for (count = 0, youHaveToContinue = 1; youHaveToContinue == 1; count++) {
			//printf("(%c)",format[count]);
			switch (format[count]) {
				case '\\':count++;
				 	 switch (format[count]) {
						case 'n': printf("\n");
							  break;
						case 't': printf("\t");	
							  break;
						default: printf("\%c", format[count]);
					} // switch Control Chars
					break;
				// case '%':			
				case '@':	// Next color char		
		 			count++;
					switch (format[count]) {
			 			case 'R': // Text in red
							if (COLOURED){
							  printf("%c[%d;%dm", 0x1B, 1, 31);
							  if (!colour) colour=1;
							}
						break;
						case 'G': // Text in green
							if (COLOURED){
						  	printf("%c[%d;%dm", 0x1B, 1, 32);
						  	if (!colour) colour=1;
							}
							break;
						case 'Y': // Text in yellow
		  					if (COLOURED){
							  printf("%c[%d;%dm", 0x1B, 1, 33);
							  if (!colour) colour=1;
							}
							break;
						case 'B': // Text in blue
							if (COLOURED){
							  printf("%c[%d;%dm", 0x1B, 1, 34);
						  	if (!colour) colour=1;
							}
							break;
						case 'M': // // Text in magenta
							if (COLOURED){
						  	printf("%c[%d;%dm", 0x1B, 1, 35);
						  	if (!colour) colour=1;
							}
							break;
						case 'C': // Text in cyan
							if (COLOURED){
							  printf("%c[%d;%dm", 0x1B, 1, 36);
							  if (!colour) colour=1;
							}
							break;		
						case 'W': // Text in white
							if (COLOURED){
							  printf("%c[%d;%dm", 0x1B, 1, 37);
							  if (!colour) colour=1;
							}
							break;
						case '@': // Text without color
							if (COLOURED && colour)
							    printf("%c[%dm", 0x1B, 0);
							break;	
					}	// switch colors chars
					break;
				case '%':			
		 			count++;
					switch (format[count]) {
						case 's':
							printf("%s",va_arg(lp, char *));
							break;
						case 'd':
							printf("%d",va_arg(lp, int));
							break;
						case 'f':
							printf("%f",va_arg(lp, double));
							break;
						case 'c':
							c = (char) va_arg(lp, int);
							printf("%c", c);
							break;
						case 'x':
							printf("%04X", va_arg(lp, int));
							break;
						default:
							youHaveToContinue = 0;
						} // switch format chars
					break;
				default:if (format[count]==0)
						youHaveToContinue=0;
					else
						printf("%c",format[count]);				
			} // switch
		} // for
		va_end(lp);
	} // if
	if (COLOURED && colour)
	    printf("%c[%dm", 0x1B, 0);
} // ComputerSystem_DebugMessage

int ComputerSystem_PrepareAditionalPrograms(int programListNextPosition, int type, char * programsFileName){
	FILE *programsFile;
	char lineRead[MAXLINELENGTH];
	PROGRAMS_DATA *progData;
	char *name, *arrivalTime;
	int time;
	int numberOfPrograms=0;

	programsFile= fopen(programsFileName, "r");

	// Check if programFile exists
	if (programsFile==NULL)
		return programListNextPosition;

	while (fgets(lineRead, MAXLINELENGTH, programsFile) != NULL
					 && programListNextPosition<PROGRAMSMAXNUMBER) {
		name=strtok(lineRead,",\n\r\t ");
	    if (name==NULL){
			continue;
		}

		arrivalTime=strtok(NULL,", \n\r");
    	if (arrivalTime==NULL
    		|| sscanf(arrivalTime,"%d",&time)==0)
    		time=0;
    	
    	progData=(PROGRAMS_DATA *) malloc(sizeof(PROGRAMS_DATA));
    	progData->executableName = (char *) malloc((strlen(name)+1)*sizeof(char));
    	strcpy(progData->executableName,name);
    	progData->arrivalTime=time;
    	progData->type=type;
    	programList[programListNextPosition++]=progData;
		numberOfPrograms++;
	}

	ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,77,POWERON,numberOfPrograms,type==DAEMONPROGRAM ? "daemon":"user", programsFileName);

	return programListNextPosition;
}

// Fill ArrivalTimeQueue heap with user program from parameters or file and daemons 
void ComputerSystem_FillInArrivalTimeQueue() { // V3-studentsCode
#ifdef ARRIVALQUEUE

  int arrivalIndex = 0; 

	while (programList[arrivalIndex]!=NULL && arrivalIndex<PROGRAMSMAXNUMBER) {
	  Heap_add(arrivalIndex,arrivalTimeQueue,QUEUE_ARRIVAL,&arrivalIndex);
	}
	numberOfProgramsInArrivalTimeQueue=arrivalIndex;
#endif
}

// Print arrivalTiemQueue program information
void ComputerSystem_PrintArrivalTimeQueue(){ // V3-studentsCode
#ifdef ARRIVALQUEUE
  int i;
  
  // Show message "Arrival Time Queue: "
  ComputerSystem_DebugMessage(TIMED_MESSAGE,100,LONGTERMSCHEDULE,"Arrival Time Queue:\n");
  if (numberOfProgramsInArrivalTimeQueue>0) {
	for (i=0; i< numberOfProgramsInArrivalTimeQueue; i++) {
	  // Show message [executableName,arrivalTime]
	  ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,70,LONGTERMSCHEDULE,programList[arrivalTimeQueue[i].info]->executableName,
			programList[arrivalTimeQueue[i].info]->arrivalTime,
			typeProgramNames[programList[arrivalTimeQueue[i].info]->type-100]);
	}
  }
  else {
  	ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,100,SHORTTERMSCHEDULE,"\t\t[--- empty queue ---]");
	ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,100,SHORTTERMSCHEDULE,"\n");
  }

 #endif
}

