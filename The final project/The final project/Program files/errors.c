#include "headers.h"

/*
* FILE: errors.c
*
* This file include functions that alert on errors that are common along all the project.
*
*/


/*
* The function: memoryError
*
* This function alerts on the position where there is a memory error. 
* 
* Parameters:
*	var_name - the name of the variable that we could'nt found space in memory for him.
*	input_file_name - the name of the input file that we received from the user.
* 
*/
void memoryError(char* var_name, char* input_file_name)
{	
	printf("--- Fatal Error: Allocating memory for %s, in file %s failed ---\n", var_name, input_file_name);
	printf("--- The program will stop without reaching its end. ---\n");
}

/*
* The function: newFileOpenError
*
* This function alerts on the position where there is a new file openning error. 
* 
* Parameters:
*	new_file_name - the name of the file that we could'nt open.
*	input_file_name - the name of the input file that we received from the user. 
* 
*/
void newFileOpenError(char* new_file_name, char* input_file_name)
{
	printf("--- Fatal Error: opening file  %s, for the file name: %s failed ---\n", new_file_name, input_file_name);
	printf("--- The program will stop without reaching its end. ---\n");
}

/*
* The function: generalError
*
* This function alerts on the position where there is a general error. 
* 
* Parameters:
*	line - the line of the error position.
*	input_file_name - the name of the input file that we received from the user. 
* 
*/
void generalError(char* input_file_name, short line_num, char* error_str)
{
	printf("--- Error: in file %s, on line %d ---\n", input_file_name, line_num);
	printf("\t--- %s ---\n\n", error_str);
	
}


/************************************************************************ - END OF ERRORS - ************************************************************************/

