#include <stdio.h>
#include "pr.h"

pid_t create_process()
{
    pid_t pid = fork();
    if (pid == -1) {
      return -1;
    }
    return pid;
}

