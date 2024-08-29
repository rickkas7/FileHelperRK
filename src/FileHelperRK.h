#ifndef __FILEHELPERRK_H
#define __FILEHELPERRK_H

#include "Particle.h"

#include <vector>

class FileHelperRK {
public:
    /**
     * @brief Container for a parsed pathname (Unix-style, with slashes)
     */
    class ParsedPath {
    public:
        /**
         * @brief Parse a pathname (Unix-style, with slashes)
         * 
         * @param path c-string containing an absolute or relative Unix-style pathname (slash separated)
         * @return int SYSTEM_ERROR_NONE (0) on success or a system error code (non-zero)
         */
        int parse(const char *path);

        /**
         * @brief Clear the parsed data in this object
         */
        void clear();

        /**
         * @brief Returns true if path starts with a slash (is absolute)
         * 
         * @return true Absolute path
         * @return false Relative path
         */
        bool getStartsWithSlash() const { return startsWithSlash; };

        /**
         * @brief Returns true if the path ends with a slash
         * 
         * @return true Path ends with a slash
         * 
         * Normally a path ending with a slash signifies a directory, but this function
         * does not validate that this is true on the file system. It merely tests
         * the input string.
         */
        bool getEndsWithSlash() const { return endsWithSlash; };

        /**
         * @brief Get the number of path parts
         * 
         * @return int (0 = empty path, 1 = one part)
         */
        int getNumParts() const { return parts.size(); };

        /**
         * @brief Get a pathname part from its index
         * 
         * @param index 0-based index to retrieve, must be 0 <= index < getNumParts()
         * @return String Copy of the pathname part
         */
        String getPart(size_t index) const { return parts[index]; };

        /**
         * @brief Get a pathname part from its index
         * 
         * @param index 0-based index to retrieve, must be 0 <= index < getNumParts()
         * @return String Copy of the pathname part
         */
        String operator[](size_t index) const { return getPart(index); };

        /**
         * @brief Generate a new pathname string
         * 
         * @param numParts -1 to include the whole string
         * @return String 
         */
        String generatePathString(int numParts = -1);


    protected:
        bool startsWithSlash = false; //!< true if the parsed path began with a slash (absolute path)
        bool endsWithSlash = false; //!< true if the parsed path ended with a slash
        std::vector<String> parts; //!< parsed parts of the pathname. Does not contain empty parts.
    };

    static int mkdirs(const char *path);

    /**
     * @brief Store bytes to a file
     * 
     * @param fileName Filename to write to. File will be created and truncated.
     * @param dataPtr Pointer to binary data to write
     * @param dataLen Length of data (0 or more bytes)
     * @return int SYSTEM_ERROR_NONE (0) on success or a system error code (non-zero)
     * 
     * See also storeString()
     */
    static int storeBytes(const char *fileName, const uint8_t *dataPtr, size_t dataLen);

    static int storeString(const char *fileName, const String &data);

    static int storeString(const char *fileName, const char *str);

    static int readBytes(const char *fileName, uint8_t *&dataPtr, size_t &dataLen, bool nullTerminate = false);

    static int readString(const char *fileName, String &result);

    static int errnoToSystemError();

    static const char *pathDelim;
};



#endif // __FILEHELPERRK_H
