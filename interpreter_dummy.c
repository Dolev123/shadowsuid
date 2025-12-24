#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

const char* paths[] = {
  "/bin/id",
  "/usr/bin/id",
};
// check where id is
int locate_id() {
    FILE * f = NULL;
    int i = 0;
    for (;i < sizeof(paths); i++) {
      if (!(f = fopen(paths[i], "r"))) {
        if (errno == EACCES) {
          fprintf(stderr, "Failed with '%s'\n", paths[i]);
          continue;
        } else {
          fprintf(stderr, "Failed opening '%s' with error: %d, exiting.\n", paths[i], errno);
          exit(errno);
        }
      }
      break;
    }
    return id;
}

int main(int argc, char * argv[], char * envp[])
{
    int i = locate_id();
    if (i == sizeof(paths)) {
      fprintf(stderr, "Failed to find a file, exiting.\n");
      return -1;
    }
    
    char * my_args[] = { paths[i] , NULL };
    setuid(0);
    setgid(0);
    execve(my_args[0], my_args, 0);
}
