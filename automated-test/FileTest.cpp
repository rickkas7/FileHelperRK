#include <stdio.h>
#include "FileHelperRK.h"
#include "FileHelperRK_AutomatedTest.h"

char runCwd[1024];
char testPath[1024];

int main(int argc, char *argv[]) {
    getcwd(runCwd, sizeof(runCwd));

    // printf("runCwd=%s\n", runCwd);
    snprintf(testPath, sizeof(testPath), "%s/test-output", runCwd);
    printf("testPath=%s\n", testPath);

    mkdir(testPath, 0777); // make error, ignore error
    chdir(testPath);
    // chroot(testPath);

    baseDir = testPath;
    runTest();
    
    return 0;
}




