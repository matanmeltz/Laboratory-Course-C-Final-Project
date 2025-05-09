
/********************************************************************* - FILES DISCRIPTION - *********************************************************************/

/*
* This file is connecting between all the files in the project, and it is including all the functions,
* structures, macros definitions and libraries that are used in this project.
*
* The project files are:
*			1. assembler.c
*			2. pre_processor.c
*			3. first_pass.c
*			4. second_pass.c
*			5. utilities.c
*			6. errors.c
*			7. headers.h
*			8. makefile
*/

/****************************************************** - THE STANDARD C LIBRARIES INCLUDED IN THE PROGRAM - ******************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

/********************************************************************** - MACRO DEFINITIONS - *********************************************************************/

/* Sizes: */
#define MAX_LENGTH_OF_LINE 82
#define MAX_LENGTH_OF_LABEL 32 
#define BUFFER 5 /* Initial size of macro commends array */
#define ERROR_MESSAGE_SIZE 110
#define MEMORY_CELLS 4095

/* File suffixes: */
#define INPUT_FILE_SIZE 4
#define OUTPUT_FILE_SIZE 4
#define OB_FILE_SIZE 4
#define EXT_FILE_SIZE 5
#define ENT_FILE_SIZE 5

/* Bitwise values: */
#define BITS_NUM 15 /* Represent the number of bits in the assembler */
#define ONE_BIT 1
#define IMMEDIATE_ADDRESSING 0
#define DIRECT_ADDRESSING 1
#define INDIRECT_REGISTER_ADDRESSING 2
#define DIRECT_REGISTER_ADDRESSING 3
#define OPCODE_START_POSITION 11
#define SOURCE_START_POSITION 7
#define TARGET_START_POSITION 3
#define OPERAND_START_POSITION 3
#define ADDRESS_START_POSITION 3
#define SOURCE_REGISTER_START_POSITION 6
#define TARGET_REGISTER_START_POSITION 3
#define A_ADDRESSING_METHOD 2
#define R_ADDRESSING_METHOD 1
#define E_ADDRESSING_METHOD 0

/* Values: */
#define FATAL_ERROR -1 /* Represents memory and file opening errors */
#define TRUE 1
#define FALSE 0 
#define EXTERN_VALUE 01
#define HAVE_BMC 0
#define ENTRY_VALUES 100
#define FIRST_MEMORY_CELL 100

/* No keyword: */
#define NO_TYPE -1
#define NO_OPCODE -1
#define NO_REGISTER -1
#define NO_DECIMAL_ADDRESS -1

/************************************************************************* - STRUCTURES - *************************************************************************/

/* pre_processor struct: */
typedef struct macro{
	char *name; /* Stores the name of the macro. */
	char **commands; /* Stores the commands of the macro. */
	struct macro *next; /* A pointer to the next node in the linked list. */
}macroNode;

/* first and second passes structures: */
typedef struct instruction{
	short ic; /* At the beginning of the first passes represents the instruction counter - IC, and at the end represents the decimal address. */
	unsigned int line; /* If line > 0, represents the number of the line where the label is mentioned.
			            * If line = 0, represents that the word has already been translated into binary machine code. */
	union{
		short bmc; /* Represent the Binary Machine Code-BMC. */
		char *name; /* The name of the label mentioned in the line. */
	}type;
	struct instruction *next; /* A pointer to the next node in the linked list. */
}instructionNode;

typedef struct data{
	short dc; /* At the beginning of the first passes represents the data counter - DC, and at the end represents the decimal address. */
	short bmc; /* Represent the Binary Machine Code-BMC. */
	struct data *next; /* A pointer to the next node in the linked list. */
}dataNode;

typedef struct declaration{
	short decimal_address; /* If it's an instruction, it represents the IC\DC counter. 
							* If it's an entry or extern, it represents the line number where it was declared. */
	char *label; /* The name of the declared label. */
	char type; /* .data represented by 0, .string by 1, .entry by 2, .extern by 3 and .code by 4. */
	struct declaration *next; /* A pointer to the next node in the linked list. */
}declarationNode;

/* Reserved keywords of the assembler: */
typedef enum {MOV, CMP, ADD, SUB, LEA, CLR, NOT, INC, DEC, JMP, BNE, RED, PRN, JSR, RTS, STOP} opcode;

typedef enum {DATA, STRING, ENTRY, EXTERN, CODE} instruction;

/****************************************************************** - PRE_PROCESSOR FUNCTIONS - ******************************************************************/

