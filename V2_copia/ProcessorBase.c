// V1
// Don't change this file !!!
#include <stdio.h>
#include <string.h>
#include "Processor.h"
#include "ProcessorBase.h"
#include "Buses.h"
#include "Clock.h"
#include "Asserts.h"

extern int registerPC_CPU; // Program counter
extern int registerAccumulator_CPU; // Accumulator
extern BUSDATACELL registerIR_CPU; // Instruction register
extern unsigned int registerPSW_CPU; // Processor state word, initially protected mode
extern int registerMAR_CPU; // Memory Address Register
extern BUSDATACELL registerMBR_CPU; // Memory Buffer Register
extern int registerCTRL_CPU; // Control bus Register
extern int registerA_CPU; // General purpose register
extern int registerB_CPU; // General purpose register
extern int registerC_CPU; // System register
extern int registerSP_CPU; // Stack pointer register
extern int interruptLines_CPU; // Processor interrupt lines
extern int interruptVectorTable[];
extern char pswmask []; 

char *InstructionNames[] = {
"NONEXISTING_INSTRUCTION",
#define INST(name) #name, // #name cast parameter name to String
#include "Instructions.def"
#undef INST
};

int registerSSP_CPU; // System stack pointer

// This is the instruction cycle loop (fetch, decoding, execution, etc.).
// The processor stops working when an POWEROFF signal is stored in its
// PSW register
void Processor_InstructionCycleLoop() {

	while (!Processor_PSW_BitState(POWEROFF_BIT)) {
		if (Processor_FetchInstruction()==CPU_SUCCESS){
			Processor_DecodeAndExecuteInstruction();
		}
		if (interruptLines_CPU){
			Processor_ManageInterrupts();
		}
	}
}

// Update PSW state
void Processor_UpdatePSW(){
	// Update ZERO_BIT
	if (registerAccumulator_CPU==0){
		if (!Processor_PSW_BitState(ZERO_BIT))
			Processor_ActivatePSW_Bit(ZERO_BIT);
	}
	else {
		if (Processor_PSW_BitState(ZERO_BIT))
			Processor_DeactivatePSW_Bit(ZERO_BIT);
	}
	
	// Update NEGATIVE_BIT
	if (registerAccumulator_CPU<0) {
		if (!Processor_PSW_BitState(NEGATIVE_BIT))
			Processor_ActivatePSW_Bit(NEGATIVE_BIT);
	}
	else {
		if (Processor_PSW_BitState(NEGATIVE_BIT))
			Processor_DeactivatePSW_Bit(NEGATIVE_BIT);
	}
}

// Check overflow, receive operands for add and register of the result
void Processor_CheckOverflow(int op1, int op2, int reg) {
	int registerUsed;
	switch (reg)
	{
	case REGISTERACCUMULATOR_CPU:
		registerUsed=registerAccumulator_CPU;
		break;
	case REGISTERA_CPU:
		registerUsed=registerA_CPU;
		break;
	case REGISTERB_CPU:
		registerUsed=registerB_CPU;
		break;
	default:
		registerUsed=registerAccumulator_CPU;
		break;
	}
	if ((op1>0 && op2>0 && registerUsed<0)
		|| (op1<0 && op2<0 && registerUsed>0))
		Processor_ActivatePSW_Bit(OVERFLOW_BIT);
}

// Save in the system stack a given value
void Processor_CopyInRawMemory(int physicalMemoryAddress, int data) {

	registerMBR_CPU.cell=data;
	registerMAR_CPU=physicalMemoryAddress;
	Buses_write_AddressBus_From_To(CPU, MAINMEMORY);
	Buses_write_DataBus_From_To(CPU, MAINMEMORY);	
	registerCTRL_CPU=CTRLWRITE;
	Buses_write_ControlBus_From_To(CPU,MAINMEMORY);
}

// Get value from system stack
int Processor_CopyFromRawMemory(int physicalMemoryAddress) {

	registerMAR_CPU=physicalMemoryAddress;
	Buses_write_AddressBus_From_To(CPU, MAINMEMORY);
	registerCTRL_CPU=CTRLREAD;
	Buses_write_ControlBus_From_To(CPU,MAINMEMORY);
	return registerMBR_CPU.cell;
}

// Save in the system stack a given value
void Processor_PushInSystemStack(int data) {

	registerMBR_CPU.cell=data;
	registerMAR_CPU=registerSSP_CPU;
	registerSSP_CPU--;
	Buses_write_AddressBus_From_To(CPU, MAINMEMORY);
	Buses_write_DataBus_From_To(CPU, MAINMEMORY);	
	registerCTRL_CPU=CTRLWRITE;
	Buses_write_ControlBus_From_To(CPU,MAINMEMORY);
}

// Get value from system stack
int Processor_PopFromSystemStack() {

	registerSSP_CPU++;
	registerMAR_CPU=registerSSP_CPU;
	Buses_write_AddressBus_From_To(CPU, MAINMEMORY);
	registerCTRL_CPU=CTRLREAD;
	Buses_write_ControlBus_From_To(CPU,MAINMEMORY);
	return registerMBR_CPU.cell;
}

// Put the specified interrupt line to a high level 
void Processor_RaiseInterrupt(const unsigned int interruptNumber) {
	unsigned int mask = 1;

	mask = mask << interruptNumber;
	interruptLines_CPU = interruptLines_CPU | mask;
}

// Put the specified interrupt line to a low level 
void Processor_ACKInterrupt(const unsigned int interruptNumber) {
	unsigned int mask = 1;

	mask = mask << interruptNumber;
	mask = ~mask;

	interruptLines_CPU = interruptLines_CPU & mask;
}

