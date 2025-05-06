#include "headers.h"

/*
* The function: secondPass
*
* The main function for managing second pass.
* This function is filling the missing bmc value where labels were seen in first pass,
* and create the output files from type: object, entry, external.
* 
* Parameters:
*	files_name - the name of the output files without the suffix.
*	error_file_name - the name of the input file with the suffix .am.
*	ic_dc_counter - contain the amount of ic(cell 0) and dc(cell 1) from first pass.
*	instructions_list - a list of instructionNode.	 
*	data_list - a pointer to list of dataNode.	
*	declarations_list - a pointer to list of declarationNode.
*	indicator - present if there was an error(value 0) or no-error(value 1) in first pass.
*
* Returns:
* 	-1 (FATAL_ERROR) - if there was fatal error(memory/files opening).
*	0 (FALSE) - if there was a syntax error.
*	1 (TRUE) - if successfully completed second pass.
*  
*/
char secondPass(char* files_name, char* error_file_name, short* ic_dc_counter, instructionNode** instructions_list, dataNode* data_list, declarationNode* declarations_list, char indicator)
{
	short ent_is_length[2] = {FALSE,0};/*If there is at least 1 entry(cell 0).The length of longest entry(cell 1)*/
	short ext_is_length[2] = {FALSE,0};/*If there is at least 1 extern(cell 0).The length of longest extern(cell 1)*/
	
	if(!isEntriesDefined(error_file_name, declarations_list))/*If the function return FALSE(which mean that there is an error) */
		indicator = FALSE;

    	if(!fillingMissingBMCofLabel(error_file_name, instructions_list, declarations_list, ent_is_length, ext_is_length))/*If the function return FALSE(which mean that there is an error) */
    		indicator = FALSE;
    		
	if(indicator)/*If there were no errors in second pass or in first pass*/
	{	
		if( FATAL_ERROR == createObAndExtFiles(files_name, ic_dc_counter, instructions_list, data_list, ext_is_length) )/*If there is memory error in the function*/
		 	return FATAL_ERROR;
	 	
		if(ent_is_length[0] && FATAL_ERROR == createEntFile(files_name, declarations_list, ent_is_length))/*If there is an entry label in the input file and there is memory error in the function*/
			return FATAL_ERROR;
	}
	return indicator;
}

/*   
* The function: fillingMissingBMCofLabel
*
* The function cheecks for errors in the labels at declarationNode list, and if it's
* proper, it's enter the binary value of the label, except of the binary value of extern label.
* 
* Parameters:
*	file_name - the name of the input file with the suffix .am.
*	instructions_list - a list of instructionNode.
*	declarations_list - a pointer to list of declarationNode.
*	ent_is_length - cell 0 indicate if there is at least 1 entry, cell 1 present the length of longest entry.
*	ext_is_length - cell 0 indicate if there is at least 1 extern, cell 1 present the length of longest extern.
*
* Returns:
*	0 (FALSE) - if there was an error and there is an undefined label.
*	1 (TRUE) - if successfully enter all the binary values of the labels in their place.
*/
char fillingMissingBMCofLabel(char* file_name, instructionNode** instructions_list, declarationNode* declarations_list, short* ent_is_length, short* ext_is_length)
{
	char found_label_name;/*Represent if the name of the label was found */
	char curr_declared_entry;/*Represent if the label name was declared as entry for the current node*/
	char curr_declared_extern;/*Represent if the label name was declared as extern for the current node*/
	char curr_declared_dsc;/*Represent if the label name was declared as data/string/code for the current node*/
	char no_error = TRUE;/*Represent if there are errors while going throw the instructions_list nodes*/
	char error_str[ERROR_MESSAGE_SIZE] = { '\0' };/*Include the content of the error, if there is an error*/
	short curr_ins_da; /*Represent the value of the decimal address(da) for the current node */
	instructionNode *curr_ins = *instructions_list;/*The current instruction node */
	declarationNode *curr_dec = declarations_list;/*The current declaration node*/
	
	while(curr_ins)/*If curr_ins not empty*/
	{
		/*Reset the variables:*/
		found_label_name = FALSE;
		curr_declared_entry = FALSE;
		curr_declared_extern = FALSE;
		curr_declared_dsc = FALSE;
		if(curr_ins->line)/*If bmc was not defined,which mean that there is a label to check */
		{
			while( curr_dec && (!curr_declared_extern) && (!(curr_declared_entry && curr_declared_dsc)) ) /*If didnt passed on all the declaration list and not extern or not entered all the 'if' conditions inside*/
			{
				if(!strcmp(curr_ins->type.name,curr_dec->label))/*If as the same label name */
				{
					found_label_name = TRUE;/*There is label with the same name*/	
					if(curr_dec->type == EXTERN)/*If the label is extern*/
					{	
						curr_declared_extern = TRUE;/*The label is extern*/
						curr_ins->line = EXTERN;/*Mark the label as extern*/
						ext_is_length[0] = TRUE;/*Generally there is an extern label*/
						if(ext_is_length[1] < strlen(curr_dec->label) )/*Check longest extern*/
							ext_is_length[1] = strlen(curr_dec->label);/*Save the longest extern*/
					}
					else if( (!curr_declared_entry) && (curr_dec->type == ENTRY) ) /*If not enterd in yet && and the label is entry*/
					{
						curr_declared_entry = TRUE;/*The label is entry*/
						ent_is_length[0] = TRUE;/*Generally there is an entry label*/
						if(ent_is_length[1] < strlen(curr_dec->label) )/*Check longest entry*/
							ent_is_length[1] = strlen(curr_dec->label);/*Save the longest entry*/
					}
					else if(!curr_declared_dsc)/*If not enterd in yet, and it is a definition node */
					{
						curr_declared_dsc = TRUE;/*The label is data/string/code*/
						curr_ins_da = curr_dec->decimal_address;
					}	
				}
				curr_dec = curr_dec->next;/*Pass to the next node*/
			}
			
			if(!found_label_name)/*if the name of the label was not found in the declaration list*/
			{
				no_error = FALSE;/*Error was founded*/
				sprintf(error_str,"The label: %s is illegal, it was not defined or declared as extern.", curr_ins->type.name);
				generalError(file_name, curr_ins->line, error_str);
			}
			if(!curr_declared_extern)/*If the label is not type extern*/
			{
				free(curr_ins->type.name);
				getRelocatableBMC(curr_ins_da,&curr_ins->type.bmc);/*Enter the bmc value of the label*/
				curr_ins->line = HAVE_BMC;/*Update that the BMC of the node has been defined*/
			}
			curr_dec = declarations_list;/*Return curr_dec back to the first node*/
		}
		curr_ins = curr_ins->next;/*Pass to the next node*/
	}
	return no_error;
}

