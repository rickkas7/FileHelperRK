#include "FileHelperRK.h"

SYSTEM_THREAD(ENABLED);

SerialLogHandler logHandler(LOG_LEVEL_TRACE);

unsigned long lastRun = 0;

void runTest();

void setup() {
}

void loop() {
    if (millis() - lastRun > 30000) {
        lastRun = millis();
        runTest();
    }
}

void runTest() {
    FileHelperRK::walk("/", [](const FileHelperRK::WalkParameters &walkParameters) {
        Log.info("walk: %s", walkParameters.toString().c_str());
    });

    FileHelperRK::Usage usage;
    usage.measure("/");
    Log.info("usage: %s", usage.toString().c_str());
}



