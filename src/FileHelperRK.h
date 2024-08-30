#ifndef __FILEHELPERRK_H
#define __FILEHELPERRK_H

#include "Particle.h"

#include <fcntl.h>
#include <vector>

/**
 * @brief Class to perform common file operations
 * 
 */
class FileHelperRK {
public:
    class Usage {
    public:
        /**
         * @brief Measure disk usage for a directory and its subdirectories and files
         * 
         * @param path c-string containing an absolute or relative Unix-style pathname (slash separated) to a file or directory
         * @param clearStats Call clear() to reset stats
         * @return int SYSTEM_ERROR_NONE (0) on success or a system error code (non-zero)
         */
        int measure(const char *path, bool clearStats = true);

        /**
         * @brief Clear the measurement stats
         */
        void clear();

        /**
         * @brief Number of bytes total for all files
         * 
         * This is a sum of the file sizes, not the usage on disk.
         */
        size_t fileBytes = 0;

        /**
         * @brief Number of sectors used
         * 
         * Each sector is 512 bytes. Each file takes one sector for metadata plus enough
         * sectors to hold all of the data. Each directory takes one sector.
         * 
         * This is an approximate value and may vary from the actual usage.
         */
        size_t sectors = 0;

        /**
         * @brief Number of files
         */
        size_t numFiles = 0;

        /**
         * @brief Number of directories
         */
        size_t numDirectories = 0;
    };

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

        /**
         * @brief Get the filename of the last component of the path without an extension
         * 
         * @return String 
         */
        String getFileBaseName() const { return fileBaseName; };

        /**
         * @brief Get the filename extension of the last component of the path
         * 
         * @return String 
         */
        String getFileExtension() const { return fileExtension; };

    protected:
        bool startsWithSlash = false; //!< true if the parsed path began with a slash (absolute path)
        bool endsWithSlash = false; //!< true if the parsed path ended with a slash
        String fileBaseName; //!< Filename (last part of path) without extension
        String fileExtension; //!< Filename (last part of path) extension (does not contain dot)
        std::vector<String> parts; //!< parsed parts of the pathname. Does not contain empty parts.
    };

    class PrintToFile : public Print {
    public:
        PrintToFile();
        PrintToFile(int fd);

        virtual ~PrintToFile();

        int open(const char *path, int mode = O_RDWR | O_CREAT | O_TRUNC, int perm = 0666);
        int close();

        virtual size_t write(uint8_t);
        virtual size_t write(const uint8_t *buffer, size_t size);

    protected:
        int fd = -1;
        bool closeFile = false;
    };

    class StreamFromFile : public Stream {
    public:
        StreamFromFile();
        StreamFromFile(int fd);

        virtual ~StreamFromFile();

        int open(const char *path, int mode = O_RDONLY, int perm = 0666);
        int close();

        virtual int available();
        virtual int read();
        virtual int peek();
        virtual int flush();

        int rewind();

    protected:
        int fd = -1;
        bool closeFile = false;
        size_t fileSize = 0;
        size_t fileOffset = 0;
    };

    /**
     * @brief Create all of the directories in path
     * 
     * @param path 
     * @return int SYSTEM_ERROR_NONE (0) on success or a system error code (non-zero)
     * 
     * The standard mkdir() function only will create the last component of the path.
     */
    static int mkdirs(const char *path);

    /**
     * @brief Delete a directory and all of the subdirectories and files
     * 
     * @param path The directory (or contents of the directory) to delete
     * @param contentsOfPathOnly If true, only delete the contents of path, leaving the top directory.
     * @return int 
     */
    static int deleteRecursive(const char *path, bool contentsOfPathOnly = false);

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

    /**
     * @brief Store a String object to a file
     * 
     * @param fileName Filename to write to. File will be created and truncated.
     * @param data String object to write. It only needs to remain valid until this method returns.
     * @return int SYSTEM_ERROR_NONE (0) on success or a system error code (non-zero)
     */
    static int storeString(const char *fileName, const String &data);

    /**
     * @brief Store a c-string to a file
     * 
     * @param fileName Filename to write to. File will be created and truncated.
     * @param str c-string to write. Can be an empty string or NULL to save an empty file.
     * @return int SYSTEM_ERROR_NONE (0) on success or a system error code (non-zero)
     */
    static int storeString(const char *fileName, const char *str);

    static int storeVariant(const char *fileName, const particle::Variant &variant);

    /**
     * @brief Read bytes from a file
     * 
     * @param fileName Filename to read from
     * @param dataPtr Filled in with an allocated pointer containing the data
     * @param dataLen On return, the length of the data in bytes
     * @param nullTerminate Set to true to null-terminate the buffer. Default is false. If null terminated, the dataLen is the actual string length, not including the null terminator.
     * @return int SYSTEM_ERROR_NONE (0) on success or a system error code (non-zero)
     * 
     * If the returned value in dataPtr is non-zero, you must delete it using delete[] dataPtr.
     */
    static int readBytes(const char *fileName, uint8_t *&dataPtr, size_t &dataLen, bool nullTerminate = false);

    /**
     * @brief Read file contents to a String object
     * 
     * @param fileName Filename to read from
     * @param result String object filled in with the data
     * @return int SYSTEM_ERROR_NONE (0) on success or a system error code (non-zero)
     */
    static int readString(const char *fileName, String &result);

    static int readVariant(const char *fileName, particle::Variant &variant);

    /**
     * @brief Internal function to convert the value of errno into a Particle system error code
     * 
     * @return int A system_error_t error code (0 = success, negative value on error)
     */
    static int errnoToSystemError();

    /**
     * @brief Join two pathnames or filenames, adding a / delimeter in between if necessary
     * 
     * @param a 
     * @param b 
     * @return String 
     */
    static String pathJoin(const char *a, const char *b);

    static const char *pathDelim; //!< Path delimeter ("/")
};



#endif // __FILEHELPERRK_H
