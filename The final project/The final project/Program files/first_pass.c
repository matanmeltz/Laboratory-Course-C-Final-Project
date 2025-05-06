#include "headers.h"

/*
* The function: firstPass
*
* The main function for managing the first pass stage. 
* The function iterates over all the lines in the input file, determines the type of the instruction in each line, and 
* directs the handling of the instruction line to the appropriate function. 
* Additionally, the function generates error messages for the user and updates the data that will be used later in the second pass stage of the process.
* 
* Parameters:
*   input_file - Pointer to the input file to be processed.
*   input_file_name - Name of the input file (for error reporting).
*   ic_dc_counter - Array that will store the final amount of instructions and data (for the second pass stage).
*   macros_list - Pointer to the head of the linked list of macro nodes.
*   instructions_list - Pointer to the head of the linked list of instruction nodes.
*   data_list - Pointer to the head of the linked list of data nodes.
*   declarations_list - Pointer to the head of the linked list of declaration nodes.
*
* Returns:
*	indicator = TRUE - If the first pass stage has finished without any errors being detected.
*	indicator = FALSE - If a syntax error is found in one of the lines.
* 	FATAL_ERROR - If a memory allocation error occurred.
*  
*/
char firstPass(FILE* input_file, char* input_file_name, short* ic_dc_counter, macroNode* macros_list, instructionNode** instructions_list, dataNode** data_list, declarationNode** declarations_list)
{
	short ic = FIRST_MEMORY_CELL; /* The instruction counter, initialized to start counting from the first decimal address number. */
	short dc = 0; /* the data counter. */
	char line[MAX_LENGTH_OF_LINE], error_str[ERROR_MESSAGE_SIZE] = { '\0' }; /* Arrays to store the line and the error messages. */ 
	char instruction_type; /* A variable that will be used to direct the handling of the instruction line to the appropriate function. */ 
	char indicator = TRUE; /* An indicator that represents the syntax correctness status in the input file. */
	unsigned int line_num = 1; /* A counter of the current line number */
	instructionNode *new_instruction = NULL; /* Pointer to newly created instruction node. */
	dataNode *new_data = NULL; /* Pointer to newly created data node. */
	declarationNode *new_declaration = NULL; /* Pointer to newly created declaration node. */
				
	/* Read each line from the input file. */
	while(fgets(line, MAX_LENGTH_OF_LINE, input_file)){
		/* Checks if a label definition exists on the current line. */
		if(strchr(line, ':')){
			/* Sends the line to be handled by the function dedicated to label processing. */
			new_declaration = readNewLabel(line, line_num, macros_list, dc, ic, new_declaration, declarations_list, error_str);			
			if(!new_declaration){
				memoryError(error_str, input_file_name);
				return FATAL_ERROR;
			}
		}
		/* Handle instructions, data, and declarations if no errors are found. */																								
		if(!error_str[0] && line[0] && line[0] != '\n'){
			instruction_type = getInstructionType(line, error_str);			
			
			/* Sends the line to be handled by the function dedicated to instruction lines. */
			if(instruction_type == CODE){
				new_instruction = readNewInstruction(&ic, line, new_instruction, instructions_list, line_num, error_str);
				if(!new_instruction){
					memoryError(error_str, input_file_name);
					return FATAL_ERROR;
				}
			}
			/* Sends the line to be handled by the function dedicated to declaration lines. */
			else if(instruction_type >= ENTRY){
				new_declaration = readNewDeclaration(line, line_num, macros_list, new_declaration, declarations_list, error_str);
				if(!new_declaration){
					memoryError(error_str, input_file_name);
					return FATAL_ERROR;
				}
			}
			/* Sends the line to be handled by the function dedicated to data instruction lines. */
			else if(instruction_type >= DATA){
				new_data = readNewData(&dc, instruction_type, line, new_data, data_list, error_str);
				if(!new_data){
					memoryError(error_str, input_file_name);
					return FATAL_ERROR;
				}
			}
		}
		/* Reports errors detected during the line processing process. */
		if(error_str[0]){
			indicator = 0;
			generalError(input_file_name, line_num, error_str);
			error_str[0] = '\0';
		}					
		line_num++; /* advances the line counter. */		
	}
	/* Updates the data counter to the final decimal address. */
	if(!UpdateDataCounter(*data_list, *declarations_list, ic_dc_counter, ic, dc)){
		indicator = 0;
		generalError(input_file_name, line_num, "The system cannot proceed due to insufficient available memory.");	
	}
	
	return indicator;
}

/*
* The function: UpdateDataCounter
*
* The function updates the data counter to the final decimal address of the linked lists of data and declaration.
* In addition, the function updates the array that stores the final amount of instructions and data.
*
* Parameters:
*   data_list - A pointer to the head of the linked list of data nodes.
*   declarations_list - A pointer to the head of the linked list of declaration nodes.
*   ic_dc_counter - The array that will store the final amount of instructions and data.
*   ic - Current instruction counter value.
*   dc - Current data counter value.
*
* Returns:
*   TRUE - If the amount of the final decimal address fits in the memory size.
*	FALSE - If the amount of the decimal address is too large for the size of the memory.
*/
char UpdateDataCounter(dataNode* data_list, declarationNode* declarations_list, short* ic_dc_counter, short ic, short dc)
{
	dataNode* temp_data = data_list; /* Temporary pointer to traverse the data nodes. */
	declarationNode* temp_declaration = declarations_list; /* Temporary pointer to traverse the declaration nodes. */
	
	/* Updates the final amount of instructions and data. */
	ic_dc_counter[0] = ic-FIRST_MEMORY_CELL;
	ic_dc_counter[1] = dc;
	
	/* Updates the decimal addresses for each data node. */
	while(temp_data){
		temp_data->dc += ic;
		/* Check if the updated address exceeds the memory limit. */
		if(temp_data->dc > MEMORY_CELLS)
			return FALSE;
		temp_data = temp_data->next;
	}
	/* Updates the decimal addresses for each declaration node, of type data or string */
	while(temp_declaration){
		if(temp_declaration->type == DATA || temp_declaration->type == STRING)
			temp_declaration->decimal_address += ic;
		temp_declaration = temp_declaration->next;
	}
	
	return TRUE;	
}

/***************************************************************** - INSTRUCTION NODE FUNCTIONS - *****************************************************************/