/*   
* The function: isEntriesDefined
*
* The function cheecks that every label which was declared has entry, in addition it was also defined
* has entry.
* 
* Parameters:
*	file_name - the name of the input file with the suffix .am.
*	declarations_list - a pointer to list of declarationNode.
*
* Returns:
*	0 (FALSE) - if there is an entry that was declared but not defined.
*	1 (TRUE) - if every entry was declared and defined.
*/
char isEntriesDefined(char* file_name, declarationNode* declarations_list)
{

	declarationNode *curr_dec = declarations_list;/*Represent the current declaraton node*/
	declarationNode *temp;/*Represent the second declaration node to be compared by the first declaration node*/
	char indicator;/*Represent if found a matching label */
	char error_str[ERROR_MESSAGE_SIZE] = { '\0' };/*Include the content of the error, if there is an error*/
	
	while(curr_dec){/*If curr_dec not empty*/
		if(curr_dec->type == ENTRY){/*If the label is entry*/
			temp = declarations_list;
			indicator = FALSE;/*Reset the variable*/
			while(temp){/*if temp not empty*/
				if(!strcmp(curr_dec->label, temp->label) && (temp->type != ENTRY)){/*If has the same name and is not entry type*/
					 indicator = TRUE;
					 curr_dec->decimal_address = temp->decimal_address;
					 break;/*Found matching label, so can finish the while*/
				}
				temp = temp->next;/*Pass to the next node*/
			}
			if(indicator == FALSE){/*If didnt found matching label*/
				sprintf(error_str, "No definition was found for the entry: %s", curr_dec->label);
				generalError(file_name, curr_dec->decimal_address, error_str);
			}				
		}
		curr_dec = curr_dec->next;/*Pass to the next node*/
	}
	if(error_str[0])/*If there are errors*/
		return FALSE;
	
	return TRUE;
}

/*   
* The function: getRelocatableBMC
*
* The function enter a value of an integer number and an entry label value('R' bit) into the bmc feature of a node.
* 
* Parameters:
*	decimal_value - the integer number value.
*	bmc - a pointer to the node bmc feature.
*/
void getRelocatableBMC(short decimal_value, short* bmc)
{	
	*bmc = HAVE_BMC; /*reset the value of bmc to 0*/
	turnOnBit(bmc, R_ADDRESSING_METHOD);
	insertBinaryValueByData(decimal_value, bmc, ADDRESS_START_POSITION);
}

