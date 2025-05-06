#include "headers.h"

/*
* The function: macrosLayout
*
* The main function for managing the processes of finding macros declarations and printing the macros commands to the output file in the appropriate place.
* 
* Parameters:
*	input_file - a pointer to a FILE that is open for reading.
*	input_file_name - The name of the file that is open for reading with the suffix.
*	output_file_name - The name of the file that will be opened for writing the content of the macros.
*	file_name - The name of the file as received from the user.
*	macros_list - The head of the linked list that will hold the contents of the macros.
*
* Returns:
*	TRUE - if the process finished and no errors were detected.
*	FALSE - if an error is detected.
*	FATAL_ERROR - If a memory error occurred.  
*/
char macrosLayout(FILE* input_file, char* input_file_name, char* output_file_name, char* file_name, macroNode** macros_list)
{			
	FILE *output_file; /* A pointer to a the file that will be opened for writing. */
	char line[MAX_LENGTH_OF_LINE]; /* An array to receive a line from the input file. */
	macroNode *new_macro = NULL; /* A pointer that will hold the last macro found each time. */
	char error_str[150] = { '\0' }; /* An array that stores the cause of the error that was found. */
	unsigned int line_num = 1; /* A variable to count the line number in the input file. */
									
	/* Scaning the file line by line until eof reached. */
	while(fgets(line, MAX_LENGTH_OF_LINE, input_file)){
		if(isValidLine(line, error_str)){ /* Checking that the line is not too long and that no additional characters appear before the sign: ';'. */																	
			if(isStartMacroDeclaration(line, error_str) == TRUE){ /* Detecting the existence of starting a macro definition. */
				new_macro = readNewMacro(line, &line_num, input_file, new_macro, macros_list, error_str); /* Creating a new node for the macro. */
				if(!new_macro){ /* Checking that all memory allocations were successful. */
					memoryError(error_str, input_file_name);
					return FATAL_ERROR; 				
				}
			}
		}
		if(error_str[0]){ /* If an error is found, the reason for the error is printed appropriately. */
			generalError(input_file_name, line_num, error_str);
			return FALSE;
		}															
		line_num++; /* Increase the line counter by oneIncrease the row counter by one. */
	}
	
	rewind(input_file);		
	if(!(output_file = fopen(output_file_name, "w"))){ /* Opening a new file to write the contents of the macros. */
		newFileOpenError(output_file_name, file_name); /* Checking that the file was opened successfully. */
		return FATAL_ERROR;
	}
	/* A second scan of the input file and searching for mentions of macro names. */
	while(fgets(line, MAX_LENGTH_OF_LINE, input_file)){
		if(line[0] != ';' && !isWhiteSpaceString(line)){ /* Skipping comment lines and empty lines. */
			skipWhiteChar(line); /* Skip white characters at the beginning of the line. */
			/* Skipping macro definition lines. */
			if(isStartMacroDeclaration(line, error_str)){ 
				while(!isEndMacroDeclaration(line, error_str))
					fgets(line, MAX_LENGTH_OF_LINE, input_file);							
				continue;
			}
			printLineToFile(output_file, line, *macros_list); /* Detects if there is a mention of a macro name and prints the appropriate lines. */
		}
	}

	fclose(output_file); /* closing the output file. */		
	return TRUE;
}

/*
* The function: readNewMacro
*
* The main function for managing the processes of identifying the data of the macro, checking their correctness, 
* creating a new macroNode and concatenation of the new node into the linked list.
* 
* Parameters:
*	line - a pointer to a string that holds the current line read from the input file.
*	line_num - The line number counter.
*	input_file - a pointer to a FILE that is opened for reading.
*	curr_macro - The pointer to the current last node of the linked list of macros.
*	macros_list - a pointer to the head of the linked list.
*	error_str - A pointer to a string, so that if an error is found, it will store the reason for the error.
*
* Returns:
*	A pointer to the new macroNode, or NULL if an error occurs during the process.  
*/
macroNode* readNewMacro(char* line, unsigned int* line_num, FILE* input_file, macroNode* curr_macro, macroNode** macros_list, char* error_str)
{	
	macroNode *new_macro = NULL; /* A pointer to the new node where the data will be inserted. */
	char indicator;
	
	new_macro = creatMacroNode(error_str); /* Allocate memory to the new node. */
	if(!new_macro) /* Check that the memory allocation was successful. */				
		return NULL;
	/* Identifying the data from the input file, and inserting it into the new macro node. */
	indicator = getMacroData(line, line_num, input_file, new_macro, *macros_list, error_str);
	if(indicator == FATAL_ERROR)		
		return NULL;
	
	if(!indicator && curr_macro)
		return curr_macro;
	
	if(!(*macros_list)) /* Setting the head of the linked list if not already set. */
		*macros_list = new_macro;
		
	if(curr_macro)
		curr_macro->next = new_macro; /* Appending the new node to the end of the linked list. */

	curr_macro = new_macro; /* Update of the last node in the linked list. */
	
	return new_macro;
}

