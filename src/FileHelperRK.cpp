#include "FileHelperRK.h"

#include <fcntl.h>
#include <dirent.h>
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

    clear();

    result = walk(path, [this](const WalkParameters &walkParameters) {
        if (walkParameters.isDirectory) {
            numDirectories++;
            sectors++;
        }
        else {
            fileBytes += walkParameters.size;
            sectors += ((walkParameters.size + 511) / 512) + 1;
            numFiles++;        
        }
    });

    return result;
}

void FileHelperRK::Usage::clear() {
    fileBytes = 0;
    sectors = 0;
    numFiles = 0;
    numDirectories = 0;
}

String FileHelperRK::Usage::toString() const {
    return String::format("fileBytes=%d, sectors=%d, numFiles=%d, numDirectories=%d",
        (int)fileBytes, (int)sectors, (int)numFiles, (int)numDirectories);
}


FileHelperRK::FileStreamBase::FileStreamBase() : fd(-1), closeFile(false) {
}

FileHelperRK::FileStreamBase::FileStreamBase(int fd) : fd(fd), closeFile(false) {

}

int FileHelperRK::FileStreamBase::open(const char *path, int mode, int perm) {
    int result = SYSTEM_ERROR_UNKNOWN;

    fd = ::open(path, mode, perm);
    if (fd != -1) {
        closeFile = true;

        result = SYSTEM_ERROR_NONE;
    }
    else {
        _fileHelperLog.info("FileStream::open did not open fileName=%s errno=%d", path, errno);
        result = errnoToSystemError();
    }
    return result;
}

FileHelperRK::FileStreamBase::~FileStreamBase() {
    if (closeFile && fd != -1) {
        ::close(fd);
        fd = -1;
    }
}

int FileHelperRK::FileStreamBase::close() {
    if (fd != -1) {
        ::close(fd);
        fd = -1;
    }
    return SYSTEM_ERROR_NONE;
}

int FileHelperRK::FileStreamRead::open(const char *path) {
    int result = FileStreamBase::open(path, O_RDONLY, 0666);
    if (result == SYSTEM_ERROR_NONE) {
        fileOffset = 0;
        updateFileSize();
    }
    else {
        fileSize = fileOffset = 0;
    }

    return result;
}

void FileHelperRK::FileStreamRead::updateFileSize() {
    if (fd != -1) {
        struct stat sb;
        fstat(fd, &sb);
        fileSize = sb.st_size;
    }
    else {
        fileSize = 0;
    }
}


int FileHelperRK::FileStreamRead::available() {
    return fileSize - fileOffset;
}

int FileHelperRK::FileStreamRead::read() {
    int result = -1;
    uint8_t c;

    if (fd != -1) {
        if (fileOffset < fileSize) {
            if (::read(fd, &c, 1) == 1) {
                fileOffset++;
                result = (int)c;
            }
        }
    }
    return result;
}

int FileHelperRK::FileStreamRead::peek() {
    int result = read();
    if (result >= 0) {
        fileOffset--;
        lseek(fd, fileOffset, SEEK_SET);
    }
    return result;
}

void FileHelperRK::FileStreamRead::flush() {
}

int FileHelperRK::FileStreamRead::rewind() {
    lseek(fd, 0, SEEK_SET);
    fileOffset = 0;
    return SYSTEM_ERROR_NONE;
}

size_t FileHelperRK::FileStreamRead::write(uint8_t) {
    return 0;
}


int FileHelperRK::FileStreamWrite::open(const char *path) {
    return FileStreamBase::open(path, O_RDWR | O_CREAT | O_TRUNC, 0666);
}

size_t FileHelperRK::FileStreamWrite::write(uint8_t c) {
    size_t countResult = 0;
    if (fd != -1) {
        countResult = ::write(fd, &c, 1);
    }
    return countResult;
}

