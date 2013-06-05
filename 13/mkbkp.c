
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
int copy_file_content(FILE* dest, FILE* src, size_t size_src);
void write_file_content_to_archive(node_t *header, FILE* archive_file);
int main(int argc, char** argv)
{
	unsigned int fValidSyntax = 0;
	unsigned int fIsCompressing = 0;

	/* At least 1 flag parameter must be entered
	 and an archive name */
	if (argc > 2)
	{
		/* archive */
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
		/* archiving */
		if (fIsCompressing == 1)
		{
			/* Open the archive file to store the archiving
			 * in
			 */
			FILE* archive_file = fopen(argv[2], "wb");

			if (archive_file == NULL)
			{
				printf("Error opening %s for writing as archive file: %s\n",
						argv[2],
						strerror(errno));
				return -1;
			}

			/* archive the given directory / file to archive! */
			archive(NULL, argv[3], archive_file);

			if (fclose(archive_file) != 0)
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
			/* Open the archive file to extract from
			 */
			FILE* archive_file = fopen(argv[2], "rb");

			if (archive_file == NULL)
			{
				printf("Error opening %s for reading as archive file: %s\n",
						argv[2],
						strerror(errno));
				return -1;
			}

			/* Extract the given archive */
			extract(archive_file);

			if (fclose(archive_file) != 0)
			{
				printf("Error closing %s: %s\n",
						argv[2],
						strerror(errno));
				return -1;
			}
		}
	}

	return 0;
}

void archive(const char* base, const char* file, FILE* archive_file)
{
	node_t header;
	struct stat status;

	/* Make sure ptrs are valid */
	if (file == NULL ||
		archive_file == NULL)
		return;

	memset(&header, 0, sizeof(header));

	/* Set the path in the header
	 TODO: RELATIVE */
	if (base)
	{
		strcpy(header.path, base);
		strcat(header.path, "/");
	}
	strcat(header.path, file);

	/* Make sure that the requested node is
	 either a file, a symlink or a folder */
	if (0 == lstat(header.path, &status))
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
				   archive_file) != 1)
		{
			printf("Error while writing header for node %s: %s\n",
					header.path,
					strerror(errno));
			return;
		}

		/* Symlink */
		if (S_ISLNK(status.st_mode))
		{
			symbolic_link_content_t content;

			memset(&content, 0, sizeof(content));

			/* Read the content of the link */
			int linked_path_length = readlink(header.path,
											  content.linked_path,
											  PATH_MAX_LENGTH);

			/* Make sure the link is valid for us to archive */
			if (linked_path_length >= PATH_MAX_LENGTH ||
				linked_path_length < 0)
			{
				printf("Error! symbolic link %s points to path which is too long!\n",
						header.path);
				perror("");
				return;
			}

			/* Write the link to the archive */
			if (fwrite(&content,
					   sizeof(content),
					   1,
					   archive_file) != 1)
			{
				printf("Error while writing content for node %s: %s\n",
						header.path,
						strerror(errno));
				return;
			}
		}
		/* Folder / File */
		else if (S_ISREG(status.st_mode) ||
				 S_ISDIR(status.st_mode))
		{
			/* Folder */
			if (S_ISDIR(status.st_mode))
			{
				DIR* dir = opendir(header.path);
				struct dirent* entry = NULL;

				/* Make sure the directory stream opened ok */
				if (dir == NULL)
				{
					printf("Directory %s couldn't be opened\n",
							header.path);
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
						archive(header.path, entry->d_name, archive_file);
					}
				}

				/* Make sure we stopped because we went thru
				 all the files */
				if (errno)
				{
					fprintf(stderr,
							"Error while processing directory %s: ",
							header.path);
					perror("");
				}

				/* Close down the dir stream */
				if (closedir(dir))
				{
					fprintf(stderr,
							"Cannot close directory %s: ",
							header.path);
					perror("");
				}
			}
			/* File */
			else
			{
				write_file_content_to_archive(&header, archive_file);
			}
		}
		else
		{
			printf("file type not supported for %s: %d\n",
					header.path,
					status.st_mode);
		}
	}
	else
	{
		printf("stat failed for %s: %s\n",
				header.path,
				strerror(errno));
	}
}

int copy_file_content(FILE* dest, FILE* src, size_t size_src)
{
	char byte_read = 0;
	size_t bytes_copied = 0;

	if (dest == NULL ||
		src == NULL)
		return -1;

	/* Copy byte by byte */
	while ((byte_read = fgetc(src)) != EOF &&
		   (size_src == 0 || bytes_copied < size_src))
	{
		if (fputc(byte_read, dest) == EOF)
		{
			return -1;
		}

		bytes_copied++;
	}

	/* Ended because of an error and not eof? */
	if (feof(src))
		return 0;

	return -1;
}

void write_file_content_to_archive(node_t *header, FILE* archive_file)
{
	FILE* curr_file = NULL;

	/* Open the file that we wish to add to the archive */
	curr_file = fopen(header->path, "rb");
	if (curr_file == NULL)
	{
		printf("Error opening %s for archiving: %s\n",
				header->path,
				strerror(errno));
		return;
	}

	/* Write the file content */
	if (0 != copy_file_content(archive_file, curr_file, 0))
	{
		printf("Error archiving file %s: %s\n",
				header->path,
				strerror(errno));
		return;
	}

	/* Done reading the current file */
	if (fclose(curr_file) != 0)
	{
		printf("Error closing %s: %s\n",
				header->path,
				strerror(errno));
		return;
	}
}

void extract(FILE* archive_file)
{
	node_t header;
	FILE* extracted_file = NULL;

	/* Make sure the file is valid */
	if (archive_file == NULL)
		return;

	/* Read the archive entry-by-entry,
	 * extracting each one to its correct place.
	 */
	while (1 == fread(&header, sizeof(header), 1, archive_file))
	{
		/* Act according to the node's type */
		if (S_ISREG(header.mode))
		{
			/* Create the file that is extracted */
			extracted_file = fopen(header.path, "wb");

			/* Make sure it was created properly */
			if (extracted_file == NULL)
			{
				printf("Error extracting file %s: %s",
						header.path,
						strerror(errno));
				return;
			}

			/* Copy the contents of the extracted file */
			if (0 != copy_file_content(extracted_file,
									   archive_file,
									   header.concrete.type.file.size))
			{
				printf("Error extracting file %s: %s\n",
						header.path,
						strerror(errno));
				return;
			}
		}
		else if (S_ISDIR(header.mode))
		{

		}
		else if (S_ISLNK(header.mode))
		{

		}
		else
		{

		}

		/* Set all the general header properties */

		/* Close the file */
		if (extracted_file != NULL)
		{
			if (fclose(extracted_file) != 0)
			{
				printf("Error closing %s: %s\n",
						header.path,
						strerror(errno));
				return;
			}
		}
	}

	/* Ended because of an error and not eof? */
	if (!feof(archive_file))
	{
		/* todo: printf */
		return;
	}
}
