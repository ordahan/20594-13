/*
 * mkbkp.h
 *
 *  Created on: May 23, 2013
 *      Author: Or Dahan 201644929
 */

#ifndef MKBKP_H_
#define MKBKP_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define FLAG_COMPRESS "-c"
#define FLAG_EXTRACT  "-x"
#define PATH_MAX_LENGTH 256

typedef struct
{
	off_t  size;
}file_node_t;

typedef struct
{

}folder_node_t;

typedef struct
{
	time_t modification;

	union
	{
		file_node_t file;
		folder_node_t folder;
	}type;
}concrete_node_t;

typedef struct
{
}symbolic_link_node_t;

typedef struct
{
	char linked_path[PATH_MAX_LENGTH];
}symbolic_link_content_t;

typedef struct
{
	char   path[PATH_MAX_LENGTH];
	mode_t mode;
	uid_t  uid;
	gid_t  gid;

	union
	{
		concrete_node_t concrete;
		symbolic_link_node_t symbolic;
	};
}node_t;

void archive(const char* base, const char* file, FILE* archive_file);

void extract(FILE* archive);


#endif /* MKBKP_H_ */
