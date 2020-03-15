#include <stdlib.h>
#include <string.h>

#include "sandbox.h"

static void usage(const char* prog) {
    printf("Usage: %s <character> <map>\n", prog);
}

int main(int argc, char** argv) {
    struct Sandbox sandbox = {0};

    if (argc < 3) {
        usage(argv[0]);
        return -1;
    }

    if (!sandbox_load(&sandbox, argv[1], argv[2])) {
        fprintf(stderr, "Error: sandbox_load failed\n");
        return -1;
    }

    while (sandbox.running) {
        sandbox_run(&sandbox);
    }
    return 0;
}