/********************************************************************* - PRINTING FUNCTIONS - *********************************************************************/

/*    
* The function: createObAndExtFiles
*
* The function creates the object and extern output files, if there are no externs,
* it is only create the object output file.
* 
* Parameters:
*	file_name - the name of the output file without the suffix.
*	ic_dc_counter - contain the amount of ic(cell 0) and dc(cell 1) from first pass.
*	instructions_list - a list of instructionNode.	 
*	data_list - a pointer to list of dataNode.
*	ext_is_length - cell 0 indicate if there is at least 1 extern, cell 1 present the length of longest extern.
*
* Returns:
*	-1(FATAL_ERROR) - if there was fatal error(memory/files opening).
*	1(TRUE) - if there was no error.
*/
char createObAndExtFiles(char* file_name, short* ic_dc_counter, instructionNode** instructions_list, dataNode* data_list, short* ext_is_length)
{
	FILE *ob_file, *ext_file;/*Pointers to the ob and ext files*/
	char* ob_file_name = NULL;/*Represent the ob file name*/
	char* ext_file_name = NULL;/*Represent the external file name*/
	
	instructionNode *curr_ins = *instructions_list;/*Represent the current instruction node */
	dataNode *curr_data = data_list;/*Represent the current data node */
	
	/*Allocate memory for the new ob file name*/
	ob_file_name = (char*)malloc(sizeof(char) * (strlen(file_name)+OB_FILE_SIZE));
	if(!ob_file_name)/*If there was an error in allocation*/
	{
		memoryError(ob_file_name, file_name);
		return FATAL_ERROR;	
	}
	strcpy(ob_file_name, file_name);
	strcat(ob_file_name, ".ob");
	
	
	if(ext_is_length[0])/*If there is extern label in the input file*/
	{
		/*Allocate memory for the new ext file name*/
		ext_file_name = (char*)malloc(sizeof(char) * (strlen(file_name)+EXT_FILE_SIZE));
		if(!ext_file_name)/*If there was an error in allocation*/
		{
			memoryError(ext_file_name, file_name);
			free(ob_file_name);
			return FATAL_ERROR;
		}
		strcpy(ext_file_name, file_name);
		strcat(ext_file_name, ".ext");
	}
	
	/*Create new ob file*/
	if(!(ob_file = fopen(ob_file_name, "w")))/*If there was an error opening the ob file*/
	{
		newFileOpenError("ob_file", file_name);
		free(ob_file_name);
		if(ext_is_length[0])/*If there is extern label in the input file*/
			free(ext_file_name);
		return FATAL_ERROR;
	}
	
	/*Create new ext file*/
	if(ext_is_length[0] && (!(ext_file = fopen(ext_file_name, "w"))) )/*If there is extern label in the input file and there was an error creating the ext file*/
	{
		newFileOpenError("ext_file", file_name);
		free(ob_file_name);
		free(ext_file_name);
		fclose(ob_file);
		return FATAL_ERROR;
	}
	
	/*Print to the output files*/
	printfObIcDcNumbersLine(ob_file, ic_dc_counter);
	while(curr_ins)/* Print instructionNode list */
	{
		if(curr_ins->line)/*If there is a value of an extern label */
			printfExtOutputLine(ext_file, curr_ins, ext_is_length);/*Print to the extern file*/
			
		fprintf(ob_file, "%04d %05o\n", curr_ins->ic,curr_ins->type.bmc);/*Print to the ob file*/
		curr_ins = curr_ins->next;/*Pass to the next node*/	
	}
	while(curr_data)/* Print dataNode list*/
	{
		fprintf(ob_file, "%04d %05o\n", curr_data->dc, curr_data->bmc);/*Print to the ob file*/
		curr_data = curr_data->next;/*Print to the ob file*/
	}

	fclose(ob_file);
	free(ob_file_name);
	if(ext_is_length[0])/*If there is extern label in the input file*/
	{
		fclose(ext_file);
		free(ext_file_name);
	}
	return TRUE;
}

