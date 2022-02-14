CS7071 - Advanced Database Systems - Course Project
====================================================

Phase one of the course project: implementing a storage manager.

Functions "createPageFile", "openPageFile", "closePageFile", "readBlock" and "destroyPageFile" are implemented first and other functions are using these functions.  
Any fucntion that gets a SM_FileHandle as an argument checks if the file handle structure is initialized. If not, it returns RC_FILE_HANDLE_NOT_INIT.

In "openPageFile" function, to count the number of pages in the file, we iterate through the file and count the number of pages.

This code is written to be a portable C program. It means it calls no system calls directly and uses the standard C library functions.