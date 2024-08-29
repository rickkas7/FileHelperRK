#include "FileHelperRK.h"

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <deque>

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

    if (!parts.empty()) {
        String str = parts[parts.size() - 1];
        
        const char offset = str.lastIndexOf(".");
        if (offset >= 0) {
            fileBaseName = str.substring(0, offset);
            fileExtension = str.substring(offset + 1);
        }
        else {
            fileBaseName = str;
        }
    }


    return SYSTEM_ERROR_NONE; // 0
}

void FileHelperRK::ParsedPath::clear() {
    parts.clear();
    startsWithSlash = false;
    endsWithSlash = false;
    fileBaseName = "";
    fileExtension = "";
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

int FileHelperRK::Usage::measure(const char *path, bool clearStats) {
    int result = SYSTEM_ERROR_UNKNOWN;
    
    if (clearStats) {
        clear();
    }

    struct stat sb;

    result = stat(path, &sb);
    if (result == -1) {
        return errnoToSystemError();
    }

    if ((sb.st_mode & S_IFDIR) != 0) {
        std::deque<String> toCheck;

        // _fileHelperLog.trace("deleteRecursive path=%s", path);  

        DIR *dirp = opendir(path);
        if (dirp) {      
            while(true) {
                struct dirent *de = readdir(dirp);
                if (!de) {
                    break;
                }
                // _fileHelperLog.trace("Usage::measure de->d_name=%s", de->d_name);  

                if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
                    continue;
                }

                toCheck.push_back(de->d_name);
            }

            closedir(dirp);        
        }        
        
        while(!toCheck.empty()) {
            result = measure(pathJoin(path, toCheck.front()), false);
            if (result) {
                return result;
            }
            toCheck.pop_front();
        }
        numDirectories++;
        sectors++;
    }
    else {
        fileBytes += sb.st_size;
        sectors += ((sb.st_size + 511) / 512) + 1;
        numFiles++;        
    }

    return result;
}

void FileHelperRK::Usage::clear() {
    fileBytes = 0;
    sectors = 0;
    numFiles = 0;
    numDirectories = 0;
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

        // _fileHelperLog.trace("mkdirs test curPart=%d result=%d errno=%d partialPath=%s", curPart, result, errno, partialPath.c_str());

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

        // _fileHelperLog.trace("mkdirs create curPart=%d result=%d errno=%d partialPath=%s", curPart, result, errno, partialPath.c_str());

        if (result) {
            return result;
        }
    }
    
    result = SYSTEM_ERROR_NONE;


    return result;
}

int FileHelperRK::deleteRecursive(const char *path, bool contentsOfPathOnly) {
    int result = SYSTEM_ERROR_UNKNOWN;

    std::deque<String> filesToDelete;
    std::deque<String> directoriesToDelete;

    // _fileHelperLog.trace("deleteRecursive path=%s", path);  

    DIR *dirp = opendir(path);
    if (dirp) {      
        while(true) {
            struct dirent *de = readdir(dirp);
            if (!de) {
                break;
            }
            // _fileHelperLog.trace("deleteRecursive de->d_name=%s", de->d_name);  

            if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
                continue;
            }

            if (de->d_type & DT_DIR) {
                directoriesToDelete.push_back(de->d_name);
            }
            else 
            if (de->d_type & DT_REG) {
                filesToDelete.push_back(de->d_name);
            }
        }

        closedir(dirp);        
    }

    while(!directoriesToDelete.empty()) {
        String newPath = pathJoin(path, directoriesToDelete.front());
        result = deleteRecursive(newPath, false);
        if (result) {
            _fileHelperLog.info("deleteRecursive inner failure fileName=%s result=%d", newPath.c_str(), result);
        }
        directoriesToDelete.pop_front();
    }

    while(!filesToDelete.empty()) {
        String newPath = pathJoin(path, filesToDelete.front());
        result = unlink(newPath);
        if (result == -1) {
            _fileHelperLog.info("deleteRecursive unlink failed fileName=%s errno=%d", newPath.c_str(), errno);
            result = errnoToSystemError();
            break;
        }
        filesToDelete.pop_front();
    }

    if (!contentsOfPathOnly) {
        result = rmdir(path);
        if (result == -1) {
            _fileHelperLog.info("deleteRecursive unlink self failed fileName=%s errno=%d", path, errno);
            result = errnoToSystemError();
        }
    }

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
                if (nullTerminate) {
                    dataPtr = new uint8_t[1];                    
                    dataPtr[0] = 0;
                }

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

String FileHelperRK::pathJoin(const char *a, const char *b) {
    String result;

    if (a && strlen(a)) {
        result.concat(a);
    }
    if (b && strlen(b)) {
        if (result.length() && result.charAt(result.length() - 1) != pathDelim[0]) {
            result.concat(pathDelim);
        }
        result.concat(b);
    }

    return result;
}