/*
* The function: readNewInstruction
*
* The function handles the parsing, validation, encoding, and creation of instruction nodes from a given instruction line.
* It orchestrates the process of extracting operands, validating them, encoding the instructions into BMC format, and 
* creating new instruction nodes according to the instruction type and operands.
*
* Parameters:
*   ic - A pointer to the current instruction counter, which is incremented for each new instruction.
*   line - A string containing the instruction line.
*   curr_instruction - A pointer to the currently last instruction node in the linked list.
*   instructions_list - A pointer to the head of the linked list of instructions.
*   line_num - The line number in the input file where the instruction is found.
*   error_str - A pointer to a buffer where error messages will be stored.
*
* Returns:
*   A pointer to the newly created `instructionNode`, or NULL if a memory allocation error occurs.
*/
instructionNode* readNewInstruction(short* ic, char* line, instructionNode* curr_instruction, instructionNode** instructions_list, unsigned int line_num, char* error_str)
{
	instructionNode *new_instruction = NULL; /* A pointer for the new instruction node. */
	char source_operand [MAX_LENGTH_OF_LINE], target_operand [MAX_LENGTH_OF_LINE]; /* Buffers for the source and target operands. */
	short opcode, source_operand_type, target_operand_type; /* Variables to store the opcode number and the operands type. */
	short first_word_bmc = 0, source_bmc = 0, target_bmc = 0; /* Variables to store the BMC values for the instruction and the operands. */	
		
	opcode = getOpcode(line); /* Retrieves the opcode number according to the opcode name in the line. */
			
	/* For the first group of instructions: */
	if(opcode <= LEA){
		/* Extracts the source and target operands from the instruction line. */
		if(!readTwoOperands(line, source_operand, target_operand, error_str) && curr_instruction)
			return curr_instruction;
		/* Checks and gets the type of each operand. */	
		source_operand_type = isValidSourceOperandType(opcode, source_operand, error_str);	
		target_operand_type = isValidTargetOperandType(opcode, target_operand, error_str);
		if(((source_operand_type == NO_TYPE) || (target_operand_type == NO_TYPE)) && curr_instruction)
			return curr_instruction;
			
		/* Encode the first word of the instruction into BMC representation. */
		getFirstWordBMC(opcode, source_operand_type, target_operand_type, &first_word_bmc);
		
		/* Creates a new instruction node for the instruction line. */		
		new_instruction = creatInstructionNode((*ic)++, first_word_bmc, NULL, line_num, &curr_instruction, error_str);
		if(new_instruction){
			/* Updates the head of the linked list, in case the new instruction node is the first node in the linked list. */					
			if(!(*instructions_list))
				*instructions_list = new_instruction;
			
			/* Creates a BMC encoding in case both operands are of register type. */						
			if((source_operand_type >= INDIRECT_REGISTER_ADDRESSING) && (target_operand_type >= INDIRECT_REGISTER_ADDRESSING)){
				getSourceRegisterBMC(source_operand, &source_bmc);	
				getTargetRegisterBMC(target_operand, &source_bmc);
				
				/* Creates one new instruction node. */
				new_instruction = creatInstructionNode((*ic)++, source_bmc, source_operand, line_num, &curr_instruction, error_str);									
			}
			/* Creates a BMC encoding according to the type of each operand. */
			else{				
				if(source_operand_type == IMMEDIATE_ADDRESSING)
					getImmediateBMC(source_operand, &source_bmc);
					
				if(source_operand_type >= INDIRECT_REGISTER_ADDRESSING)
					getSourceRegisterBMC(source_operand, &source_bmc);
			
				if(target_operand_type == IMMEDIATE_ADDRESSING)
					getImmediateBMC(target_operand, &target_bmc);	
					
				if(target_operand_type >= INDIRECT_REGISTER_ADDRESSING)
					getTargetRegisterBMC(target_operand, &target_bmc);
				
				/* Creates a new instruction node for each operand. */
				new_instruction = creatInstructionNode((*ic)++, source_bmc, source_operand, line_num, &curr_instruction, error_str);
				if(new_instruction)		
					new_instruction = creatInstructionNode((*ic)++, target_bmc, target_operand, line_num, &curr_instruction, error_str);							
			}
		}
	}
	/* For the second group of instructions: */
	else if(opcode <= JSR){
		/* Extracts the target operand from the line. */
		if(!readTargetOperand(line, target_operand, error_str) && curr_instruction)
			return curr_instruction;
		/* Checks and gets the type of the operand. */
		target_operand_type = isValidTargetOperandType(opcode, target_operand, error_str);
		if((target_operand_type == NO_TYPE) && curr_instruction)
			return curr_instruction;
		
		/* Encode the first word of the instruction into BMC representation. */		
		getFirstWordBMC(opcode, NO_TYPE, target_operand_type, &first_word_bmc);
		if(target_operand_type == IMMEDIATE_ADDRESSING)
			getImmediateBMC(target_operand, &target_bmc);
		
		/* Creates a BMC encoding according to the type of the operand. */
		if(target_operand_type >= INDIRECT_REGISTER_ADDRESSING)
			getTargetRegisterBMC(target_operand, &target_bmc);
		
		/* Creates a new instruction node for the instruction line. */
		new_instruction = creatInstructionNode((*ic)++, first_word_bmc, NULL, line_num, &curr_instruction, error_str);
		if(new_instruction){
			/* Updates the head of the linked list, in case the new instruction node is the first node in the linked list. */
			if(!(*instructions_list))
				*instructions_list = new_instruction;
			
			/* Creates a new instruction node for the operand. */
			new_instruction = creatInstructionNode((*ic)++, target_bmc, target_operand, line_num, &curr_instruction, error_str);
		}	
	}
	/* For the third group of instructions: */
	else{
		/* Checks if the instruction line contains operands when none are expected. */
		if(!isWhiteSpaceString(line)){
			strcpy(error_str, "The number of operands does not match the opcode type.");
		}
		/* Encode the first word of the instruction into BMC representation. */
		getFirstWordBMC(opcode, NO_TYPE, NO_TYPE, &first_word_bmc);
		
		/* Creates a new instruction node for the instruction line. */
		new_instruction = creatInstructionNode((*ic)++, first_word_bmc, NULL, line_num, &curr_instruction, error_str);
		if(new_instruction){
			/* Updates the head of the linked list, in case the new instruction node is the first node in the linked list. */
			if(!(*instructions_list))
				*instructions_list = new_instruction;
		}	
	}	
	return new_instruction;		
}

