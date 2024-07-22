// V4-studentsCode
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "Simulator.h"
#include "ComputerSystem.h"
#include "OperatingSystemBase.h"
#include "Asserts.h"
#include "MainMemory.h"

// Functions prototypes
int Simulator_GetOption(char *);

extern int initialPID;
extern int endSimulationTime; // For end simulation forced by time
extern char *debugLevel;
extern MEMORYCELL * mainMemory;

// Main memory size (number of memory cells)
extern int MAINMEMORYSIZE;
extern int PROCESSTABLEMAXSIZE;

char *options[] = {
"NONEXISTING_OPTION",
#define OPTION(name,defValue) #name, // #name cast parameter name to String
#include "Options.def"
#undef OPTION
NULL,
};

char *optionsDefault[]={
"NONEXISTING_OPTION_DEFAULT",
#define OPTION(name,defValue) #defValue, // #name cast parameter name to String
#include "Options.def"
#undef OPTION
NULL,
};

int main(int argc, char *argv[]) {
  
	int paramIndex=1; // argv index
	int i, rc, numPrograms=0, isOption=1, thereAreProgramFileOption=0;
	char *option, *optionValue;
	int numericDefaultValue;
	// the options must be always before programs
	for (i=paramIndex; (i < argc) && isOption ;) {
		if ((argv[i][0]=='-') && (argv[i][1]=='-')) {
			option=strtok((char *)&(argv[i][2]),"=");
			optionValue=strtok(NULL," ");
			int optionIndex=Simulator_GetOption(option);
			rc=sscanf(optionsDefault[optionIndex],"%d",&numericDefaultValue);

			switch (optionIndex) {
				// case INITIALPID:
				case initialPID_OPT:
					if (optionValue==NULL || sscanf(optionValue,"%d",&initialPID)==0 || initialPID<0) 
							initialPID=PROCESSTABLEMAXSIZE-1;
					break;
				// case ENDSIMULATIONTIME:
				case endSimulationTime_OPT:
					if (optionValue==NULL || sscanf(optionValue,"%d",&endSimulationTime)==0)
						endSimulationTime=numericDefaultValue;
					break;
				// case NUMASSERTS:
				case numAsserts_OPT:
					if (optionValue==NULL) 
						MAX_ASSERTS=numericDefaultValue;
					else {
						rc=sscanf(optionValue,"%d",&MAX_ASSERTS);
						if (rc<=0 || MAX_ASSERTS<=0)
							MAX_ASSERTS=numericDefaultValue;
					}
					break;
				// case ASSERTSFILE:
				case assertsFile_OPT:
					if (optionValue==NULL){
						optionValue=(char *) malloc((strlen(optionsDefault[assertsFile_OPT])+1)*sizeof(char));
						strcpy(optionValue,optionsDefault[assertsFile_OPT]);
						// LOAD_ASSERTS_CONF=1;
					}
					strcpy(ASSERTS_FILE,optionValue);
					break;
				// case MESSAGESFILE:
				case messagesSTDFile_OPT:
					if (optionValue==NULL){
						optionValue=(char *) malloc((strlen(optionsDefault[messagesSTDFile_OPT])+1)*sizeof(char));
						strcpy(optionValue,optionsDefault[messagesSTDFile_OPT]);
					}
					strcpy(STUDENT_MESSAGES_FILE,optionValue);
					break;
				// case DEBUGSECTIONS:
				case debugSections_OPT:
					if (optionValue==NULL){
						optionValue=(char *) malloc((strlen(optionsDefault[debugSections_OPT])+1)*sizeof(char));
						strcpy(optionValue,optionsDefault[debugSections_OPT]);
					}
					debugLevel=optionValue;
					break;
				// case DAEMONS_PROGRAMS_FILE:
				case daemonsProgramsFile_OPT:
					if (optionValue==NULL){
						optionValue=(char *) malloc((strlen(optionsDefault[daemonsProgramsFile_OPT])+1)*sizeof(char));
						strcpy(optionValue,optionsDefault[daemonsProgramsFile_OPT]);
					}
					strcpy(DAEMONS_PROGRAMS_FILE,optionValue);
					break;
				// case USER_PROGRAMS_FILE:
				case userProgramsFile_OPT:
					thereAreProgramFileOption=1;
					if (optionValue==NULL){
						optionValue=(char *) malloc((strlen(optionsDefault[userProgramsFile_OPT])+1)*sizeof(char));
						strcpy(optionValue,optionsDefault[userProgramsFile_OPT]);
					}
					strcpy(USER_PROGRAMS_FILE,optionValue);
					break;
				// case MAINMEMORYSIZE:
				case memorySize_OPT:
					if (optionValue==NULL) 
						MAINMEMORYSIZE=numericDefaultValue;
					else {
						rc=sscanf(optionValue,"%d",&MAINMEMORYSIZE);
						if (rc<=0 || MAINMEMORYSIZE<=0)
							MAINMEMORYSIZE=numericDefaultValue;
					}
					break;
				// case NUMPROCESSES:
				case numProcesses_OPT:
					if (optionValue==NULL) 
						PROCESSTABLEMAXSIZE=numericDefaultValue;
					else {
						rc=sscanf(optionValue,"%d",&PROCESSTABLEMAXSIZE);
						if (rc<=0 || PROCESSTABLEMAXSIZE<=0)
							PROCESSTABLEMAXSIZE=numericDefaultValue;
					}
					
					break;
				// case GENERATEASSERTS:
				case generateAsserts_OPT:
					GEN_ASSERTS=1; 
					break;
				// case HELP:
				case help_OPT:
					{
						int j;
						printf("Use one or more of these options:\n");
						for (j=1; options[j]!=NULL; j++)
							if (j<generateAsserts_OPT)
								printf("\t%s=ValueOfOption  [%s]\n",options[j], optionsDefault[j]);
							else
								printf("\t%s\n",options[j]);
					}
					break;
				// case INTERVALBETWEENINTERRUPTS:
				case intervalBetweenInterrupts_OPT:  // V2-studentsCode
					if (optionValue==NULL || sscanf(optionValue,"%d",&intervalBetweenInterrupts)<1 || intervalBetweenInterrupts<5)
						intervalBetweenInterrupts=numericDefaultValue;
					break;
				// case MEMCONFIG_FILE:
				case memConfigFile_OPT: // V4-studentsCode
					if (optionValue==NULL){
						optionValue=(char *) malloc((strlen(optionsDefault[memConfigFile_OPT])+1)*sizeof(char));
						strcpy(optionValue,optionsDefault[memConfigFile_OPT]);
					}
					strcpy(MEMCONFIG_FILE,optionValue);
					break;
				default :
					printf("Invalid option: %s\n", option);
					break;
			}
			i++;
		} // End of if is option
		else isOption=0;
	} // End loop processing options
	paramIndex=i; // Next parameter that is not an option parameter

	mainMemory = (MEMORYCELL *) malloc(MAINMEMORYSIZE*sizeof(MEMORYCELL));

	// Now we counting the programs in command line		
	for (i=paramIndex;i<argc;){
		rc=strncmp(argv[i],"--",2);
		i++;
		if (rc!=0) {
			numPrograms++;
			if ((i < argc)
			 && (sscanf(argv[i], "%d", &rc) == 1))
				// An arrival time has been read. Increment i
				i++;
		}
		else {
			numPrograms=-1; // Options beetwen programs are not allowed
			break;
		}
	}

	if (!thereAreProgramFileOption) // no userProgramsFile option used...
		// We now have a multiprogrammed computer system
		// No more than PROGRAMSMAXNUMBER in the command line
		if ((numPrograms<=0) || (numPrograms>PROGRAMSMAXNUMBER)) {
			printf("USE: Simulator [--optionX=optionXValue ...] <program1> [arrivalTime] [<program2> [arrivalTime] .... <program%d [arrivalTime]] \n",PROGRAMSMAXNUMBER);
			if (numPrograms<0)
				printf("Options must be before program names !!!\n");
			else 
				printf("Must have beetwen 1 and %d program names, or use userProgramsFile option !!!\n", PROGRAMSMAXNUMBER);
			exit(-1);
		}



	// The simulation starts
	ComputerSystem_PowerOn(argc, argv, paramIndex);
	// The simulation ends
	ComputerSystem_PowerOff();
	return 0;
}

// to get the index number of the option
int Simulator_GetOption(char *option){
	int i;
	for (i=0; options[i]!=NULL ; i++)
		if (strcasecmp(options[i],option)==0)
			return i;
	return -1;
}
