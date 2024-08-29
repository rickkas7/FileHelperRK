#include "FileHelperRK.h"

#include <fcntl.h>
#include <sys/stat.h>


int FileHelperRK::storeString(const char *fileName, const String &data)
{
    int resultLen = -1;

    int fd = open(fileName, O_RDWR | O_CREAT | O_TRUNC);
    if (fd != -1) {
        resultLen = write(fd, data.c_str(), data.length());
        close(fd);
    }
    return resultLen;
}

int FileHelperRK::readString(const char *fileName, String &result)
{
    int resultLen = -1;

    result = "";

    int fd = open(fileName, O_RDONLY);
    if (fd != -1) {
        struct stat sb = {0};
        fstat(fd, &sb); 
        resultLen = sb.st_size;

        if (resultLen > 0) {
            char *buf = new char[resultLen + 1];
            if (buf) {
                resultLen = read(fd, buf, resultLen);
                if (resultLen >= 0) {
                    buf[resultLen] = 0;
                    result = buf;
                }
    
                delete[] buf;
            }
        }
        else {
            resultLen = 0;
        }

        close(fd);
    }
    
    return resultLen;
}
