
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
#define PATH_MAX_LENGTH 256

typedef struct
{
	char   szPath[PATH_MAX_LENGTH];
	off_t  size;
	mode_t mode;
}file_header;

void compress(const char* szPathToCompress, FILE* flResult);
int copy_file_content(FILE* dest, FILE* src);

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
	file_header header;
	struct stat status;

	// Make sure ptrs are valid
	if (szPath == NULL ||
		flResult == NULL)
		return;

	// Set the path in the header
	// TODO: RELATIVE
	memset(header.szPath, 0, sizeof(header.szPath));
	strcpy(header.szPath, szPath);

	// Make sure that the requested node is
	// either a file, a symlink or a folder
	if (0 == lstat(szPath, &status))
	{
		// Save the type of the file
		header.mode = status.st_mode;

		// Save the size of the file
		header.size = status.st_size;

		// Save the modification time

		// Save the uid & gid

		// Save file permissions

		// Write the current file header to the archive
		if (fwrite(&header,
				   sizeof(header),
				   1,
				   flResult) != 1)
		{
			printf("Error while writing header for file %s: %s\n",
					header.szPath,
					strerror(errno));
			return;
		}

		// Save the file / folder / symlink
		if (S_ISREG(status.st_mode) ||
			S_ISLNK(status.st_mode))
		{
			FILE* curr_file = NULL;

			// Open the file that we wish to add to the archive
			curr_file = fopen(header.szPath, "rb");
			if (curr_file == NULL)
			{
				printf("Error opening %s for compression: %s\n",
						header.szPath,
						strerror(errno));
				return;
			}

			// Write the file content
			if (0 != copy_file_content(flResult, curr_file))
			{
				printf("Error compressing file %s: %s\n",
						header.szPath,
						strerror(errno));
				return;
			}

			// Done reading the current file
			if (fclose(curr_file) != 0)
			{
				printf("Error closing %s: %s\n",
						header.szPath,
						strerror(errno));
				return;
			}
		}
		else if (S_ISDIR(status.st_mode))
		{
			// TODO: Continue the recursion
		}
		else
		{
			printf("file type not supported for %s: %d\n",
					szPath,
					status.st_mode);
		}
	}
	else
	{
		printf("stat failed for %s: %s\n",
				szPath,
				strerror(errno));
	}
}

int copy_file_content(FILE* dest, FILE* src)
{
	char buf[256];
	size_t bytes_read = 0;

	if (dest == NULL ||
		src == NULL)
		return -1;

	while ((bytes_read = fread(buf, 1, sizeof(buf), src)) > 0)
	{
		if (fwrite(buf, 1, bytes_read, dest) != bytes_read)
		{
			return -1;
		}
	}

	/* Ended because of an error and not eof? */
	if (feof(src))
		return 0;

	return -1;
}
