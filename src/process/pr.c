#include <stdio.h>
#include "pr.h"

pid_t *create_process()
{
    pid_t pid, *parent;
    pid = fork();
    if (!pid == -1) {
        *parent = &pid;
    }
    return parent;
}

