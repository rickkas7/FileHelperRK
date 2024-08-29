#include <stdio.h>
#include "FileHelperRK.h"
#include "FileHelperRK_AutomatedTest.h"

const char *testPath = "./test-output";

int main(int argc, char *argv[]) {
    mkdir(testPath, 0777);
    chdir(testPath);
    runTest();
    return 0;
}




