#include <stdio.h>    //printf(3)
#include <stdlib.h>   //exit(3)
#include <unistd.h>   //fork(3), chdir(3), sysconf(3)
#include <signal.h>   //signal(3)
#include <sys/stat.h> //umask(3)
#include <syslog.h>   //syslog(3), openlog(3), closelog(3)


/*                             Function Prototypes
***************************************************************************************************/

int
daemonize(const char* name, const char* path, const char* outfile, const char* errfile, const char* infile);

/**************************************************************************************************/


