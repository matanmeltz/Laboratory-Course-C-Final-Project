#include "headers.h"

/*
* FILE: assembler.c
*
* This file runs the entire project , and execute all the project files together, also known as the 'assembler'.
* The assembler contains 4 stages: 
*	1. receive the input files from the user.
*	2. pre_processor - create .am type file, that if there are macros in the input file, it spread them and include tham in the .am file
*	3. first pass - translate all the words into binary machine code, except from the labels.
*	4. second pass - translate the labels into binary machine code, and create output files(ob,ent,ext).
*
*/

int main(int argc, char *argv[])
{
	FILE *input_file;/*Pointer to the input file*/
	short i = 1;/*The current input file number*/
	short ic_dc_counter[2]; /*The first cell represents the number of instructions to execute in the file,
				 *the second cell represents the number of data instructions in the file*/
	char indicator;/*Represnt if there is errors ( -1 - fatal error
							0 - error
							1 - no error )*/
	char *input_file_name;/*Represent file with the .as sufix*/
	char *output_file_name;/*Represent file with the .am sufix*/
	macroNode *macros_list;/*List of macro nodes*/
	instructionNode *instructions_list;/*List of instructions nodes*/
	dataNode *data_list;/*List of data nodes*/
	declarationNode *declarations_list;/*List of declarations nodes*/
	
/************************************************************** - STAGE 1: RECEIVING THE FILE NAME - **************************************************************/

	if(argc == 1){/*If there are no input files*/
		printf("--- No files names was received. ---\n--- End of assembler. ---\n");
		return 0;
	}
			
	while(--argc > 0){/*If there are still input files left to check*/	
		/*Reset the variables:*/
		input_file_name = NULL;
		output_file_name = NULL;
		macros_list = NULL;
		instructions_list = NULL;		
		data_list = NULL;	
		declarations_list = NULL;
		
		printf("--- Start reading file: %s ---\n",argv[i]);
		/*Allocate memory for the input file name*/
		input_file_name = (char*)malloc(sizeof(char) * (strlen(argv[i]+INPUT_FILE_SIZE)) );
		if(!input_file_name){/*If there was an error in allocation*/
			memoryError(input_file_name, argv[i]);
			exit(EXIT_FAILURE);
		}
		/*Allocate memory for the output file name*/
		output_file_name = (char*)malloc(sizeof(char) * (strlen(argv[i]+OUTPUT_FILE_SIZE)) );
		if(!output_file_name){/*If there was an error in allocation*/
			memoryError(output_file_name, argv[i]);
			free(input_file_name);
			exit(EXIT_FAILURE);
		}
		
		strcpy(input_file_name, argv[i]);
		strcat(input_file_name, ".as");
		/*Opening new input(.as) file*/
		if(!(input_file = fopen(input_file_name, "r"))){/*If there was an error opening the input file*/
			printf("--- Error: file %s does not exist. ---\n", input_file_name);
			free(input_file_name);
			free(output_file_name);
			checkIfRemainMoreFile(argc);
			i++;
			continue;
		}
					
		strcpy(output_file_name, argv[i]);
		strcat(output_file_name, ".am");		
		
/******************************************************************* - STAGE 2: PRE-PROCESSOR - *******************************************************************/
		
		if(!(indicator = macrosLayout(input_file, input_file_name, output_file_name, argv[i], &macros_list))){/*If there where errors in pre processor stage*/
			printf("--- The program didn't create .am file for: %s ---\n", argv[i]);
			checkIfRemainMoreFile(argc);
			free(output_file_name);
			mainGeneralFree(input_file, input_file_name, macros_list, declarations_list, data_list, instructions_list);
			i++;
			continue;	
		}
		if(indicator == FATAL_ERROR){/*If there was memory error in pre processor stage*/
			printf("--- Memory error, the program will stop without reaching its end. ---\n");
			free(output_file_name);
			mainGeneralFree(input_file, input_file_name, macros_list, declarations_list, data_list, instructions_list);
			exit(EXIT_FAILURE);/*Stops the program because memory errors*/
		}
		
		fclose(input_file);
		free(input_file_name);
		
/******************************************************************** - STAGE 3: FIRST PASS - ********************************************************************/
		
		/*Opening new output file*/
		input_file_name = output_file_name;		
		if(!(input_file = fopen(input_file_name, "r"))){/*If there was an error opening the output file*/		
			newFileOpenError("input_file", argv[i]);
			mainGeneralFree(input_file, input_file_name, macros_list, declarations_list, data_list, instructions_list);
			exit(EXIT_FAILURE);/*Stops the program because memory errors*/
		}
									
		indicator = firstPass(input_file, input_file_name, ic_dc_counter, macros_list, &instructions_list, &data_list, &declarations_list);/*Set the values according if there was errors in first pass*/
	
		if(indicator == FATAL_ERROR){/*If there was memory error in first pass stage*/
			printf("--- Memory error, the program will stop without reaching its end. ---\n");
			mainGeneralFree(input_file, input_file_name, macros_list, declarations_list, data_list, instructions_list);
			exit(EXIT_FAILURE);/*Stops the program because memory errors*/
		}
		
/******************************************************************** - STAGE 4: SECOND PASS - ********************************************************************/

		if(!(indicator = secondPass(argv[i], input_file_name, ic_dc_counter, &instructions_list, data_list, declarations_list, indicator)))/*If there were errors in second pass stage*/
			printf("--- The program will not create an object file for: %s ---\n", argv[i]);
		
		mainGeneralFree(input_file, input_file_name, macros_list, declarations_list, data_list, instructions_list);
		
		if(indicator == FATAL_ERROR)/*If there was memory error in second pass stage*/
		{
			printf("--- Memory error, the program will stop without reaching its end. ---\n");
			exit(EXIT_FAILURE);/*Stops the program because memory errors*/
		}
		checkIfRemainMoreFile(argc);
		i++;
	}
		
	printf("--- End of assembler, finished reading all the files. ---\n");
	return 0;
}

/************************************************************************* - END OF MAIN - *************************************************************************/

