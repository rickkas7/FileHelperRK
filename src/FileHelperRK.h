#ifndef __FILEHELPERRK_H
#define __FILEHELPERRK_H

#include "Particle.h"


class FileHelperRK {
public:
    static int storeString(const char *fileName, const String &data);

    static int readString(const char *fileName, String &result);


};



#endif // __FILEHELPERRK_H