/*
* The function: creatInstructionNode
*
* The function creates a new `instructionNode` with the given parameters and adds it to the end of the linked list of instructions.
*
* Parameters:
*   current_ic - The current instruction counter value to be assigned to the new node.
*   new_bmc - The BMC value associated with the new node.
*   label - The label name for the new instruction node, used if it's found that the operand is of a direct addressing type.
*   line_num - The line number in the input file where the label is mentioned, this will be used in case an error is found during the second pass.
*   curr_instruction - A pointer to the currently last instruction node in the linked list.
*   error_str - A pointer to a buffer where an error message will be stored if memory allocation fails.
*
* Returns:
*   A pointer to the newly created `instructionNode`, or NULL if memory allocation fails.
*/
instructionNode* creatInstructionNode(short current_ic, short new_bmc, char* label, unsigned int line_num, instructionNode** curr_instruction, char* error_str)
{	
	instructionNode *new_instruction; /* A pointer for the new instruction node. */
	
	/* Allocate memory for the new instructionNode. */
	new_instruction = (instructionNode*)malloc(sizeof(instructionNode));
	if(!new_instruction){
		strcpy(error_str, "new instructionNode");
		return NULL;
	}
				
	new_instruction->ic = current_ic; /* Set the current instruction counter for the new node. */
	
	/* Check if it is an absolute type encoding or an address type encoding. */
	if(isAbsolute(new_bmc)){
		/* The case that the word can be encoded in the first pass. */
		new_instruction->line = 0; /* Represents that there is a BMC representation. */
		new_instruction->type.bmc = new_bmc;
	}
	/* Handles the case that the word cannot be encoded in the first pass. */
	else{
		new_instruction->line = line_num; /* Set the number of the line where the name of the label is mentioned. */		
		/* Allocate memory for the name of the label instructionNode. */
		new_instruction->type.name = (char*)malloc(sizeof(char) * MAX_LENGTH_OF_LINE);
		if(!(new_instruction->type.name)){
			strcpy(error_str, "new instructionNode->type.name");
			free(new_instruction);
			return NULL;
		}
		strcpy(new_instruction->type.name, label); /* Set the label for the instruction node. */
	}		
	new_instruction->next = NULL; /* Initialize the next pointer to NULL. */
	
	/* Link the new node to the end of the list, if the current node already exists. */
	if(*curr_instruction)
		(*curr_instruction)->next = new_instruction;
		
	*curr_instruction = new_instruction; /* Updates the curr_instruction node to point to the address of the new_instruction node. */
	
	return new_instruction;
}

/*
* The function: isAbsolute
*
* The function checks if the given BMC value represents an absolute addressing method.
*
* Parameters:
*   bmc - The BMC value to be checked.
*
* Returns:
*   TRUE - If the BMC value represents an absolute addressing type.
*	FALSE - If the BMC value represents a direct addressing type or not yet initialized. 
*/
char isAbsolute(short bmc)
{
	/* Calculate the bitmask for the absolute addressing method. */
	short absolute = (short)pow(2, A_ADDRESSING_METHOD);
	
	/* Check if the BMC value includes the absolute addressing method bit. */
	if(bmc & absolute)
		return TRUE;
	
	return FALSE;
}

/*
* The function: getFirstWordBMC
*
* The function encodes the first word of an instruction into its BMC representation based on the opcode and operands type.
*
* Parameters:
*   opcode - The opcode of the instruction, used to determine the corresponding binary value.
*   source_operand_type - The type of the source operand, used to set the appropriate bits in the BMC representation.
*   target_operand_type - The type of the target operand, used to set the appropriate bits in the BMC representation.
*   bmc - A pointer to the variable where the BMC representation of the instruction will be stored.
*/
void getFirstWordBMC(short opcode, short source_operand_type, short target_operand_type, short* bmc)
{		
	/* Inserts the binary value of the opcode into the BMC representation. */
	insertBinaryValueByKeyWords(opcode, bmc, OPCODE_START_POSITION);
	
	/* Sets the addressing method bit in the BMC representation. */
	turnOnBit(bmc, A_ADDRESSING_METHOD);
	
	/* Checks if the operand exist on the command line and, if so, sets the corresponding bit in the BMC representation. */
	if(target_operand_type != NO_TYPE)		
		turnOnBit(bmc, TARGET_START_POSITION + target_operand_type);
	
	if(source_operand_type != NO_TYPE)
		turnOnBit(bmc, SOURCE_START_POSITION + source_operand_type);
}

/*
* The function: getImmediateBMC
*
* The function converts the numeric value of an operand of immediate addressing type to its BMC representation.
*
* Parameters:
*   operand - A pointer to a string that holds the immediate operand.
*   bmc - A pointer to the variable where the BMC representation of the immediate operand will be stored.
*/
void getImmediateBMC(char* operand, short* bmc)
{
	short operand_value; /* Stores the value of the numeric part of the immediate operand. */
	
	/* Convert the numeric part of the immediate operand to a short integer. */
	operand_value = (short)atof(operand+1);
	
	/* Gets the BMC representation of the immediate operand. */
	insertBinaryValueByData(operand_value, bmc, OPERAND_START_POSITION);
	turnOnBit(bmc, A_ADDRESSING_METHOD);	
}

/*
* The function: getSourceRegisterBMC
*
* The function converts the source register's numeric value into its BMC representation.
*
* Parameters:
*   source_register - A pointer to a string that holds the source register.
*   bmc - A pointer to the variable where the BMC representation of the source register will be stored.
*/
void getSourceRegisterBMC(char* source_register, short* bmc)
{
	short operand_value; /* Stores the value of the numeric part of the source register. */
	
	/* Convert the numeric part of the source register to a short integer. */
	operand_value = (short)atoi(source_register+1);
	
	/* Gets the BMC representation of the source register. */
	insertBinaryValueByKeyWords(operand_value, bmc, SOURCE_REGISTER_START_POSITION);
	turnOnBit(bmc, A_ADDRESSING_METHOD);	
}

/*
* The function: getTargetRegisterBMC
*
* The function converts the target register's numeric value into its BMC representation.
*
* Parameters:
*   target_register - A pointer to a string that holds the target register.
*   bmc - A pointer to the variable where the BMC representation of the target register will be stored.
*/
void getTargetRegisterBMC(char* target_register, short* bmc)
{
	short operand_value; /* Stores the value of the numeric part of the target register . */
	
	/* Convert the numeric part of the target register to a short integer. */
	operand_value = (short)atoi(target_register+1);	
	
	/* Gets the BMC representation of the target register. */
	insertBinaryValueByKeyWords(operand_value, bmc, TARGET_REGISTER_START_POSITION);
	if(!isAbsolute(*bmc))
		turnOnBit(bmc, A_ADDRESSING_METHOD);		
}

/*
* The function: isValidTargetOperandType
*
* The function determines if the target operand's addressing type is valid for the given opcode.
*
* Parameters:
*   opcode - The numerical value representing the opcode to be checked.
*   target_operand - A pointer to the string containing the target operand to be validated.
*   error_str - A pointer to a buffer where an error message will be stored if validation fails.
*
* Returns:
*   The addressing type of the target operand if valid.
*   NO_TYPE - If the operand type does not match the opcode or an error is detected.
*/
short isValidTargetOperandType(short opcode, char* target_operand, char* error_str)
{	
	short target_operand_type; /* A variable to store the addressing type of the target operand. */
	
	/* Get the addressing type of the target operand. */
	target_operand_type = getOperandAddressingType(target_operand, error_str);
	if(target_operand_type == NO_TYPE)
		return NO_TYPE;
	
	/* Check if the target operand type is valid for the given opcode. */	
	if((target_operand_type >= IMMEDIATE_ADDRESSING) && ((opcode == CMP) || (opcode == PRN)))	
		return target_operand_type;
		
	if((target_operand_type >= DIRECT_ADDRESSING) && ((opcode <= DEC) || (opcode == RED)))
		return target_operand_type;
		
	if(((target_operand_type == DIRECT_ADDRESSING) || (target_operand_type == INDIRECT_REGISTER_ADDRESSING)) && (opcode <= JSR))
		return target_operand_type;
	
	/* If no valid type is found, set the error message and return NO_TYPE. */
	strcpy(error_str, "Invalid target operand type, the operand type does not match the opcode type.");
	
	return NO_TYPE;	
}