// Returns the state of a given interrupt line (1=high level, 0=low level)
unsigned int Processor_GetInterruptLineStatus(const unsigned int interruptNumber) {
	unsigned int mask = 1;

	mask = mask << interruptNumber;
	return (interruptLines_CPU & mask) >> interruptNumber;
}

// Set a given bit position in the PSW register
void Processor_ActivatePSW_Bit(const unsigned int nbit) {
	unsigned int mask = 1;

	mask = mask << nbit;
	
	registerPSW_CPU = registerPSW_CPU | mask;
}

// Unset a given bit position in the PSW register
void Processor_DeactivatePSW_Bit(const unsigned int nbit) {
	unsigned int mask = 1;

	mask = mask << nbit;
	mask = ~mask;

	registerPSW_CPU = registerPSW_CPU & mask;
}

// Returns the state of a given bit position in the PSW register
unsigned int Processor_PSW_BitState(const unsigned int nbit) {
	unsigned int mask = 1;

	mask = mask << nbit;
	return (registerPSW_CPU & mask) >> nbit;
}

// Getter for the registerMAR_CPU
int Processor_GetMAR() {
  return registerMAR_CPU;
}

// Setter for the registerMAR_CPU
void Processor_SetMAR(int data) {
  registerMAR_CPU=data;
}

// pseudo-getter for the registerMBR_CPU
void Processor_GetMBR(BUSDATACELL *toRegister) {
  memcpy((void*) toRegister, (void *) (&registerMBR_CPU), sizeof(BUSDATACELL));
}

// pseudo-setter for the registerMBR_CPU
void Processor_SetMBR(BUSDATACELL *fromRegister) {
  memcpy((void*) (&registerMBR_CPU), (void *) fromRegister, sizeof(BUSDATACELL));
}

// Getter for the registerCTRL_CPU
int Processor_GetCTRL() {
	// Only return last byte
	return registerCTRL_CPU & 0xf;
}

// Setter for the registerCTRL_CPU
void Processor_SetCTRL(int ctrl) {
  	registerCTRL_CPU = (ctrl & 0xff);
}

// Getter for the Accumulator
int Processor_GetAccumulator() {
  return registerAccumulator_CPU;
}

// Setter for the Accumulator
void Processor_SetAccumulator(int acc){
  registerAccumulator_CPU=acc;
}

// Setter for the PC
void Processor_SetPC(int pc){
  registerPC_CPU= pc;
}

// Getter for the RegisterA
int Processor_GetRegisterA() {
  return registerA_CPU;
}

// Setter for the RegisterA
void Processor_SetRegisterA(int reg){
  registerA_CPU= reg;
}

// Getter for the RegisterB
int Processor_GetRegisterB() {
  return registerB_CPU;
}

// Setter for the RegisterB
void Processor_SetRegisterB(int reg){
  registerB_CPU= reg;
}

// Getter for the RegisterC 
int Processor_GetRegisterC() {
  return registerC_CPU;
}

// Setter for the RegisterC
void Processor_SetRegisterC(int reg){
  registerC_CPU= reg;
}

// Getter for the Stack Point Register
int Processor_GetRegisterSP() {
  return registerSP_CPU;
}

// Setter for the RegisterSP
void Processor_SetRegisterSP(int reg){
  registerSP_CPU= reg;
}

// Getter for the PSW
unsigned int Processor_GetPSW(){
	return registerPSW_CPU;
}

// Setter for the PSW
void Processor_SetPSW(unsigned int psw){
	registerPSW_CPU=psw;
}

// Getter for the SSP
int Processor_GetSSP(){
	return registerSSP_CPU;
}

// Setter for the System Stack Pointer
void Processor_SetSSP(unsigned int top){
	registerSSP_CPU=top;
}

int Processor_Encode(int opCode, int op1, int op2) {
	int mask=0x7ff; // binary: 0111 1111 1111 
	int sigOp1=op1<0;
	op1=sigOp1 ? ((-op1) & mask) : (op1 & mask);
	int sigOp2=op2<0;
	op2=sigOp2 ? ((-op2) & mask) : (op2 & mask);
	int cell=(opCode<<24);
	cell = cell | (sigOp1<<23) | (op1<<12);
	cell = cell | (sigOp2<<11) | op2;
	return cell;
}

int Processor_DecodeOperationCode(BUSDATACELL memCell) {
	int opCode=(memCell.cell>>24) & 0xff;
	return (opCode<LAST_INST ? opCode : NONEXISTING_INST);
}

int Processor_DecodeOperand1(BUSDATACELL memCell) {
	int sigOp1=memCell.cell & (0x1<<23);
	int op1=(memCell.cell & (0x7ff<<12))>>12;
	op1=sigOp1?-op1:op1;
	return op1;
}

int Processor_DecodeOperand2(BUSDATACELL memCell) {
	int sigOp2= memCell.cell & (0x1<<11);
	int op2=(memCell.cell & (0x7ff));
	op2=sigOp2?-op2:op2;
	return op2;
}

void Processor_GetCodedInstruction(char * result, BUSDATACELL memCell){
	sprintf(result,"%02X %03X %03X",((memCell.cell>>24)&0xff),((memCell.cell>>12)&0xfff),(memCell.cell&0xfff));	
}

int Processor_ToInstruction(char * operation) {
	int i;
	for (i=0;i<LAST_INST;i++)
		if (strcasecmp(InstructionNames[i],operation)==0)
			return i;
	return NONEXISTING_INST;
}