/*
* The function: creatMacroNode
*
* The function Allocates and initializes a new macroNode structure..
* 
* Parameters:
*	error_str - A pointer to a string to store the name of the variable for which the memory allocation failed.
*
* Returns:
*	A pointer to the new macroNode, or NULL if a memory allocation failure occurred.  
*/
macroNode* creatMacroNode(char* error_str)
{	
	macroNode *new_macro; /* A pointer for the new macro node. */
	
	/* Allocate memory for the macroNode structure. */
	new_macro = (macroNode*)malloc(sizeof(macroNode));
	if(!new_macro){ /* Check that the memory allocation was successful. */
		strcpy(error_str, "new macroNode");
		return NULL;
	}
	/* Allocate memory for the macro name. */		
	new_macro->name = (char*)malloc(sizeof(char) * MAX_LENGTH_OF_LINE);
	if(!new_macro->name){
		free(new_macro); 
		strcpy(error_str, "new macroNode->name");
		return NULL;
	}
	/* Allocate memory for the commands array. */		
	new_macro->commands = (char**)malloc(sizeof(char*) * BUFFER);
	if(!new_macro->commands){
		free(new_macro);
		free(new_macro->name);
		strcpy(error_str, "new macroNode->commands");
		return NULL;
	}			
	new_macro->next = NULL; /* Initialize the next node to NULL. */
	
	return new_macro;	
}

/*
* The function: getMacroData
*
* The main function that manages the process of identifying and receiving the data from the input file and checking the data.
* 
* Parameters:
*   line - A pointer to the current line from the input file.
*   line_num - The line number counter.
*   input_file - A pointer to the file being read.
*   new_macro - A pointer to the macroNode where the data will be stored.
*   macros_list - A pointer to the head of the linked list of macros.
*   error_str - A pointer to a string to store error messages if validation fails.
*
* Returns:
*	TRUE if the data was received properly.
*   FALSE if an error was found in one of the data.
*	FATAL_ERROR if a memory allocation failure occurred. 
*/
char getMacroData(char* line, unsigned int* line_num, FILE* input_file, macroNode* new_macro, macroNode* macros_list, char* error_str)
{	
	char tmp_error_str[ERROR_MESSAGE_SIZE];
	
	/* Skip the word 'macr' and extract the macro name. */
	skipWord(line);	
	if(!getMacroName(line, new_macro->name, error_str))
		return FALSE; 
	/* Checks that the macro name is not a keyword. */	
	if(!isValidName(new_macro->name, tmp_error_str)){
		strcpy(error_str, "Invalid macro name, ");
		strcat(error_str, tmp_error_str);
		return FALSE;
	}
	/* Checks that the macro name has not already been previously defined in the current file. */
	if(isMacroNameExists(new_macro->name, macros_list)){
		strcpy(error_str, "The macro name has already been defined in the current file.");
		return FALSE;
	}
	
	fgets(line, MAX_LENGTH_OF_LINE, input_file); /* Advance the input file pointer to the next line. */
	(*line_num)++;
	if(!isValidLine(line, error_str))
		return FALSE;	
	
	/*Extracts the commands in the macro content, and store them in the new macro node. */
	return getCommands(line, input_file, line_num, new_macro, error_str);
}

/*
* The function: getMacroName
*
* The function extracts the name of the macro from the current line, and verifies that the definition line is correct.
* 
* Parameters:
*	line - A pointer to the current line from the input file.
*   macro_name - A pointer to a string to store the extracted macro name.
*   error_str - A pointer to a string to store error messages if validation fails.
*
* Returns:
*	TRUE if the macro name is successfully extracted and validated.
*   FALSE if the name is invalid or too long.  
*/
char getMacroName(char* line, char* macro_name, char* error_str)
{	
	char temp_name[MAX_LENGTH_OF_LINE];
	
	/* Extract the macro name from the line. */
	getWord(line, temp_name);
	
	/* Ensure the macro name is not too long. */
	if(strlen(temp_name) >= MAX_LENGTH_OF_LABEL){
		strcpy(error_str, "A too long macro name was detected.");
		return FALSE;
	}
	strcpy(macro_name, temp_name);
			
	/* Validate that no additional characters follow the macro name. */
	if(!isWhiteSpaceString(line)){	
		strcpy(error_str, "Characters were detected after the macro name");
		return FALSE;
	}
			
	return TRUE;
}

