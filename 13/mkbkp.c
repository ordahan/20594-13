
/*
 * mkbkp.c
 *
 *  Created on: May 23, 2013
 *      Author: Or Dahan 201644929
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "mkbkp.h"

/* Internal functions */
/* TODO: doc */
int copy_file_content(FILE* dest, FILE* src);
void write_file_content_to_archive(node_t *header, FILE* dest);

int main(int argc, char** argv)
{
	unsigned int fValidSyntax = 0;
	unsigned int fIsCompressing = 0;

	/* At least 1 flag parameter must be entered
	 and an archive name */
	if (argc > 2)
	{
		/* Compress */
		if ((0 == strcmp(FLAG_COMPRESS, argv[1])) &&
			(argc == 4))
		{
			/* Syntax is ok */
			fValidSyntax = 1;
			fIsCompressing = 1;
		}
		/* Extract */
		else if ((0 == strcmp(FLAG_EXTRACT, argv[1])) &&
				 (argc == 3))
		{
			/* Syntax is ok */
			fValidSyntax = 1;
			fIsCompressing = 0;
		}
	}

	/* Syntax is incorrect */
	if (fValidSyntax == 0)
	{
		printf("Usage: mkbkp <-c|-x> <backup_file> [file_to_backup|directory_to_backup]\n");
		return -1;
	}
	/* Ok lets get this show on the road */
	else
	{
		/* Compression */
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

			/* Compress the given directory / file to archive! */
			compress(NULL, argv[3], archive);

			if (fclose(archive) != 0)
			{
				printf("Error closing %s: %s\n",
						argv[2],
						strerror(errno));
				return -1;
			}
		}
		/* Extract */
		else
		{

		}
	}

	return 0;
}

void compress(const char* base, const char* file, FILE* flResult)
{
	node_t header;
	struct stat status;

	/* Make sure ptrs are valid */
	if (file == NULL ||
		flResult == NULL)
		return;

	/* Set the path in the header
	 TODO: RELATIVE */
	memset(header.szPath, 0, sizeof(header.szPath));
	if (base)
	{
		strcpy(header.szPath, base);
		strcat(header.szPath, "/");
	}
	strcat(header.szPath, file);

	/* Make sure that the requested node is
	 either a file, a symlink or a folder */
	if (0 == lstat(header.szPath, &status))
	{
		/* Save the type & permissions of the node */
		header.mode = status.st_mode;

		/* Save the uid & gid */
		header.uid = status.st_uid;
		header.gid = status.st_gid;

		/* Save modification time (for file / folders) */
		header.concrete.modification = status.st_mtime;

		/* Save the file size (for files) */
		header.concrete.type.file.size = status.st_size;

		/* Write the current node header to the archive */
		if (fwrite(&header,
				   sizeof(header),
				   1,
				   flResult) != 1)
		{
			printf("Error while writing header for node %s: %s\n",
					header.szPath,
					strerror(errno));
			return;
		}

		/* Symlink */
		if (S_ISLNK(status.st_mode))
		{
			/* TODO: anything else to do actually? */
		}
		/* Folder / File */
		else if (S_ISREG(status.st_mode) ||
				 S_ISDIR(status.st_mode))
		{
			/* Folder */
			if (S_ISDIR(status.st_mode))
			{
				DIR* dir = opendir(header.szPath);
				struct dirent* entry = NULL;

				/* Make sure the directory stream opened ok */
				if (dir == NULL)
				{
					printf("Directory %s couldn't be opened\n",
							header.szPath);
					perror(strerror(errno));
				}

				/* Read all the entries in the directory */
				while ((entry = readdir(dir)))
				{
					/* Recursively run this function on the tree,
					 * don't traverse upwards or call on the
					 * current directory!*/
					if (strcmp(".", entry->d_name) != 0 &&
						strcmp("..", entry->d_name) != 0)
					{
						compress(header.szPath, entry->d_name, flResult);
					}
				}

				/* Make sure we stopped becuase we went thru
				 all the files */
				if (errno)
				{
					fprintf(stderr,
							"Error while processing directory %s: ",
							header.szPath);
					perror("");
				}

				/* Close down the dir stream */
				if (closedir(dir))
				{
					fprintf(stderr,
							"Cannot close directory %s: ",
							header.szPath);
					perror("");
				}
			}
			/* File */
			else
			{
				write_file_content_to_archive(&header, flResult);
			}
		}
		else
		{
			printf("file type not supported for %s: %d\n",
					header.szPath,
					status.st_mode);
		}
	}
	else
	{
		printf("stat failed for %s: %s\n",
				header.szPath,
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

void write_file_content_to_archive(node_t *header, FILE* dest)
{
	FILE* curr_file = NULL;

	/* Open the file that we wish to add to the archive */
	curr_file = fopen(header->szPath, "rb");
	if (curr_file == NULL)
	{
		printf("Error opening %s for compression: %s\n",
				header->szPath,
				strerror(errno));
		return;
	}

	/* Write the file content */
	if (0 != copy_file_content(dest, curr_file))
	{
		printf("Error compressing file %s: %s\n",
				header->szPath,
				strerror(errno));
		return;
	}

	/* Done reading the current file */
	if (fclose(curr_file) != 0)
	{
		printf("Error closing %s: %s\n",
				header->szPath,
				strerror(errno));
		return;
	}
}
