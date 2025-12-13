#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char * argv[], char * envp[])
{
    // check where id is
    
    FILE * f = NULL;
    char* paths[] = {
      "/bin/id",
      "/usr/bin/id",
    };
    int i = 0;
    for (;i < sizeof(paths); i++) {
      if (!(f = fopen(paths[i], "r"))) {
        if (errno == EACCES) {
          printf("Failed with '%s'\n", paths[i]);
          continue;
        } else {
          printf("Failed opening '%s' with error: %d, exiting.\n", paths[i], errno);
          exit(errno);
        }
      }
      break;
    }
    if (i == sizeof(paths)) {
      printf("Failed to find a file, exiting.\n");
      return -1;
    }
    
    char * my_args[] = { paths[i] , NULL };
    setuid(0);
    setgid(0);
    execve(my_args[0], my_args, 0);
}
