
/*
 * mkbkp.c
 *
 *  Created on: May 23, 2013
 *      Author: Or Dahan 201644929
 */

#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define FLAG_COMPRESS "-c"
#define FLAG_EXTRACT  "-x"
#define PATH_MAX_LENGTH 97

typedef struct
{
	char szPath[PATH_MAX_LENGTH];
}file_header;

void compress(const char* szPathToCompress, FILE* flResult);

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
		// Compression
		if (fIsCompressing == 1)
		{
			/* Open the archive file to store the compression
			 * in
			 */
			FILE* archive = fopen(argv[2], "wb");

			if (archive == NULL)
			{
				printf("Error opening %s for writing as archive file: %s\n",
						argv[2],
						strerror(errno));
				return -1;
			}

			compress(argv[3], archive);

			if (fclose(archive) != 0)
			{
				printf("Error closing %s: %s\n",
						argv[2],
						strerror(errno));
				return -1;
			}
		}
		// Extract
		else
		{

		}
	}

	return 0;
}


void compress(const char* szPath, FILE* flResult)
{
	file_header current_file;
	struct stat to_compress;

	// Set the path in the header
	// TODO: RELATIVE
	memset(current_file.szPath, 0, sizeof(current_file.szPath));
	strcpy(current_file.szPath, szPath);

	if (szPath == NULL ||
		flResult == NULL)
		return;

	// Make sure that the requested node is
	// either a file, a symlink or a folder
	if (0 == lstat(szPath, &to_compress))
	{
		if (S_ISREG(to_compress.st_mode) ||
			S_ISLNK(to_compress.st_mode))
		{
			// Write the current file header to the archive
			if (fwrite(&current_file,
					   sizeof(current_file),
					   1,
					   flResult) != 1)
			{
				printf("Error while writing header for file %s: %s\n",
						current_file.szPath,
						strerror(errno));
				return;
			}
		}
		else if (S_ISDIR(to_compress.st_mode))
		{

		}
		else
		{
			printf("file type not supported for %s: %d\n",
					szPath,
					to_compress.st_mode);
		}
	}
	else
	{
		printf("stat failed for %s: %s\n",
				szPath,
				strerror(errno));
	}
}