/*
* The function: isValidSourceOperandType
*
* The function determines if the source operand's addressing type is valid for the given opcode.
*
* Parameters:
*   opcode - The numerical value representing the opcode to be checked.
*   source_operand - A pointer to the string containing the source operand to be validated.
*   error_str - A pointer to a buffer where an error message will be stored if validation fails.
*
* Returns:
*   The addressing type of the source operand if valid.
*   NO_TYPE - If the operand type does not match the opcode or an error is detected.
*/
short isValidSourceOperandType(short opcode, char* source_operand, char* error_str)
{	
	short source_operand_type; /* A variable to store the addressing type of the source operand. */
	
	/* Get the addressing type of the source operand. */
	source_operand_type = getOperandAddressingType(source_operand, error_str);
	
	/* Check if the source operand type is valid for the given opcode. */
	if(source_operand_type == NO_TYPE)
		return NO_TYPE;
	
	if((opcode <= LEA) && (source_operand_type == DIRECT_ADDRESSING))
		return source_operand_type;
		
	if((opcode <= SUB) && (source_operand_type >= IMMEDIATE_ADDRESSING))
		return source_operand_type;
	
	/* If no valid type is found, set the error message and return NO_TYPE. */
	strcpy(error_str, "Invalid source operand type, the operand type does not match the opcode type.");
	
	return NO_TYPE;	
}

/*
* The function: getOperandAddressingType
*
* The function determines the addressing type of a given operand and returns the corresponding type.
*
* Parameters:
*   operand - A pointer to the string containing the operand to be analyzed.
*   error_str - A pointer to a buffer where an error message will be stored if validation fails.
*
* Returns:
*   The addressing type of the operand
*	NO_TYPE - If an error is detected.
*/
short getOperandAddressingType(char* operand ,char* error_str)
{
	double num; /* Stores the numerical value of an operand in immediate addressing. */
	char tmp_error_str[ERROR_MESSAGE_SIZE]; /* Temporary buffer for storing error messages from isValidName. */
	
	/* Check if the operand is an immediate addressing type. */
	if(operand[0] == '#'){
		if(operand[1]){
			if(!(num = atof(operand+1))){
				strcpy(error_str, "Invalid operand, the character after the hash symbol is not a number.");
				return NO_TYPE;
			}
			if(num != (int)num){
				strcpy(error_str, "Invalid operand, the number is not an integer.");
				return NO_TYPE;		
			}
			if(!checkBitException((int)num, (BITS_NUM - OPERAND_START_POSITION -1))){
				strcpy(error_str, "Invalid operand, the number is too large or too small.");
				return NO_TYPE;;
			}
			return IMMEDIATE_ADDRESSING;
		}
		else{
			strcpy(error_str, "Invalid operand, no number detected after the hash symbol.");
			return NO_TYPE;
		}		
	}
	/* Check if the operand is an indirect register addressing type. */
	if(operand[0] == '*'){
		if(operand[1]){
			if(isRegister(operand+1) == NO_REGISTER){
				strcpy(error_str, "Invalid operand, the register name does not exist in the system.");
				return NO_TYPE;
			}
			continueTo(operand, 1);
			return INDIRECT_REGISTER_ADDRESSING;
		}
		else{
			strcpy(error_str, "Invalid operand, missing register name after the asterisk symbol.");
			return NO_TYPE;
		}
	}
	/* Check if the operand is a direct register addressing type. */
	if(isRegister(operand) != NO_REGISTER)
		return DIRECT_REGISTER_ADDRESSING;
	
	 /* Validate if the operand is not used as a system keyword. */	
	if(!isValidName(operand, tmp_error_str)){
		strcpy(error_str, "Invalid operand, ");
		strcat(error_str, tmp_error_str);
		return NO_TYPE;
	}
	
	return DIRECT_ADDRESSING;
}

/*
* The function: readTargetOperand
*
* The function extracts the target operands from a given line and ensuring the correctness of the format.
*
* Parameters:
*   line - A pointer to the string containing the line to be processed.
*   target_operand - A pointer to the buffer where the target operand will be stored.
*   error_str - A pointer to a buffer where an error message will be stored if validation fails.
*
* Returns:
*   TRUE - If the target operand is valid.
*	FALSE - If an error is detected.
*/
char readTargetOperand(char* line, char* target_operand, char* error_str)
{
	char* temp; /* A pointer to check if another operand exists. */
	
	/* Check for additional operands in the line. */
	temp = strchr(line, ',');
	if(temp && !isWhiteSpaceString(temp+1)){
		strcpy(error_str, "The number of operands does not match the opcode type.");
		return FALSE;
	}	
	getWord(line, target_operand); /* Extract the target operand from the line. */
	
	/* Validate the target operand's presence. */
	if(!strlen(target_operand)){
		strcpy(error_str, "The target operand is missing.");
		return FALSE;
	}
	/* Check for extra characters after the target operand. */
	if(!isWhiteSpaceString(line)){
		strcpy(error_str, "Extra characters was detected after the target operand.");
		return FALSE;
	}
	
	return TRUE;
}

/*
* The function: readTwoOperands
*
* The function extracts the source and target operands from a given line and ensuring the correctness of the format.
*
* Parameters:
*   line - A pointer to the string containing the line to be processed.
*   source_operand - A pointer to a buffer where the source operand will be stored.
*   target_operand - A pointer to a buffer where the target operand will be stored.
*   error_str - A pointer to a buffer where an error message will be stored if the operands' format is incorrect.
*
* Returns:
*   TRUE - If the operands are correctly extracted. 
*	FALSE - If there is an error in the operands format or amount of operands.
*/
char readTwoOperands(char* line, char* source_operand, char* target_operand, char* error_str)
{
	short i = 0, j = 0; /* Indexes for traversing the strings. */
	char *tail; /* Pointer to track the position of the comma separating the operands. */
	
	skipWhiteChar(line);/* Skip initial white spaces in the line */
	
	/* Locate the comma that separates the two operands */	
	tail = strchr(line, ',');
	if(!tail){
		strcpy(error_str, "The number of operands does not match the opcode type.");
		return FALSE;
	}
	 /* Extract the source operand */
	while(line+i != tail){
		if(isspace(line[i])){
			i++;
			continue;
		}
		source_operand[j++] = line[i++];
	}
	source_operand[j] = '\0';
	
	/* Check if the source operand is missing */
	if(!strlen(source_operand)){
		strcpy(error_str, "The source operand is missing.");
		return FALSE;
	}
	continueTo(line, ++i); /* Skip the comma and move to the target operand */
	
	/* Check for additional commas (which would indicate an error) */
	tail = strchr(line, ',');
	if(tail){
		strcpy(error_str, "Two commas were detected on the command line.");
		return FALSE;
	}
	/* Extract the target operand and validate it */
	if(!readTargetOperand(line, target_operand, error_str))
		return FALSE;
	
	return TRUE;
}

