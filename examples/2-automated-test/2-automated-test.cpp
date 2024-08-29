#include "FileHelperRK.h"
#include "FileHelperRK_AutomatedTest.h"


SYSTEM_THREAD(ENABLED);

SerialLogHandler logHandler(LOG_LEVEL_TRACE);

unsigned long lastRun = 0;

void setup() {
    baseDir = "/usr";
}

void loop() {
    if (millis() - lastRun > 30000) {
        lastRun = millis();
        runTest();
    }
}