char macrosLayout(FILE*, char*, char*, char*, macroNode**);
macroNode* readNewMacro(char*, unsigned int*, FILE*, macroNode*, macroNode**, char*);
macroNode* creatMacroNode(char*);
void freeMacrosList(macroNode*);
char getMacroData(char*, unsigned int*, FILE*, macroNode*, macroNode*, char*);
char getMacroName(char*, char*, char*);
char getCommands(char*, FILE*, unsigned int*, macroNode*, char*);
char isStartMacroDeclaration(char*, char*);
char isEndMacroDeclaration(char*, char*);
char isValidLine(char*, char*);
/* Printing functions: */
macroNode* isMacroNameExists(char*, macroNode*);
void printLineToFile(FILE*, char*, macroNode*);
void printMacroCommands(macroNode*, FILE*);

/******************************************************************** - FIRST_PASS FUNCTIONS - ********************************************************************/

char firstPass(FILE*, char*, short* ,macroNode*, instructionNode**, dataNode**, declarationNode**);
char UpdateDataCounter(dataNode*, declarationNode*, short*, short, short);
char checkBitException(int, short);

/* Declaration nodes functions: */
declarationNode* readNewDeclaration(char*, unsigned int, macroNode*, declarationNode*, declarationNode**, char*);
declarationNode* readNewLabel(char*, unsigned int, macroNode*, short, short, declarationNode*, declarationNode**, char*);
declarationNode* creatDeclarationNode(short, char*, char, declarationNode*, char*);
char getLabelName(char*, char*, char*, macroNode*);
char getInstructionType(char*, char*);
char checkLabelName(char*, macroNode*, char*);
char checkIfLabelNameAlreadyDeclared(char*, short, declarationNode*, char*);
void freeDeclarationsList(declarationNode*);

/* Data nodes functions: */
dataNode* readNewData(short*, short, char*, dataNode*, dataNode**, char*);
dataNode* creatDataNode(short, short, dataNode**, char*);
short readNumbers(char*, short*, char*);
char dataLineCheck(char*, char*);
short readString(char*, short*, char*);
char stringLineCheck(char*, char*);
void freeDataList(dataNode*);

/* Instruction nodes functions: */
instructionNode* readNewInstruction(short*, char*, instructionNode*, instructionNode**, unsigned int, char*);
instructionNode* creatInstructionNode(short, short, char*, unsigned int, instructionNode**, char*);
char isAbsolute(short);
void getFirstWordBMC(short, short, short, short*);
void getImmediateBMC(char*, short*);
void getSourceRegisterBMC(char*, short*);
void getTargetRegisterBMC(char*, short*);
short isValidTargetOperandType(short, char*, char*);
short isValidSourceOperandType(short, char*, char*);
short getOperandAddressingType(char*, char*);
char readTargetOperand(char*, char*, char*);
char readTwoOperands(char*, char*, char*, char*);
short getOpcode(char*);
void freeInstructionsList(instructionNode*);

/******************************************************************* - SECOND_PASS FUNCTIONS - *******************************************************************/

char secondPass(char*, char*, short*, instructionNode**, dataNode*, declarationNode*, char);
char isEntriesDefined(char*, declarationNode*);
char fillingMissingBMCofLabel(char*, instructionNode**, declarationNode*, short*, short*);
char createObAndExtFiles(char*, short*, instructionNode**, dataNode*, short*);
char createEntFile(char*, declarationNode*, short*);
void printfObIcDcNumbersLine(FILE*, short*);
void printfEntOutputLine(FILE*, declarationNode*, short*);
void printfExtOutputLine(FILE*, instructionNode*, short*);
void getRelocatableBMC(short, short*);

/********************************************************************* - UTILITIES FUNCTIONS - *********************************************************************/

short isOpcode(char*);
short isRegister(char*);
short isInstruction(char*);
char isValidName(char*, char*);
char isWhiteSpaceString(char*);
void skipWhiteChar(char*);
void skipWord(char*);
void getWord(char*, char*);
void continueTo(char*, short);
void mainGeneralFree(FILE*, char*, macroNode*, declarationNode*, dataNode*, instructionNode*);
void checkIfRemainMoreFile(int);

/* Bitwise functions: */
void insertBinaryValueByKeyWords(short, short*, short);
void insertBinaryValueByData(short, short*, short);
void turnOnBit(short*, short);

/********************************************************************** - ERRORS FUNCTIONS - **********************************************************************/

void memoryError(char*, char*);
void newFileOpenError(char*, char*);
void generalError(char*, short, char*);

/*********************************************************************** - END OF HEADERS - ***********************************************************************/