/*
* The function: getOpcode
*
* The function extracts the opcode from a given line and returns its corresponding numerical value.
*
* Parameters:
*   line - A pointer to the string containing the line to be processed.
*
* Returns:
*   The numerical value of the opcode if found, or a negative value if the opcode is not valid.
*/
short getOpcode(char* line)
{
	char opcode[MAX_LENGTH_OF_LINE]; /* Array to store the extracted opcode. */
	short opcode_num; /* Variable to store the numerical value of the opcode. */
	
	getWord(line, opcode); /* Extract the opcode from the line */
	opcode_num = isOpcode(opcode); /* Determine the numerical value of the opcode */

	return opcode_num;
}

/*
* The function: freeInstructionsList
*
* The function responsible for freeing the memory allocated for a linked list of instructionNode structures, 
* including the dynamically allocated memory for the instruction data.
* 
* Parameters:
*   instructions_list - A pointer to the head of the linked list of instructionNode structures to be freed. 
*/
void freeInstructionsList(instructionNode* instructions_list)
{
	instructionNode* temp; /* A temporary pointer to hold the next node in the list. */
	
	/* Iterating through the linked list until all nodes are freed. */	
	while(instructions_list){
		/* If line is non-zero then there is memory allocation for the mentioned label name in the instruction. */
		if(instructions_list->line)
			free(instructions_list->type.name);									
		temp = instructions_list->next; /* Store the next node in the list. */
		free(instructions_list); /* Free the current node. */
		instructions_list = temp; /* Move to the next node in the list. */
	}
}

/******************************************************************** - DATA NODE FANCTIONS - ********************************************************************/

/*
* The function: readNewData
*
* The function handles the parsing, validation, and creation of data nodes from a given data instruction line. 
* It processes the line based on the type of data instruction (either .data or a .string), encodes the data into 
* the BMC format, and creates new data nodes in the linked list accordingly.
*
* Parameters:
*   dc - A pointer to the current data counter value, which will be incremented as new data nodes are created.
*   type - A short value indicating the type of data (either DATA or STRING).
*   line -  A string containing the data instruction line.
*   curr_data - A pointer to the currently last data node in the linked list.
*   data_list - A pointer to the head of the linked list of data nodes.
*   error_str - A pointer to a buffer where an error message will be stored.
*
* Returns:
*   A pointer to the newly created data node if successful, or NULL if memory allocation fails.
*/
dataNode* readNewData(short* dc, short type, char* line, dataNode* curr_data, dataNode** data_list, char* error_str)
{
	dataNode* new_data; /* Pointer to the newly created data node. */
	short data_arr[MAX_LENGTH_OF_LINE] = {0}; /* Buffer for storing the parsed data elements. */
	short length; /* Length of the parsed data elements array. */
	short i = 0; /* Index for iterating through the data elements array. */
	
	skipWord(line);	/* Skips the data instruction word in the line. */
	
	/* Reads and parses the data instruction line according to the specified type. */
	if(type == DATA)
		length = readNumbers(line, data_arr, error_str);
	if(type == STRING)
		length = readString(line, data_arr, error_str);
	
	/* If no data was processed. */	
	if((length == 0) && curr_data)
		return curr_data;
	
	/* Iterates through the parsed data elements and creates new data nodes. */
	while(i < length){
		new_data = creatDataNode((*dc)++, data_arr[i++], &curr_data, error_str);
		if(!new_data)
			return NULL;
		
		/* Updates the head of the linked list if the new data node is the first node. */
		if(!(*data_list))
			*data_list = new_data;

	}
	return new_data;
}

/*
* The function: creatDataNode
*
* The function creates a new data node, initializes it with the given data counter and binary machine code values, and links the new data node to the end of the linked list.
*
* Parameters:
*   current_dc - The current data counter value.
*   new_bmc - The binary machine code value to be stored in the new data node.
*   curr_data - A pointer to the currently last data node in the linked list.
*   error_str - A pointer to a buffer where an error message will be stored if memory allocation fails.
*
* Returns:
*   A pointer to the newly created data node if successful, or NULL if memory allocation fails.
*/
dataNode* creatDataNode(short current_dc, short new_bmc, dataNode** curr_data, char* error_str)
{	
	dataNode *new_data; /* Pointer to the newly created data node. */
	
	/* Allocate memory for the new data node. */
	new_data = (dataNode*)malloc(sizeof(dataNode));
	if(!new_data){
		strcpy(error_str, "new dataNode");
		return NULL;
	}
	/* Initialize the new data node with the given data counter and binary machine code. */			
	new_data->dc = current_dc;
	new_data->bmc = new_bmc;			
	new_data->next = NULL;
	
	/* Link the new node to the end of the linked list. */
	if(*curr_data)
		(*curr_data)->next = new_data;
		
	*curr_data = new_data; /* Updates the curr_data node to point to the address of the new_data node. */
		
	return new_data;
}

/*
* The function: readNumbers
*
* The function parses a line of text containing numbers separated by commas, checks their validity, and stores the numbers in the given list.
*
* Parameters:
*   line - A pointer to the string containing the line of numbers to be processed.
*   numbers_list - A pointer to the array where the numbers will be stored.
*   error_str - A pointer to a buffer where an error message will be stored if a validation fails.
*
* Returns:
*   The number of numbers stored in the numbers_list array. 
*	FALSE - If a validation fails.
*/
short readNumbers(char* line, short* numbers_list, char* error_str)
{
	char *token; /* Pointer to the current token in the line. */
	short length = 0; /* Counts to the number of numbers stored. */
	double num; /* Temporary variable to store the current number being processed. */

	/* Check if the line is valid according to .data instruction format. */
	if(!dataLineCheck(line, error_str))
		return FALSE;
	
	/* Split the line into tokens separated by commas. */		
	token = strtok(line, ",");	
	while(token){
		num = atof(token);
		
		/* Check if the number is a non-integer. */
		if(num != (int)num){
			strcpy(error_str, "Invalid data instruction, a non-integer number was detected.");
			return FALSE;	
		}
		/* Check if the number fits within the allowed bit range. */
		if(!checkBitException((int)num, BITS_NUM-1)){
			strcpy(error_str, "Invalid data instruction, a number that is too large or too small was detected.");
			return FALSE;
		}
		/* Store the number in the numbers_list array. */
		insertBinaryValueByData((short)num, &numbers_list[length++], 0);
						
		token = strtok(NULL, ",");  /* Get the next token. */
	}

	return length;	
}

