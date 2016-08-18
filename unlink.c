#define _BSD_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>
#include "data_types.h"

#define SHARED_OBJECT_PATH         "/messenger"



int main(int argc, char* argv[])
{

 if (shm_unlink(SHARED_OBJECT_PATH) != 0) {
                perror("In shm_unlink()");
                exit(1);
        }
return 0;
}

