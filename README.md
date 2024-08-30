# FileHelperRK

*Useful functions for working with the Particle flash file system*

This library contains a number of utility functions for working with the Particle flash file system on Gen 3 and Gen 4 devices, such as:

- Store or read a string in a file
- Store or read an array of bytes in a file
- Store or read a struct in a file
- Store or read a Variant in a file. This can be used for structured data, including JSON (Device OS 5.6.0 and later).
- Create a directory and parent directories (mkdirs)
- Delete a directory recursively (deleteRecursive)
- Measure disk usage of a directory
- Walk the directory tree and call a callback or lambda for each file or directory
- Parse a pathname
- Join pathname components

There is full [browseable API documentation](https://rickkas7.github.io/FileHelperRK/class_file_helper_r_k.html) available
online and in the docs directory of this repository.


License: MIT (can use in free or commercial open or closed source products)
Github Repository: https://github.com/rickkas7/FileHelperRK
 
## Version history

### 0.0.2 (2024-08-30)

- Added storeStruct and readStruct

### 0.0.1 (2024-08-30)

- Initial version

