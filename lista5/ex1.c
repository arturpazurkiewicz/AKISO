#include <stdio.h>
#include <zconf.h>

int main() {
    char *name[2];
    name[0] = "bash";
    name[1] = NULL;
    setuid(0);
    execvp("/bin/bash", name);
    return 0;
}