/*
* The function: dataLineCheck
*
* The function checks if a line containing a .data instruction is valid by ensuring proper formatting of the numbers and commas.
*
* Parameters:
*   line - A pointer to the string containing the line to be checked.
*   error_str - A pointer to a buffer where an error message will be stored if the line is not valid.
*
* Returns:
*   TRUE - If the line is valid according to the .data instruction format.
*   FALSE - If the line is not valid or contains formatting errors.
*/
char dataLineCheck(char* line, char* error_str)
{
	char *tail; /* Pointer to the last occurrence of a comma in the line. */
	short i = 0; /* Index for traversing the characters in the line. */
	short j = 0; /* Counter for consecutive commas. */
	
	skipWhiteChar(line); /* Skip initial white spaces in the line. */
	
	/* Check if the line is empty after the .data instruction. */
	if(!line[0]){
		strcpy(error_str, "Invalid data instruction, no number was detected after the instruction name.");	
		return FALSE;
	}
	/* Check if the line starts with a comma. */
	if(line[0] == ','){
		strcpy(error_str, "Invalid data instruction, a comma was detected before the first number.");
		return FALSE;
	}
	/* Check if the line ends with a comma. */
	tail = strrchr(line, ',');
	if(tail && isWhiteSpaceString(tail+1)){
		strcpy(error_str, "Invalid data instruction, a comma was detected without a number following it.");
		return FALSE;
	}
	/* Traverse the line to check for formatting issues. */		
	while(line[i]){
		if(line[i] == ',')
			j++;		
		else if(!isspace(line[i]))
			j = 0;		
		if(j > 1){
			strcpy(error_str, "Invalid data instruction, two consecutive commas were detected without a digit between them.");
			return FALSE;
		}
		i++;
	}
	return TRUE;
}

/*
* The function: checkBitException
*
* The function checks if the given integer can fit within the range of the number of bits received.
*
* Parameters:
*	num - The integer value to check.
*	max_bits - Number of bits for the signed integer representation.
*
* Returns:
*	TRUE - If the integer is within range.
*	FALSE - If the integer is out of range.
*/
char checkBitException(int num, short max_bits)
{
	int max_value = pow(2,max_bits) -1; /* Maximum representable value. */
	int min_value = -pow(2,max_bits); /* Minimum representable value. */
	
	return ((max_value >= num) && (num >= min_value))? TRUE : FALSE;
}

/*
* The function: readString
*
* The function reads a string from the line, checks its validity, and stores the characters in the given list.
*
* Parameters:
*   line - A pointer to the string containing the line to be processed.
*   char_list - A pointer to an array where the characters of the string will be stored.
*   error_str - A pointer to a buffer where an error message will be stored if a validation fails.
*
* Returns:
*   The length of the string (including the null terminator) if successful.
*   FALSE - If the string is not valid or an unprintable character is detected.
*/
short readString(char* line, short* char_list, char* error_str)
{
	char *temp; /* Pointer to traverse the string within the line. */
	short length = 0; /* Counter to track the length of the string being processed. */
	
	skipWhiteChar(line); /* Skip initial white spaces in the line. */
	
	/* Check if the line is a valid string. */
	if(!stringLineCheck(line, error_str))
		return FALSE;
	
	/* Start reading the string after the opening quotation mark. */
	temp = line+1;
	while(temp[length] != '"'){
		/* Check for unprintable characters within the string. */
		if(!isprint(temp[length])){
			strcpy(error_str, "Invalid string instruction, a non-printable character was detected.");
			return FALSE;
		}
		/* Store the current character in the char_list and increment the length counter. */
		char_list[length] = temp[length];
		length++;
	}
	char_list[length++] = '\0';  /* Add the null terminator at the end of the string in char_list. */
	
	return length; 	
}

/*
* The function: stringLineCheck
*
* The function checks if a given line is correctly formatted as a string.
*
* Parameters:
*   line - A pointer to the string to be checked.
*   error_str - A pointer to a buffer where an error message will be stored if a validation fails.
*
* Returns:
*   TRUE - If the line is correctly formatted as a string.
*   FALSE - If the line is not correctly formatted.
*/
char stringLineCheck(char* line, char* error_str)
{
	char *tail1, *tail2; /* Pointers to track the positions of the first and the second quotation marks. */
	
	/* Checks if the line starts with a quotation mark. */
	if(*line != '"'){
		strcpy(error_str, "Invalid string instruction, a character was detected before the first quotation mark.");
		return FALSE;
	}
	/* Finds the first quotation mark in the line. */
	tail1 = strchr(line, '"');
	if(!tail1){
		strcpy(error_str, "Invalid string instruction, no quotation marks were detected.");
		return FALSE;
	}
	tail1 = tail1+1; 
	
	/* Finds the closing quotation mark in the line. */
	tail2 = strchr(tail1, '"');
	if(!tail2){
		strcpy(error_str, "Invalid string instruction, no closing quotation marks were detected.");
		return FALSE;
	}
	 /* Checks if there are any characters after the closing quotation mark. */
	if(!isWhiteSpaceString(tail2+1)){
		strcpy(error_str, "Invalid string instruction, characters were detected after the second quotation mark.");
		return FALSE;
	}
	
	return TRUE;
}

/*
* The function: freeDataList
*
* The function responsible for freeing the memory allocated for a linked list of dataNode structures.
* 
* Parameters:
*   data_list - A pointer to the head of the linked list of dataNode structures to be freed.
*/
void freeDataList(dataNode* data_list)
{
	dataNode* temp; /* A temporary pointer to hold the next node in the list. */
		
	/* Iterating through the linked list until all nodes are freed. */
	while(data_list){								
		temp = data_list->next; /* Store the next node in the list. */
		free(data_list); /* Free the current node. */
		data_list = temp; /* Move to the next node in the list. */
	}
}

/***************************************************************** - DECLARATION NODE FUNCTIONS - *****************************************************************/

/*
* The function: readNewDeclaration
*
* The main function to handle the process of creating a new declaration label. (e.g. entery and external)
*
* Parameters:
*   line - A pointer to the string containing the line to be processed.
*   line_num - The number of the line being processed.
*   macros_list - A pointer to the head of the linked list of macro nodes.
*   curr_declaration - A pointer to the currently last declaration node in the linked list.
*   declarations_list - A pointer to the head of the linked list of declaration nodes.
*   error_str - A pointer to a buffer where an error message will be stored if a validation fails or memory allocation fails.
*
* Returns:
*   A pointer to the newly created declaration node if successful, or NULL if memory allocation fails.
*/
declarationNode* readNewDeclaration(char* line, unsigned int line_num, macroNode* macros_list, declarationNode* curr_declaration, declarationNode** declarations_list, char* error_str)
{
	char label_name[MAX_LENGTH_OF_LINE]; /* Holds the extracted label name from the line. */
    char instruction_type; /* Stores the type of instruction identified in the line. */
    declarationNode *new_declaration; /* Pointer to the newly created declaration node. */
	
	instruction_type = getInstructionType(line, error_str); /* Determines the type of instruction in the line. */
	skipWord(line);
	
	/* Extracts the label name from the line and validates its length. */
	getWord(line, label_name);
	if(strlen(label_name) > MAX_LENGTH_OF_LABEL-1){
		strcpy(error_str, "Invalid label name, label name contains more than 31 characters.");
		label_name[MAX_LENGTH_OF_LABEL] = '\0';
	}
	/* Checks for additional characters after the label name and sets an error message if found. */
	if(!isWhiteSpaceString(line)){
		sprintf(error_str, "Invalid label declaration, additional characters after the label name: %s", label_name);
		if(curr_declaration)
			return curr_declaration;
	}
	
	/* Check the correctness of the name of the label. */	
	if(!checkLabelName(label_name, macros_list, error_str) && curr_declaration)
		return curr_declaration;
		
	/* Checks for conflicts with existing labels. */
	if(!checkIfLabelNameAlreadyDeclared(label_name, instruction_type, *declarations_list, error_str) && curr_declaration)
		return curr_declaration;
	
	/* Creates a new declaration node and handles memory allocation failure. */			
	new_declaration = creatDeclarationNode(line_num, label_name, instruction_type, curr_declaration, error_str);
	if(!new_declaration)
		return NULL;	
	
	/* Updates the head of the linked list, in case the new declaration node is the first declaration node created. */
	if(!(*declarations_list))
		*declarations_list = new_declaration;
	
	return new_declaration;

}