/* 
* The function: createEntFile
*
* The function creates the entry output file.
* 
* Parameters:
*	file_name - the name of the output file without the suffix.
*	declarations_list - a pointer to list of declarationNods.
*	ent_is_length - cell 0 indicate if there is at least 1 entry, cell 1 present the length of longest entry.
*
* Returns:
*	-1(FATAL_ERROR) - if there was fatal error(memory/files opening).
*	1(TRUE) - if there was no error.
*/
char createEntFile(char* file_name, declarationNode* declarations_list, short* ent_is_length)
{
	FILE* ent_file;/*Pointer to the ent file*/
	char* ent_file_name = NULL;/*Represent the ent file name*/
	declarationNode *curr_dec = declarations_list;/*Represent the current declaration node*/
	
	/*Allocate memory for the new ent file name*/
	ent_file_name = (char*)malloc(sizeof(char) * (strlen(file_name)+ENT_FILE_SIZE));
	if(!ent_file_name)/*If there was an error in allocation*/
	{
		memoryError(ent_file_name, file_name);
		return FATAL_ERROR;
	}
	strcpy(ent_file_name, file_name);
	strcat(ent_file_name, ".ent");
	
	/*Create new ent file*/
	if(!(ent_file = fopen(ent_file_name, "w")))/*If there was an error opening the ent file*/
	{
		newFileOpenError("ent_file", file_name);
		free(ent_file_name);
		return FATAL_ERROR;
	}
	
	while(curr_dec)/*If curr_dec not empty*/
	{
		if(curr_dec->type == ENTRY)/*If the label is entry */	
			printfEntOutputLine(ent_file, curr_dec, ent_is_length);/*Print to the ent file*/
		curr_dec = curr_dec->next;/*Pass to the next node*/
	}
	fclose(ent_file);
	free(ent_file_name);
	return TRUE;
}

/*   
* The function: printfObIcDcNumbersLine
*
* The function prints the first line of the object file that includes the numbers of ic and dc lines.
* 
* Parameters:
*	oc_file - the object file to be printed on.
*	ic_dc_counter - contain the numbers of ic(cell 0) and dc(cell 1).
*/
void printfObIcDcNumbersLine(FILE* ob_file,short* ic_dc_counter)
{
	short space_length = 3;/*Represent the max length of space between the ic numbers to start of the line*/
	short i;/*Counter*/
	short ic_digits = ic_dc_counter[0];/*Represent the number of ic lines */
	const short SINGLE_DIGIT_NUMBER = 9;/*Represent the 1 digit number*/
	while(ic_digits > SINGLE_DIGIT_NUMBER)/*If ic_digits is bigger then 1 digit number*/
	{
		ic_digits/=10;/*Make the number smaller by dividing it by 10*/
		space_length--;/*Reduce the space length*/
	}
	for(i=0 ; i<space_length; i++)/*Make space according to the needed space length*/
		fprintf(ob_file, " ");
		
	fprintf(ob_file, "%d %d\n", ic_dc_counter[0], ic_dc_counter[1]);/*Print the ic dc number*/
}

/*   
* The function: printfEntOutputLine
*
* The function prints a line to the entry file that includes the entry name and decimal address.
* 
* Parameters:
*	ent_file - the entry file to be printed on.
*	curr_dec - current declaration node that include the entry label name and decimal address.
*	ent_is_length - contain the length of the longest entry(in cell 1).
*/
void printfEntOutputLine(FILE* ent_file, declarationNode* curr_dec, short* ent_is_length)
{
	short i;/*Counter*/
	fprintf(ent_file, "%s", curr_dec->label);/*Print the label name*/

	for(i=0 ; i < (ent_is_length[1]-strlen(curr_dec->label)+1 ) ; i++ )/*Make space between the labels name to their decimal address according to the needed space length*/
		fprintf(ent_file, " ");
		
	fprintf(ent_file, "%04d\n", curr_dec->decimal_address);/*Print their decimal address*/
}

/*   
* The function: printfExtOutputLine
*
* The function prints a line to the extern file that includes the extern name and decimal address.
* 
* Parameters:
*	ext_file - the extern file to be printed on.
*	curr_ins - current instruction node that include the extern label name and decimal address.
*	ext_is_length - contain the length of the longest extern(in cell 1).
*/
void printfExtOutputLine(FILE* ext_file, instructionNode* curr_ins, short* ext_is_length)
{
	int i;/*Counter*/
	fprintf(ext_file, "%s", curr_ins->type.name);/*Print the label name*/
	
	for(i=0 ; i < (ext_is_length[1]-strlen(curr_ins->type.name)+2 ) ; i++ )/*Make space between the labels name to their decimal address according to the needed space length*/
		fprintf(ext_file, " ");
	
	free(curr_ins->type.name);
	curr_ins->type.bmc = EXTERN_VALUE;/*Set the value of EXTERN label*/
	curr_ins->line = HAVE_BMC;/*Update that the BMC of the node has been defined*/
	fprintf(ext_file, "%04d\n", curr_ins->ic);/*Print their decimal address*/
}


/********************************************************************* - END OF SECOND_PASS - *********************************************************************/