/*
* The function: getCommands
*
* The function extracts the command lines from the input file, stores them in the new macro node,
* and verifies the correctness of the end line of the macro definition.
* 
* Parameters:
*	line - A pointer to the current line from the input file.
*   input_file - A pointer to the file being read.
*   line_num - The line number counter.
*   new_macro - A pointer to the macroNode where the commands will be stored.
*   error_str - A pointer to a string to store error messages if an error occurs.
*
* Returns:
*   TRUE if the commands were extracted successfully.
*   FALSE if a validation error occurs.
*   FATAL_ERROR if a memory allocation failure occurred.  
*/
char getCommands(char* line, FILE* input_file, unsigned int* line_num, macroNode* new_macro, char* error_str)
{	
	int i = 0, buffer = BUFFER;
	char **temp_commands;
	
	/* Loop to read and store commands until the end of the macro definition is detected. */
	while(!isEndMacroDeclaration(line, error_str)){
		/* Reallocate memory if necessary. */
		if(i >= buffer){
			buffer += BUFFER;
			temp_commands = (char**)realloc(new_macro->commands, sizeof(char*) * buffer);
			if(!temp_commands){
				strcpy(error_str, "reallocate macroNode->commands");
				freeMacrosList(new_macro);
				return FATAL_ERROR;			
			}
			new_macro->commands = temp_commands;
		}
		/* Allocate memory for the current command. */
		new_macro->commands[i] = (char*)malloc(sizeof(char) * MAX_LENGTH_OF_LINE);
		if(!(new_macro->commands[i])){
			sprintf(error_str, "new macroNode->commands[%d]", i);
			freeMacrosList(new_macro);
			return FATAL_ERROR;			
		}					
		strcpy(new_macro->commands[i++], line); /* Storing the current command into the commands array of the new macro node. */
		
		/* Read the next line from the input file and validate it. */
		fgets(line, MAX_LENGTH_OF_LINE, input_file); 
		(*line_num)++;
		if(!isValidLine(line, error_str))
			return FALSE;		
	}
	/* Checking that the end line of the macro definition is correct. */
	if(isEndMacroDeclaration(line, error_str) < 0)
		return FALSE;
	
	/* Adjust memory allocation to fit the actual number of commands and one more cell to mark the end of the array. */		
	temp_commands = (char**)realloc(new_macro->commands, sizeof(char*) * (i + 1));
	if(!temp_commands){
		strcpy(error_str, "reallocate macroNode->commands");
		freeMacrosList(new_macro);
		return FATAL_ERROR;	
	}
	new_macro->commands = temp_commands;			
	new_macro->commands[i] = NULL;
	
	return TRUE;		
}

/*
* The function: isStartMacroDeclaration
*
* The function checks if the received line contains the macro declaration word "macr" 
* and if the declaration word has found then checks the correctness of the line.
* 
* Parameters:
*	line - A pointer to the current line from the input file..
*	error_str - A pointer to a string to store error messages if an error occurs.
*
* Returns:
*	FALSE if no macro declaration is found.
*	-1 if a declaration is found but an error was found.
*	TRUE if a valid declaration was found. 
*/
char isStartMacroDeclaration(char* line, char* error_str)
{	
	char *temp;
	
	temp = strstr(line, "macr ");/* Search for the macro declaration keyword "macr ". */
	if(temp){
		/* Check if the keyword "macr" is at the beginning of the line or there are additional characters before the keyword. */
		if(line != temp){
			strcpy(error_str, "Extra characters were detected before the start of the macro declaration line.");
			return -1;
		}
		return TRUE;
	}
	return FALSE;
}

/*
* The function: isEndMacroDeclaration
*
* The function checks if the received line contains the end of macro declaration keyword "endmacr" 
* and if the end of declaration keyword has found then checks the correctness of the line.
* 
* Parameters:
*	line - A pointer to the current line from the input file.
*   error_str - A pointer to a string to store error messages if extra characters are detected.
*
* Returns:
*	TRUE if the macro end declaration is valid and correctly formatted.
*   FALSE if the macro end declaration is not found.
*   -1 if extra characters are detected before or after the declaration.
*/
char isEndMacroDeclaration(char* line, char* error_str)
{
	char* temp = NULL;
	
	/* Search for the macro end declaration keyword "endmacr". */	
	temp = strstr(line, "endmacr");	
	if(temp){
		/* Check if the keyword "endmacr" is at the beginning of the line or there are additional characters before the keyword. */
		if(line != temp){
			strcpy(error_str, "Extra characters were detected before the end of the macro declaration line.");
			return -1;
		}
		/* Skip the keyword and check if there are additional characters after the keyword. */				
		skipWord(line);
		if(!isWhiteSpaceString(line)){
			strcpy(error_str, "Extra characters were detected after the end of the macro declaration line.");
			return -1;
		}
		return TRUE;
	}
	return FALSE;	
}