/*
* The function: readNewLabel
*
* The function processes a label declaration from a given line, verify that the label name is correct, 
* and creates a new declaration node in the linked list accordingly. 
* It handles defining labels for several different types of instructions (data, string, code).
*
* Parameters:
*   line - A pointer to the string containing the line to be processed.
*   line_num - The number of the line being processed.
*   macros_list - A pointer to the head of the linked list of macro nodes.
*   dc - The current data counter value.
*   ic - The current instructions counter value.
*   curr_declaration - A pointer to the currently last declaration node in the linked list.
*   declarations_list - A pointer to the head of the linked list of declaration nodes.
*   error_str - A pointer to a buffer where an error message will be stored.
*
* Returns:
*   A pointer to the newly created declaration node if successful, or NULL if memory allocation fails.
*/
declarationNode* readNewLabel(char* line, unsigned int line_num, macroNode* macros_list, short dc, short ic, declarationNode* curr_declaration, declarationNode** declarations_list, char* error_str)
{
	char instruction_type; /* Stores the type of instruction identified in the line. */
	char label_name[MAX_LENGTH_OF_LINE]; /* Buffer for storing the label name parsed from the line. */
	declarationNode *new_declaration; /* Pointer to the newly created declaration node. */
	
	/* Validates and extracts the label name from the line. */
	if(!getLabelName(line, label_name, error_str, macros_list) && curr_declaration)
		return curr_declaration;
	
	/* Determines the type of instruction in the line. */
	instruction_type = getInstructionType(line, error_str);
	if((instruction_type == NO_TYPE) && curr_declaration)
		return curr_declaration;
	
	/* Checks if the label name conflicts with a name that has already been defined or declared as extern. */
	if(!checkIfLabelNameAlreadyDeclared(label_name, instruction_type, *declarations_list, error_str) && curr_declaration)
		return curr_declaration;
	
	/* Creates a new declaration node based on the instruction type: */
	if(instruction_type == CODE)
		new_declaration = creatDeclarationNode(ic, label_name, instruction_type, curr_declaration, error_str);
	
	else if(instruction_type >= ENTRY){
		/* Handles extern and entry declarations if a label precedes them is found. */
		if(instruction_type == ENTRY)
			printf("--- Warning: A label name was detected before an entry declaration in line number: %d ---\n", line_num);
		else
			printf("--- Warning: A label name was detected before an extern declaration in line number: %d ---\n", line_num);
		
		new_declaration = readNewDeclaration(line, line_num, macros_list, curr_declaration, declarations_list, error_str);
	}
	else
		new_declaration = creatDeclarationNode(dc, label_name, instruction_type, curr_declaration, error_str);

	skipWhiteChar(line); /* Advance to the start of the instruction or to the end of the line in the case of an entry or extern declaration. */
	
	/* Handles memory allocation failure for the new declaration node. */
	if(!new_declaration)
		return NULL;
	
	/* Updates the head of the declarations list if this is the first node. */
	if(!(*declarations_list))
		*declarations_list = new_declaration;
				
	return new_declaration;
}

/*
* The function: creatDeclarationNode
*
* The function creates a new declaration node with the given parameters and links it to the current declaration node.
*
* Parameters:
*   current_num - The decimal address or the line number to be assigned to the new declaration node.
*   label_name - The name of the label to be assigned to the new declaration node.
*   instruction_type - The type of the instruction.
*   curr_declaration - A pointer to the currently last declaration node in the linked list.
*   error_str - A pointer to a buffer where an error message will be stored if memory allocation fails.
*
* Returns:
*   A pointer to the newly created declaration node if successful, or NULL if memory allocation fails.
*/
declarationNode* creatDeclarationNode(short current_num, char* label_name, char instruction_type, declarationNode* curr_declaration, char* error_str)
{
	declarationNode *new_declaration; /* A pointer for the new declaration node. */
	
	/* Allocate memory for the new declaration node. */
	new_declaration = (declarationNode*)malloc(sizeof(declarationNode));
	if(!new_declaration){
		strcpy(error_str, "new declarationNode");
		return NULL;
	}
	/* Allocate memory for the label name in the new node. */
	new_declaration->label = (char*)malloc(sizeof(char) * (strlen(label_name)+1));
	if(!new_declaration->label){
		strcpy(error_str, "new declarationNode->label");
		free(new_declaration);
		return NULL;
	}
	/* Initialize the fields of the new declaration node. */
	new_declaration->decimal_address = current_num;
	strcpy(new_declaration->label, label_name);
	new_declaration->type = instruction_type;
	new_declaration->next = NULL;
	
	/* Link the new node to the end of the linked list. */
	if(curr_declaration)
		curr_declaration->next = new_declaration;
	
	curr_declaration = new_declaration; /* Updates the curr_declaration node to point to the address of the new_declaration node. */
	
	return new_declaration;
}

/*
* The function: getLabelName
*
* The main function for handling label names, the function extracts the label name from the current line and verifies that the name is correct.
*
* Parameters:
*   line - A pointer to the string containing the line from which the label name will be extracted.
*   label_name - A pointer to a buffer where the extracted label name will be stored.
*   error_str - A pointer to a buffer where an error message will be stored if the label name is invalid.
*   macros_list - A pointer to the head of the linked list of macro nodes used to check if the label name already exists as a macro name.
*
* Returns:
*   TRUE - If the label name is valid.
*   FALSE - If there is an error in the label name.
*/
char getLabelName(char* line, char* label_name, char* error_str, macroNode* macros_list)
{
	char *tail; /* A temporary pointer to hold the position of the colon in the string. */
	short label_len; /* A variable to hold the length of the name of the label. */
	
	tail = strchr(line, ':'); /* Find the position of the colon in the line. */		
	label_len = strlen(line) - strlen(tail); /* Calculate the length of the label name based on the position of the colon. */
	
	/* Check if the label name length exceeds the maximum allowed length. */
	if(label_len >= MAX_LENGTH_OF_LABEL){
		strcpy(error_str, "Invalid label name, label name contains more than 31 characters.");
		label_len = MAX_LENGTH_OF_LABEL -1;
	}
	/* Copy the label name from the line to the label_name buffer. */	
	strncpy(label_name, line, label_len);
	label_name[label_len] = '\0';
	
	/* Validate the label name and check if it is not already used as a name of macro. */
	if(!checkLabelName(label_name, macros_list, error_str))
		return FALSE; 
	
	/* Checks if there is at least one white character after the colon */	
	if(!isspace(line[label_len+1])){
		strcpy(error_str, "Invalid label definition, no space was detected after the colon.");
		return FALSE;	
	}	
	skipWord(line);/* Skip the label name and the colon in the line. */
	
	/* Check if there is any instruction following the label. */
	if(isWhiteSpaceString(line)){
		sprintf(error_str, "Invalid label definition, no instruction was detected after the label: %s", label_name);
		return FALSE;
	}
						
	return TRUE;
}

