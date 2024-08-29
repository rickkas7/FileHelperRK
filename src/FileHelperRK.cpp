#include "FileHelperRK.h"

#include <fcntl.h>
#include <sys/stat.h>

const char *FileHelperRK::pathDelim = "/";

static Logger _fileHelperLog("app.file");

int FileHelperRK::ParsedPath::parse(const char *path) {
    clear();

    char *pathCopy = strdup(path);
    if (!pathCopy) {
        return SYSTEM_ERROR_NO_MEMORY;
    }

    int len = strlen(pathCopy);
    startsWithSlash = (len > 0) && pathCopy[0] == '/';
    endsWithSlash = (len > 0) && pathCopy[len - 1] == '/';

    char *save = nullptr;
    const char *pathDelim = "/";

    const char *part = strtok_r(pathCopy, pathDelim, &save);
    while(part) {
        parts.push_back(part);

        part = strtok_r(nullptr, pathDelim, &save);
    }
    return SYSTEM_ERROR_NONE; // 0
}

void FileHelperRK::ParsedPath::clear() {
    parts.clear();
    startsWithSlash = false;
    endsWithSlash = false;
}

String FileHelperRK::ParsedPath::generatePathString(int numParts) {
    String result;

    if (numParts < 0 || numParts > (int)parts.size()) {
        numParts = (int)parts.size();
    }

    if (startsWithSlash) {
        result.concat(pathDelim);
    }
    for(int ii = 0; ii < numParts; ii++) {
        if (ii > 0) {
            result.concat(pathDelim);
        }
        result.concat(parts[ii]);        
    }

    return result;
}



int FileHelperRK::mkdirs(const char *path) {
    int result = SYSTEM_ERROR_UNKNOWN;

    ParsedPath parsed;
    result = parsed.parse(path);
    if (result != SYSTEM_ERROR_NONE) {
        return result;
    }

    int numParts = (int) parsed.getNumParts();
    int curPart = numParts;

    while(curPart > 0) {
        struct stat sb;

        String partialPath = parsed.generatePathString(curPart);
        result = stat(partialPath.c_str(), &sb);
        // _fileHelperLog.trace("curPart=%d result=%d errno=%d partialPath=%s", curPart, result, errno, partialPath.c_str());
        if (result == 0) {
            if ((sb.st_mode & S_IFDIR) == 0) {
                // Not a directory
                return SYSTEM_ERROR_FILESYSTEM_NOTDIR;
            }
            break;
        }
        curPart--;
    }

    if (curPart == numParts) {
        return SYSTEM_ERROR_NONE;
    }

    for(curPart++; curPart <= numParts; curPart++){
        String partialPath = parsed.generatePathString(curPart);

        result = mkdir(partialPath.c_str(), 0777);
        if (result) {
            return result;
        }
    }
    
    result = SYSTEM_ERROR_NONE;


    return result;
}

int FileHelperRK::storeBytes(const char *fileName, const uint8_t *dataPtr, size_t dataLen)
{
    int result = SYSTEM_ERROR_UNKNOWN;

    int fd = open(fileName, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd != -1) {
        if (dataPtr && dataLen > 0) {
            int resultLen = write(fd, dataPtr, dataLen);
            if (resultLen == dataLen) {
                result = SYSTEM_ERROR_NONE;
            }
            else {
                _fileHelperLog.error("storeBytes bad length expected=%d got=%d", (int)dataLen, (int)resultLen);
            }
        }
        else {
            // Empty string, not an error
            result = SYSTEM_ERROR_NONE;            
        }

        close(fd);
    }
    else {
        _fileHelperLog.info("storeBytes did not open fileName=%s errno=%d", fileName, errno);
        result = errnoToSystemError();
    }

    return result;
}


int FileHelperRK::storeString(const char *fileName, const String &data)
{
    return storeBytes(fileName, (const uint8_t *)data.c_str(), data.length());
}

int FileHelperRK::storeString(const char *fileName, const char *str)
{
    if (str) {
        return storeBytes(fileName, (const uint8_t *)str, strlen(str));
    }
    else {
        return storeBytes(fileName, nullptr, 0);
    }
}

int FileHelperRK::readBytes(const char *fileName, uint8_t *&dataPtr, size_t &dataLen, bool nullTerminate)
{
    int result = SYSTEM_ERROR_UNKNOWN;

    dataPtr = nullptr;
    dataLen = 0;

    int fd = open(fileName, O_RDONLY);
    if (fd != -1) {
        struct stat sb = {0};
        result = fstat(fd, &sb); 
        if (result == 0) {
            if (sb.st_size > 0) {
                dataPtr = new uint8_t[sb.st_size + (nullTerminate ? 1 : 0)];
                if (dataPtr) {
                    int readLen = read(fd, dataPtr, sb.st_size);
                    if (readLen == sb.st_size) {
                        if (nullTerminate) {
                            dataPtr[readLen] = 0;
                        }
                        dataLen = (size_t)readLen;
                        result = SYSTEM_ERROR_NONE;
                    }
                    else {
                        _fileHelperLog.error("readBytes bad length expected=%d got=%d", (int)sb.st_size, (int)readLen);
                        result = errnoToSystemError();
                    }        
                }
                else {
                    result = SYSTEM_ERROR_NO_MEMORY;
                }
            }
            else {
                // Empty file, not an error                
                result = SYSTEM_ERROR_NONE;
            }

            close(fd);
        }
        else {
            result = errnoToSystemError();
        }
    }
    else {
        _fileHelperLog.info("readBytes did not open fileName=%s errno=%d", fileName, errno);
        result = errnoToSystemError();
    }
    
    return result;
}



int FileHelperRK::readString(const char *fileName, String &resultStr)
{
    int result = SYSTEM_ERROR_UNKNOWN;
    resultStr = "";

    uint8_t *dataPtr = nullptr;
    size_t dataLen;

    result = readBytes(fileName, dataPtr, dataLen, true);
    if (result == SYSTEM_ERROR_NONE) {
        if (dataPtr && dataLen) {
            resultStr = (const char *)dataPtr;
        }
    }
    
    if (dataPtr) {
        delete[] dataPtr;
    }

    return result;
}

int FileHelperRK::errnoToSystemError() {
    switch(errno) {
        case EIO:
            return SYSTEM_ERROR_FILESYSTEM_IO;

        // SYSTEM_ERROR_FILESYSTEM_CORRUPT

        case ENOENT:
            return SYSTEM_ERROR_FILESYSTEM_NOENT;

        case EEXIST:
            return SYSTEM_ERROR_FILESYSTEM_EXIST;

        case ENOTDIR:
            return SYSTEM_ERROR_FILESYSTEM_NOTDIR;

        case EISDIR:
            return SYSTEM_ERROR_FILESYSTEM_ISDIR;

        case ENOTEMPTY:
            return SYSTEM_ERROR_FILESYSTEM_NOTEMPTY;

        case EBADF:
            return SYSTEM_ERROR_FILESYSTEM_BADF;

        case EFBIG:
            return SYSTEM_ERROR_FILESYSTEM_FBIG;

        case EINVAL:
            return SYSTEM_ERROR_FILESYSTEM_INVAL;

        case ENOSPC:
            return SYSTEM_ERROR_FILESYSTEM_NOSPC;

        case ENOMEM:
            return SYSTEM_ERROR_FILESYSTEM_NOMEM;

        default:
            return SYSTEM_ERROR_UNKNOWN;
    }

    return SYSTEM_ERROR_UNKNOWN;
}