size_t FileHelperRK::FileStreamWrite::write(const uint8_t *buffer, size_t size) {
    size_t countResult = 0;

    if (fd != -1) {
        countResult = ::write(fd, buffer, size);
    }

    return countResult;
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
#if defined(SYSTEM_VERSION_550) || defined(UNITTEST)
                return SYSTEM_ERROR_FILESYSTEM_NOTDIR;
#else
                return -1;
#endif
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

String FileHelperRK::WalkParameters::toString() const {
    if (isDirectory) {
        return String::format("%s (directory)", path);
    }
    else {
        return String::format("%s (%d bytes)", path, size);
    }
}


int FileHelperRK::walk(const char *path, std::function<void(const WalkParameters &pParam)> cb) {
    int result = SYSTEM_ERROR_UNKNOWN;
    

    struct stat sb;

    result = stat(path, &sb);
    if (result == -1) {
        return errnoToSystemError();
    }

    WalkParameters walkParameters;
    walkParameters.path = path;


    if ((sb.st_mode & S_IFDIR) != 0) {
        walkParameters.isDirectory = true;
        walkParameters.size = 0;
        cb(walkParameters);

        // _fileHelperLog.trace("walk path=%s", path);  
        std::deque<String> filesToCheck;
        std::deque<String> directoriesToCheck;

        DIR *dirp = opendir(path);
        if (dirp) {      
            while(true) {
                struct dirent *de = readdir(dirp);
                if (!de) {
                    break;
                }
                // _fileHelperLog.trace("walk de->d_name=%s", de->d_name);  

                if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
                    continue;
                }

                if (de->d_type & DT_DIR) {
                    directoriesToCheck.push_back(de->d_name);
                }
                else 
                if (de->d_type & DT_REG) {
                    filesToCheck.push_back(de->d_name);
                }
            }

            closedir(dirp);        
        }        

        while(!directoriesToCheck.empty()) {
            result = walk(pathJoin(path, directoriesToCheck.front()), cb);
            if (result) {
                return result;
            }
            directoriesToCheck.pop_front();
        }
        while(!filesToCheck.empty()) {
            result = walk(pathJoin(path, filesToCheck.front()), cb);
            if (result) {
                return result;
            }
            filesToCheck.pop_front();
        }
    }
    else {
        walkParameters.isDirectory = false;
        walkParameters.size = sb.st_size;
        cb(walkParameters);
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
            if (resultLen == (int) dataLen) {
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

#if defined(SYSTEM_VERSION_560) || defined(UNITTEST)
int FileHelperRK::storeVariant(const char *fileName, const particle::Variant &variant) {
    int result = SYSTEM_ERROR_UNKNOWN;

    FileHelperRK::FileStreamWrite stream;

    result = stream.open(fileName);
    if (result != SYSTEM_ERROR_NONE) {
        return result;
    }
    result = particle::encodeToCBOR(variant, stream);

    stream.close();
    return result;
}
#endif // SYSTEM_VERSION_560

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

int FileHelperRK::readBytesNoAlloc(const char *fileName, uint8_t *dataPtr, size_t &dataLen) {
    int result = SYSTEM_ERROR_UNKNOWN;

    int fd = open(fileName, O_RDONLY);
    if (fd != -1) {
        struct stat sb = {0};
        result = fstat(fd, &sb); 
        if (result == 0) {
            if (dataLen > sb.st_size) {
                dataLen = sb.st_size;
            }

            if (dataLen > 0) {
                int readLen = read(fd, dataPtr, dataLen);
                if (readLen == dataLen) {
                    result = SYSTEM_ERROR_NONE;
                }
                else {
                    if (readLen >= 0) {
                        dataLen = (size_t) readLen;
                    }
                    else {
                        dataLen = 0;
                    }
                    _fileHelperLog.error("readBytesNoAlloc bad length expected=%d got=%d", (int)dataLen, (int)readLen);
                    result = errnoToSystemError();
                }        
            }
            else {
                // Empty file, not an error                
                dataLen = 0;
                result = SYSTEM_ERROR_NONE;
            }

            close(fd);
        }
        else {
            dataLen = 0;
            result = errnoToSystemError();
        }
    }
    else {
        _fileHelperLog.info("readBytesNoAlloc did not open fileName=%s errno=%d", fileName, errno);
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

#if defined(SYSTEM_VERSION_560) || defined(UNITTEST)
int FileHelperRK::readVariant(const char *fileName, particle::Variant &variant) {
    int result = SYSTEM_ERROR_UNKNOWN;

    FileHelperRK::FileStreamRead stream;

    result = stream.open(fileName);
    if (result != SYSTEM_ERROR_NONE) {
        return result;
    }

    result = particle::decodeFromCBOR(variant, stream);

    stream.close();
    return result;
}
#endif // SYSTEM_VERSION_560

int FileHelperRK::errnoToSystemError() {

    // Earlier versions of Device OS don't define these constants, so just always return unknown
#if defined(SYSTEM_VERSION_550) || defined(UNITTEST)
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
#endif // defined(SYSTEM_VERSION_550) || defined(UNITTEST)

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