/*
* The function: checkIfLabelNameAlreadyDeclared
*
* The function checks if the given label name conflicts with a previously defined or declared label name according to the instruction type.
*
* Parameters:
*   label_name - A pointer to the string containing the label name to be checked.
*   instruction_type - The instruction type of the given label.
*   declarations_list - A pointer to the head of the linked list of declaration nodes.
*   error_str - A pointer to a buffer where an error message will be stored if a conflict is found.
*
* Returns:
*   TRUE - If the label name does not conflict with any other label name.
*   FALSE - If there is a conflict with a previously defined or declared label name.
*/
char checkIfLabelNameAlreadyDeclared(char* label_name, short instruction_type, declarationNode* declarations_list, char* error_str)
{
	declarationNode *temp = declarations_list; /* A temporary pointer to traverse all nodes of the linked list */
	
	/* Iterate through the list of declaration nodes. */
	while(temp){
		/* Check if the current node's label matches the given label name. */
		if(!strcmp(label_name, temp->label)){
			/* Handle conflicts based on the instruction type of the given label and the instruction type of the existing label. */
			if((instruction_type == EXTERN) && (temp->type != EXTERN)){
				if(temp->type == ENTRY)
					sprintf(error_str, "Invalid label, the label: %s has already been declared as an entry.", label_name);
				else
					sprintf(error_str, "Invalid label, the label: %s has already been defined in the current file.", label_name);

				return FALSE;
			}			
			if((instruction_type == ENTRY) && (temp->type == EXTERN)){
				sprintf(error_str, "Invalid label, the label: %s has already been declared as extern.", label_name);
				return FALSE;
			}
			if((instruction_type != EXTERN) && (instruction_type != ENTRY) && (temp->type != ENTRY)){
				if(temp->type == EXTERN)
					sprintf(error_str, "Invalid label, the label: %s has already been declared as extern.", label_name);
				else
					sprintf(error_str, "Invalid label, the label: %s has already been defined in the current file.", label_name);
				return FALSE;
			}
		}
		temp = temp->next; /* Move to the next node in the list. */		
	}
	return TRUE;	
}

/*
* The function: checkLabelName
*
* The function validates that the label name meets the criteria for label names,
* and ensuring that there is no conflict with any macro name or system keyword.
* 
* Parameters:
*   label_name - A pointer to the string containing the label name to be checked.
*   macros_list - A pointer to the head of the linked list of macro nodes.
*   error_str - A pointer to a buffer where an error message will be stored if the label name is invalid.
*
* Returns:
*   TRUE - If the label name is valid.
*   FALSE - If the label name is invalid.
*/
char checkLabelName(char* label_name, macroNode* macros_list, char* error_str)
{
	short i = 0; /* An index to track the current position in the label name. */
	char tmp_error_str[ERROR_MESSAGE_SIZE]; /* A buffer to store temporary error messages. */
	
	/* Checks that the label name does not conflict with any system keyword. */
	if(!isValidName(label_name, tmp_error_str)){
		strcpy(error_str, "Invalid label name, ");
		strcat(error_str, tmp_error_str);
		return FALSE;
	}
	/* Checks that the label name does not conflict with any macro name. */
	if(isMacroNameExists(label_name, macros_list)){
		sprintf(error_str, "Invalid label name, the name: %s is already used as a macro name.", label_name);
		return FALSE;
	}
	/* Check if the label name starts with an alphabetic character. */
	if(!isalpha(label_name[i++])){
		sprintf(error_str, "Invalid label name, the label name: %s starts with an invalid character.", label_name);
		return FALSE;
	}
	/* Check if the label name contains only alphanumeric characters. */
	while(label_name[i]){
		if(!isalnum(label_name[i++])){
			sprintf(error_str, "Invalid label name, the label name: %s contains an invalid character.", label_name);
			return FALSE;
		}
	}
	
	return TRUE;
}

/*
* The function: getInstructionType
*
* The function identifies the type of instruction from a given line by extracting the instruction name and 
* checking if it corresponds to a recognized instruction or opcode.
* 
* Parameters:
*   line - A pointer to the string that holds the current line being analyzed.
*   error_str - A pointer to a string where an error message will be stored if the instruction is not recognized.
*
* Returns:
*   The type of the instruction as a char value, or NO_TYPE if the instruction is not recognized.
*/
char getInstructionType(char* line, char* error_str)
{
	short i = 0; /* An index to track the current position in the line. */
	char instruction_type, instruction_name[MAX_LENGTH_OF_LINE]; /* Variables to store the instruction type and name. */
	
	skipWhiteChar(line);/* Skips to the first non-white character. */
	
	/* Extract the instruction name from the line. */
	while(!isspace(line[i]) && line[i] != '\n' && line[i] ){
		instruction_name[i] = line[i];
		i++;	
	}
	instruction_name[i] = '\0';	
			
	/* Determine if the instruction name matches a known instruction and what type of instruction it is. */
	instruction_type = isInstruction(instruction_name); 
	if(isOpcode(instruction_name) != NO_OPCODE)
		instruction_type = CODE;
	
	/* If the instruction is not recognized, then sets an error message. */
	if(instruction_type == NO_TYPE)
		sprintf(error_str, "Invalid instruction, the word: %s is not recognized by the system.", instruction_name);
	
	return instruction_type;
}

/*
* The function: freeDeclarationsList
*
* The function responsible for freeing the memory allocated for a linked list of declarationNode structures, 
* including the memory allocated for each label in the nodes.
* 
* Parameters:
*   declarations_list - A pointer to the head of the linked list of declarationNode structures to be freed.
*/
void freeDeclarationsList(declarationNode* declarations_list)
{
	declarationNode* temp; /* A temporary pointer to hold the next node in the list. */
			
	/* Iterating through the linked list until all nodes are freed. */
	while(declarations_list){				
		free(declarations_list->label);	/* Free the memory allocated for the label. */			
		temp = declarations_list->next; /* Store the next node in the list. */
		free(declarations_list); /* Free the current node. */
		declarations_list = temp; /* Move to the next node in the list. */
	}
}

/********************************************************************* - END OF FIRST PASS - *********************************************************************/

