#include "headers.h"

/*
* File: general_functions.c
*
* This file is including functions that are being used in the assembler.c file, and 
* also functions that are being used in more than one file from the project files.
*
*/

/***************************************************************** - RESERVED KEYWORD FUNCTIONS - *****************************************************************/

/*
* The function: isOpcode
*
* The function checks if the provided string matches any of the predefined opcodes. 
*
* Parameters:
*   word - A pointer to a string containing the word to be checked.
*
* Returns:
*   The index of the matched opcode which represents the opcode number.
*   NO_OPCODE if the provided word does not match any of the predefined opcodes.
*/
short isOpcode(char* word)
{
	short i = 0; /* Index for iterating through the opcodes array. */
	/* The list of the opcodes that defined in the system. */
	const char *opcodes[] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop", '\0'};
		
	/* Iterate through the opcodes array to find a match. */
	while(opcodes[i]){
		if(!strcmp(word, opcodes[i]))
			return i;
		i++;	
	}
	return NO_OPCODE;
}

/*
* The function: isRegister
*
* The function checks if the provided string matches any of the predefined registers.
*
* Parameters:
*   word - A pointer to a string containing the word to be checked.
*
* Returns:
*   The index of the matched register which represents the register number.
*   NO_REGISTER if the provided word does not match any of the predefined registers.
*/
short isRegister(char* word)
{
	short i = 0; /* Index for iterating through the registers array. */
	/* The list of registers that defined in the system. */
	const char *registers[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "PSW", "PC", '\0'};
	
	/* Iterate through the registers array to find a match. */
	while(registers[i]){
		if(!strcmp(word, registers[i]))
			return i;
		i++;	
	}
	return NO_REGISTER;
}

/*
* The function: isInstruction
*
* The function checks if the provided string matches any of the predefined instruction types.
*
* Parameters:
*   word - A pointer to a string containing the word to be checked.
*
* Returns:
*   The index of the matched instruction type which represents the instruction number.
*   NO_TYPE if the provided word does not match any of the predefined instruction types.
*/
short isInstruction(char* word)
{
	short i = 0; /* Index for iterating through the instructions array. */
	/* The list of instruction types that defined in the system. */
	const char *instructions[] = {".data", ".string", ".entry", ".extern", '\0'};
	
	/* Iterate through the instructions array to find a match. */
	while(instructions[i]){
		if(!strcmp(word, instructions[i]))
			return i;
		i++;	
	}
	return NO_TYPE;
}


/*
* The function: isValidName
*
* The function checks if the provided name is valid by ensuring it does not match any reserved keywords 
* (opcodes, registers, or instructions).
*
* Parameters:
*   name - A pointer to the name to be validated.
*   error_str - A buffer for storing an error message if the name is invalid.
*
* Returns:
*   TRUE if the name is valid.
*   FALSE if the name matches any reserved keywords and an error message is generated.
*/
char isValidName(char* name, char* error_str)
{	
	/* Check if the name is used as an opcode. */
	if(isOpcode(name) != NO_OPCODE){
		sprintf(error_str, "the word: %s is used as an opcode.", name);
		return FALSE;
	}
	/* Check if the name is used as a register. */
	if(isRegister(name) != NO_REGISTER){
		sprintf(error_str, "the word: %s is used as an register.", name);
		return FALSE;
	}
	/* Check if the name is used as an instruction. */
	if(isInstruction(name) != NO_TYPE){
		sprintf(error_str, "the word: %s is used as an instruction.", name);
		return FALSE;
	}
	return TRUE;
}

/**************************************************************** - STRING MANIPULATION FUNCTIONS - ****************************************************************/

/*
* The function: isWhiteSpaceString
*
* The function checks if a given string contains only whitespace characters.
* 
* Parameters:
*   line - A pointer to the string that is being checked for whitespace characters.
*
* Returns:
*   TRUE if the string contains only whitespace characters.
*   FALSE if there is any non-whitespace character in the string.
*/
char isWhiteSpaceString(char* line)
{		
	short i = 0; /* An index to track the current position in the string. */
	
	while(line[i]){
		if(!isspace(line[i])) /* Check if the current character is not a whitespace. */
			return FALSE; /* Return FALSE if a non-whitespace character is found. */
		i++;		
	}
	return TRUE; /* Return TRUE if all characters in the string are whitespace. */
}

/*
* The function: skipWhiteChar
*
* The function skips to the first non-white character in a given line.
* 
* Parameters:
*   line - A pointer to the string from which leading whitespace characters will be removed.
*/
void skipWhiteChar(char* line)
{
	short i = 0; /* An index to track the current position in the line. */
			
	/* Count the number of leading whitespace characters. */
	while(line[i] && isspace(line[i]))
		i++;
	
	continueTo(line, i); /* Update the line to start from the first non-white character. */
}

