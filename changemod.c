#include	<sys/types.h>
#include	<sys/stat.h>
#include        <fcntl.h>
#include        <stdlib.h>
#include        <stdio.h>
#include        <unistd.h>

int
main(void)
{
	struct stat		statbuf;

	// remove foo and bar if exist
	unlink("foo"); unlink("bar");

	printf("create file foo with -rw-rw-rw-\n");fflush(stdout);
	umask(0); 
        if (creat("foo", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) < 0)
                fprintf(stderr,"creat error for foo");

        system("ls -all foo");


	printf("\n\ncreate file bar with -rw------- using a mask\n");fflush(stdout); 
        umask(S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if (creat("bar", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) < 0)
                fprintf(stdout,"creat error for bar");


	system("ls -all bar");

	printf("\n\nturn on  group-execute permission for the foo\n");
	fflush(stdout);

	if (stat("foo", &statbuf) < 0)
		fprintf(stderr,"stat error for foo");


	if (chmod("foo", statbuf.st_mode | S_IXGRP) < 0)
		fprintf(stderr,"chmod error for foo");

        system("ls -all foo");


        printf("\n\nset -rw-r--r-- permissions for the bar\n");fflush(stdout);
	if (chmod("bar", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) < 0)
		fprintf(stderr,"chmod error for bar");

        system("ls -all bar");


	exit(0);
}
