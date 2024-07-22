// V1
#ifndef PROCESSORBASE_H
#define PROCESSORBASE_H

#include "Buses.h"

enum Instruction {
NONEXISTING_INST,
#define INST(name) name ## _INST, // name ## _INST concatena y define los enumerados ADD_INST, y SHIFT_INST
#include "Instructions.def"
#undef INST
LAST_INST,
};

// Enumerated type that assigns numbers to processor registers
enum CPU_REGISTERS_NUMBERED {REGISTERACCUMULATOR_CPU=0, REGISTERA_CPU=1, REGISTERB_CPU=2};

void Processor_UpdatePSW();
void Processor_CheckOverflow(int,int,int);

void Processor_PushInSystemStack(int );
int Processor_PopFromSystemStack();
int Processor_CopyFromRawMemory(int);
void Processor_CopyInRawMemory(int, int);
void Processor_RaiseInterrupt(const unsigned int);
void Processor_ACKInterrupt(const unsigned int);
unsigned int Processor_GetInterruptLineStatus(const unsigned int);


void Processor_ActivatePSW_Bit(const unsigned int);
void Processor_DeactivatePSW_Bit(const unsigned int);
unsigned int Processor_PSW_BitState(const unsigned int);

// The OS needs to access MAR and MBR registers to save the context of
// the process to which the processor is being assigned
// Buses needs to access MAR and MBR
int Processor_GetMAR();
void Processor_SetMAR(int);
void Processor_GetMBR(BUSDATACELL *);
void Processor_SetMBR(BUSDATACELL *);

// The OS needs to access the accumulator register to restore the context of
// the process to which the processor is being assigned and to save the context
// of the process being preempted for another ready process
void Processor_SetAccumulator(int);
int Processor_GetAccumulator();

// The OS needs to access the PC register to restore the context of
// the process to which the processor is being assigned
void Processor_SetPC(int);

// The OS needs to access register C to when executing the system call management
// routine, so it will be able to know the invoked system call identifier
int Processor_GetRegisterC();

// The OS needs to access register A to when executing the PRINTPID system call
int Processor_GetRegisterA();

// The OS needs to access register B to when executing the PRINTPID system call
int Processor_GetRegisterB();

// The OS needs to access Stack Point register
int Processor_GetRegisterSP();

// The OS needs to access Stack Point register
void Processor_SetRegisterSP(int);

// The OS needs to access registerA
void Processor_SetRegisterA(int);

// The OS needs to access registerB
void Processor_SetRegisterB(int);

// The OS needs to access the PSW register to restore the context of
// the process to which the processor is being assigned
void Processor_SetPSW(unsigned int);
unsigned int Processor_GetPSW();

int Processor_GetSSP();
void Processor_SetSSP(unsigned int);

int Processor_Encode(int , int , int);
int Processor_DecodeOperationCode(BUSDATACELL);
int Processor_DecodeOperand1(BUSDATACELL);
int Processor_DecodeOperand2(BUSDATACELL);
void Processor_GetCodedInstruction(char * , BUSDATACELL );
int Processor_ToInstruction(char *); 

#endif