/* #####NEED TO UPDATE THIS FUNCTION API###### */
/*
* The function: isValidLine
*
* The function checks if the line is correct and advances the line pointer to the first non-white character. 
* 
* Parameters:
*	line - A pointer to the current line from the input file.
*   error_str - A pointer to a string to store error messages if an error detected.
*
* Returns:
*	TRUE if no errors were found.
*	FALSE if an error was found. 
*/
char isValidLine(char* line, char* error_str)
{
	char *temp;
	
	/* Checks if the line length is correct. */	
	if((strlen(line) == MAX_LENGTH_OF_LINE-1) && line[MAX_LENGTH_OF_LINE-2] != '\n'){
		printf("strlen(line) = %d\n", strlen(line));
		strcpy(error_str, "The line contains more than 80 characters.");
		return FALSE;
	}
	
	temp = strchr(line, ';'); /* Search for the comment mark character. */
	
	/* Checks that if a comment line character is found then there are no additional characters before it. */
	if(temp && temp != line){
		strcpy(error_str, "Extra characters were detected before the character: ';'");		
		return FALSE;
	}
	skipWhiteChar(line); /* Skips to the first non-white character. */
	
	return TRUE;
}

/*
* The function: isMacroNameExists
*
* The function searches the linked list of macros for a macro with the given variable name.
* 
* Parameters:
*	variable_name - A pointer to the string containing a name of variable to search for.
*   macros_list - A pointer to the head of the linked list of macros.
*
* Returns:
*	A pointer to the macroNode if the macro name exists in the list.
*   NULL if the name is not found. 
*/
macroNode* isMacroNameExists(char* variable_name, macroNode* macros_list)
{
	macroNode* temp_node = macros_list;
	
	/* Iterate through the linked list to find the macro name. */
	while(temp_node){
		/* Compare the current node's name with the given variable name. */
		if(!strcmp(variable_name, temp_node->name))
			return temp_node;
		/* Move to the next node in the list. */			
		temp_node = temp_node->next;
	}	
	return NULL;
}

/*
* The function: printLineToFile
*
* The function manages the printing process to the output file, 
* If the line starts with a macro name it prints the corresponding macro's commands instead of the line.
* 
* Parameters:
*	output_file - A pointer to the FILE where the line or macro commands will be written.
*   line - A pointer to the string containing the line to be printed.
*   macros_list - A pointer to the head of the linked list of macros.	  
*/
void printLineToFile(FILE* output_file, char* line, macroNode* macros_list)
{
	macroNode *temp_node;
	char macro_name[MAX_LENGTH_OF_LINE];
	short i = 0;
	
	/* Extract the first word from the line, assuming it could be a macro name. */
	while(!isspace(line[i]) && line[i] != '\n' && line[i]){
		macro_name[i] = line[i];
		i++;	
	}
	macro_name[i] = '\0';
	
	/* Check if the word is a macro name and print accordingly. */
	temp_node = isMacroNameExists(macro_name, macros_list);					
	if(!temp_node)
		fputs(line, output_file);				
	else
		printMacroCommands(temp_node, output_file);	
}

/*
* The function: printMacroCommands
*
* The function writes the commands of the specified macro to the output file.
* 
* Parameters:
*	macro - A pointer to the macroNode containing the commands to be printed.
*   output_file - A pointer to the FILE where the commands will be written.	  
*/
void printMacroCommands(macroNode* macro, FILE* output_file)
{	
	int i = 0;
	
	/* Prints the commands of the macro to the output file. */
	while(macro->commands[i]){
		fputs(macro->commands[i], output_file);
		i++;
	}		
}

/*
* The function: freeMacrosList
*
* The function Frees all memory allocated for a linked list of macroNode structures, including each macro's name and commands.
* 
* Parameters:
*	macros_list - A pointer to the head of the linked list of macros to be freed.  
*/
void freeMacrosList(macroNode* macros_list)
{
	int i;
	macroNode* temp;
	
	/* Iterate through the linked list, freeing memory for each macroNode. */			
	while(macros_list){
		i = 0;				
		free(macros_list->name);/* Free the memory allocated for the macro's name. */
		
		/* Free the memory allocated for each command in the macro. */		
		while(macros_list->commands[i])
			free(macros_list->commands[i++]);
			
		free(macros_list->commands);/* Free the memory allocated for the commands array. */		
		temp = macros_list->next; /* Move to the next node in the list. */
		free(macros_list); /* Free the current macroNode. */
		macros_list = temp; /* Update the head of the list. */
	}
}

/******************************End of pre_processor**********************************************/

