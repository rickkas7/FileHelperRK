#ifndef __FILEHELPERRK_H
#define __FILEHELPERRK_H

#include "Particle.h"

#include <vector>

/**
 * @brief Class to perform common file operations
 * 
 */
class FileHelperRK {
public:
    /**
     * @brief Measure file system usage
     */
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

    /**
     * @brief Class for providing Stream and Print object for reading or writing a file
     * 
     * Important limitations:
     * - This class is only for reading or writing, not both at the same time
     * - When reading from a stream, the underling file is not expected to change; it cannot be used for live tail for example
     */
    class FileStreamBase {
    public:
        /**
         * @brief Construct object; you will typically do this and then call openForReading or openForWriting
         */
        FileStreamBase();

        /**
         * @brief Construct object from an existing file descriptor from open()
         * 
         * @param fd File descriptor from open(). Will not be closed on destruction.
         */
        FileStreamBase(int fd);

        /**
         * @brief Destructor. 
         * 
         * If file was opened using openForReading(), openForWriting(), or open() and close()
         * has not been called, the file will be closed when the object is deleted. 
         */
        virtual ~FileStreamBase();
        
        /**
         * @brief Open a file
         * 
         * @param path Filename to open.
         * @param mode Mode such as O_RDONLY, or O_RDWR | O_CREAT | O_TRUNC.
         * @param perm Permissions, defaults to 0666 (read and write for everyone).
         * @return int SYSTEM_ERROR_NONE (0) on success or a system error code (non-zero)
         */
        int open(const char *path, int mode, int perm = 0666);

        /**
         * @brief Close the underling file if it was opened using open()
         * 
         * @return int SYSTEM_ERROR_NONE (0) on success or a system error code (non-zero)
         * 
         * If you don't call close(), the file will be closed on object destruction if
         * it was opened.
         */
        int close();

    protected:
        int fd = -1;
        bool closeFile = false;
    };

    /**
     * @brief Class for reading from a file as a Stream
     * 
     * Used for reading a Variant from a file as CBOR.
     */
    class FileStreamRead : public Stream, public FileStreamBase {
    public:
        /**
         * @brief Open a file for reading. Opens as O_RDONLY.
         * 
         * @param path Filename to read from.
         * @return int SYSTEM_ERROR_NONE (0) on success or a system error code (non-zero)
         */
        int open(const char *path);

        /**
         * @brief Start reading from the beginning of the file again.
         * 
         * @return int SYSTEM_ERROR_NONE (0) on success or a system error code (non-zero)
         */
        int rewind();

        // Overrides for stream

        /**
         * @brief Returns number of bytes available to read. Override for Stream pure virtual function.
         * 
         * @return int Number of bytes to read or 0 at end of file
         */
        virtual int available();

        /**
         * @brief Read a byte from the file.  Override for Stream pure virtual function.
         * 
         * @return int A value from 0 - 255 inclusive or -1 on error.
         */
        virtual int read();

        /**
         * @brief Read a byte from the file without consuming it.  Override for Stream pure virtual function.
         * 
         * @return int A value from 0 - 255 inclusive or -1 on error.
         */
        virtual int peek();

        /**
         * @brief Doesn't do anything. Override for Stream pure virtual function.
         */
        virtual void flush();

        /**
         * @brief Doesn't do anything. Override for Stream::Print pure virtual function.
         */
        virtual size_t write(uint8_t);

        /**
         * @brief Updates the fileSize parameter. Called from open().
         */
        void updateFileSize();

    protected:
        size_t fileSize = 0;  //!< File size in bytes, set in open() and updateFileSize().
        size_t fileOffset = 0; //!< File position, set in open() and rewind(), updated on read()

    };

    /**
     * @brief Class for writing to a file as a Print
     * 
     * Used for writing a Variant to a file as CBOR.
     */
    class FileStreamWrite : public Print, public FileStreamBase {
    public:
        /**
         * @brief Open a file for writing. Opens as O_RDWR | O_CREAT | O_TRUNC.
         * 
         * @param path Filename to write to. File will be created and truncated.
         * @return int SYSTEM_ERROR_NONE (0) on success or a system error code (non-zero)
         * 
         * If you want to use a different mode, call the open() method in the base class.
         */
        int open(const char *path);

        // Overrides for Print
        /**
         * @brief Writes a character to the file Override for Print pure virtual function.
         * 
         * @param c Byte to write. Can be binary data.
         * @return size_t Number of bytes written (normally 1). 0 on error.
         */
        virtual size_t write(uint8_t c);

        /**
         * @brief Writes multiple bytes to the file Override for Print pure virtual function.
         * 
         * @param buffer Pointer to a buffer of bytes to write
         * @param size Number of bytes to write.
         * @return size_t Number of bytes written (normally size).
         */
        virtual size_t write(const uint8_t *buffer, size_t size);
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

    /**
     * @brief Store a Variant to a file
     * 
     * @param fileName Filename to write to. File will be created and truncated.
     * @param variant Variant to write.
     * @return int SYSTEM_ERROR_NONE (0) on success or a system error code (non-zero)
     */
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

    /**
     * @brief Read file contents to a Variant object
     * 
     * @param fileName Filename to read from
     * @param variant Variant object filled in with the data
     * @return int SYSTEM_ERROR_NONE (0) on success or a system error code (non-zero)
     */
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

    /**
     * @brief Structure to hold the parameters to the walk method
     * 
     */
    struct WalkParameters {
        const char *path;   //!< Pathname
        bool isDirectory;   //!< true if a directory, false if a file
        size_t size;        //!< size of file if file (0 for directories)
    };

    /**
     * @brief Walk the file system calling the callback function or lambda
     * 
     * @param path Path to start checking (can be file or directory)
     * @param cb Callback function or lambda to call
     * @return int SYSTEM_ERROR_NONE (0) on success or a system error code (non-zero)
     * 
     */
    static int walk(const char *path, std::function<void(const WalkParameters &walkParameters)> cb);



    static const char *pathDelim; //!< Path delimeter ("/")
};



#endif // __FILEHELPERRK_H
