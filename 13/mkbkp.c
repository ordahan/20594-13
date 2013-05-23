
/*
 * mkbkp.c
 *
 *  Created on: May 23, 2013
 *      Author: Or Dahan 201644929
 */

#include <stdio.h>
#include <string.h>

#define FLAG_COMPRESS "-c"
#define FLAG_EXTRACT  "-x"

int main(int argc, char** argv)
{
	unsigned int fValidSyntax = 0;
	unsigned int fIsCompressing = 0;

	// At least 1 flag parameter must be entered
	// and an archive name
	if (argc > 2)
	{
		// Compress
		if ((0 == strcmp(FLAG_COMPRESS, argv[1])) &&
			(argc == 4))
		{
			// Syntax is ok
			fValidSyntax = 1;
			fIsCompressing = 1;
		}
		// Extract
		else if ((0 == strcmp(FLAG_EXTRACT, argv[1])) &&
				 (argc == 3))
		{
			// Syntax is ok
			fValidSyntax = 1;
			fIsCompressing = 0;
		}
	}

	// Syntax is incorrect
	if (fValidSyntax == 0)
	{
		printf("Usage: mkbkp <-c|-x> <backup_file> [file_to_backup|directory_to_backup]\n");
		return -1;
	}
	// Ok lets get this show on the road
	else
	{
		// Compresssion
		if (fIsCompressing == 1)
		{

		}
		else
		{

		}
	}

	return 0;
}