/*
* The function: skipWord
*
* The function skips over the first word in a given line, including any leading whitespace.
* 
* Parameters:
*   line - A pointer to the string from which the word will be skipped.
*/
void skipWord(char* line)
{
	short i = 0; /* An index to track the current position in the line. */
	
	skipWhiteChar(line); /* Skip leading whitespace characters. */
		
	/* Count the number of characters until the first white character appears. */
	while(!isspace(line[i]) && line[i])
		i++;
	
	continueTo(line, i); /* Update the line to start from the character following the skipped word. */	
}

/*
* The function: getWord
*
* The function extracts the first word from a given line and updates the line to start from the character following the extracted word.
* 
* Parameters:
*   line - A pointer to the string from which the word will be extracted.
*   word - A pointer to a buffer where the extracted word will be stored.
*/
void getWord(char* line, char* word)
{
	int i = 0; /* An index to track the current position in the line. */
	
	skipWhiteChar(line);		
	while(!isspace(line[i]) && line[i] != '\n' && line[i]){
		word[i] = line[i]; /* Copy characters to the word buffer. */
		i++;	
	}
	word[i] = '\0';
	
	continueTo(line, i); /* Update the line to start from the character following the extracted word. */	
}

/*
* The function: continueTo
*
* The function modifies the given line so that it starts from the character at index `i` of the original line.
* 
* Parameters:
*   line - A pointer to the string to be modified.
*   i - The index from which the line should start.
*/
void continueTo(char* line, short i)
{
	char temp[MAX_LENGTH_OF_LINE]; /* A temporary buffer to store the modified line. */
	
	if(i < 1)
		return; /* Return without modifying if the index is less than 1. */
	
	strcpy(temp, line+i); /* Copy the portion of the line starting from index i to the temporary buffer. */
	strcpy(line, temp);	/* Copy the modified string back to the original line. */
}

/********************************************************************* - BITWISE FUNCTIONS - *********************************************************************/

/* 
* The function: insertBinaryValueByKeyWords
*
* The function receive a value of a decimal number that represent saved key words (opcodes
* and registers) and enter it's value inside the wanted bits on a short variable.
* 
* Parameters:
*	decimal_value - number that represent the value we want to enter.
*	bmc - a pointer that represent the number that we would like to change.
*	start - the index of the first bit that we want to change on variable num.
*/
void insertBinaryValueByKeyWords(short decimal_value, short* bmc, short start)
{
	short bit_position = (short)pow(2, start);/*represent the bit in the wanted index*/
	for(; decimal_value; bit_position <<= ONE_BIT, decimal_value >>= ONE_BIT){
		if(decimal_value & 01)
			*bmc |= bit_position;
	}
}

/* 
* The function: insertBinaryValueByData
*
* The function receive a value of an integer decimal number and enter it's value inside the wanted
* bits on a short variable.
* 
* Parameters:
*	decimal_value - number that represent the value we want to enter.
*	bmc - a pointer that represent the number that we would like to change.
*	start - the index of the first bit that we want to change on variable num.
*/
void insertBinaryValueByData(short decimal_value, short* bmc, short start)
{
	short decimal_with_start = (decimal_value << start);/*move the decimal number to left by the value of start*/
	*bmc |= decimal_with_start;
	*bmc &= (short)(pow(2,16)-1);/* turn off the 16 bit of short, which mean it keeps over only 15 bits of short*/	
}

/* 
* The function: turnOnBit
*
* The function receive a number, and turn the value of the bit in the wanted index to 1.
* 
* Parameters:
*	num - a pointer that represent the number that we would like to change.
*	start - the index of the bit that we want to change on variable num.
*/
void turnOnBit(short* num, short start)
{
	short bit_position = (short)pow(2, start);/*represent the bit in the wanted index*/
	*num |= bit_position;
}

/******************************************************************* - MORE GENERAL FUNCTIONS - *******************************************************************/

/* 
* The function: mainGeneralFree
*
* The function receives the main variables of this program and check if they take memory in
* the system,and if yes, it's free their memory.
* 
* Parameters:
*	input_file - a pointer for the input file.
*	input_file_name -a pointer for the input file name. 
*	macros_list - a pointer for the macros node list.
*	declarations_list -a pointer for the declarations node list.
*	data_list - a pointer for the data node list.
*	instructions_list - a pointer for the instructions node list.
*/
void mainGeneralFree(FILE* input_file, char* input_file_name, macroNode* macros_list, declarationNode* declarations_list, dataNode* data_list, instructionNode* instructions_list)    
{
	fclose(input_file);
	free(input_file_name);
	
	if(macros_list)
		freeMacrosList(macros_list);
	
	if(declarations_list)
		freeDeclarationsList(declarations_list);
	
	if(data_list)
		freeDataList(data_list);
	
	if(instructions_list)
		freeInstructionsList(instructions_list);
}

/*
* The function: checkIfRemainMoreFile
*
* The function checks if there are any more input files left to receive,
* and if so, she informs about it.
* 
* Parameters:
*	argc - the amount of input files that got left in the main function that need to be checked.
*/
void checkIfRemainMoreFile(int argc)
{
	if(argc-1 > 0)
		printf("Continue reading the next file\n");
}

/****************************************************************** - END OF GENERAL_FUNCTIONS - ******************************************************************/

