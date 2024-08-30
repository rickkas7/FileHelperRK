# FileHelperRK

*Useful functions for working with the Particle flash file system*

This library contains a number of utility functions for working with the Particle flash file system on Gen 3 and Gen 4 devices, such as:

- Store or read a string in a file
- Store or read an array of bytes in a file
- Store or read a Variant in a file. This can be used for structured data, including JSON (Device OS 5.6.0 and later).
- Create a directory and parent directories (mkdirs)
- Delete a directory recursively (deleteRecursive)
- Measure disk usage of a directory
- Walk the directory tree and call a callback or labmda for each file or directory
- Parse a pathname
- Join pathname components




## Version history

### 0.0.1 (2024-08-30)

- Initial version

