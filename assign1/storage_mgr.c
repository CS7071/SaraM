#include "storage_mgr.h"

// /**
//  * @brief Initialize the storage manager
//  * 
//  */
// extern void initStorageManager (void) {
//     /* Just creating a directory to store files */
//     mkdir(".storage", 0777);
// }

/**
 * @brief Initialize the storage manager
 * 
 */
extern void initStorageManager (void) {
    return;
}


/**
 * @brief Create a Page File object
 * 
 * @param fileName Name of the file to be created
 * @return RC Return Code
 */
extern RC createPageFile (char *fileName) {
    FILE * fh;
    fh = fopen(fileName, "w+");
    const unsigned char zeros[PAGE_SIZE] = {0};
    fwrite(zeros, PAGE_SIZE, 1, fh);
    fclose(fh);
    return RC_OK;
}


/**
 * @brief Open a Page File object
 * 
 * @param fileName Name of the file to be opened
 * @param fHandle Pointer to the file handle structure
 * @return RC Return Code
 */
extern RC openPageFile (char *fileName, SM_FileHandle *fHandle) {
    if (fHandle == NULL) {
        printError(RC_FILE_HANDLE_NOT_INIT);
        return RC_FILE_HANDLE_NOT_INIT;
    }
    fHandle->fileName = fileName;
    fHandle->mgmtInfo = fopen(fileName, "r+");
    if (fHandle->mgmtInfo == NULL) {
        printError(RC_FILE_NOT_FOUND);
        return RC_FILE_NOT_FOUND;
    }
    fHandle->curPagePos = 0;
    //compute total number of pages
    // fclearerr(fHandle->mgmtInfo);
    int totalNumPages = 0;
    unsigned char buffer[PAGE_SIZE];
    while (1) {
        fseek(fHandle->mgmtInfo, totalNumPages * PAGE_SIZE, SEEK_SET);
        fread(buffer, PAGE_SIZE, 1, fHandle->mgmtInfo);
        if (feof(fHandle->mgmtInfo)) {
            break;
        }
        totalNumPages++;
    }
    fseek(fHandle->mgmtInfo, 0, SEEK_SET);
    fHandle->totalNumPages = totalNumPages;
    return RC_OK;
}


/**
 * @brief Close a Page File object
 * 
 * @param fHandle Pointer to the file handle structure
 * @return RC Return Code
 */
extern RC closePageFile (SM_FileHandle *fHandle) {
    if (fHandle == NULL) {
        printError(RC_FILE_HANDLE_NOT_INIT);
        return RC_FILE_HANDLE_NOT_INIT;
    }
    fclose(fHandle->mgmtInfo);
    return RC_OK;
}


/**
 * @brief Destroy a Page File object
 * 
 * @param fileName Name of the file to be destroyed
 * @return RC Return Code
 */
extern RC destroyPageFile (char *fileName) {
    if (remove(fileName) == 0) {
        return RC_OK;
    } else {
        printError(RC_FILE_NOT_FOUND);
        return RC_FILE_NOT_FOUND;
    }
}


/* reading blocks from disc */

/**
 * @brief Read a page from disc
 * 
 * @param pageNum Page number to be read
 * @param fHandle Pointer to the file handle structure
 * @param memPage Pointer to the page content in memory
 * @return RC Return Code
 */
extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle == NULL) {
        printError(RC_FILE_HANDLE_NOT_INIT);
        return RC_FILE_HANDLE_NOT_INIT;
    }
    if (pageNum < 0 || pageNum > fHandle->totalNumPages) {
        printError(RC_READ_NON_EXISTING_PAGE);
        return RC_READ_NON_EXISTING_PAGE;
    }
    fseek(fHandle->mgmtInfo, pageNum * PAGE_SIZE, SEEK_SET);
    fread(memPage, PAGE_SIZE, 1, fHandle->mgmtInfo);
    return RC_OK;
}


/**
 * @brief Get the Block Pos object
 * 
 * @param fHandle Pointer to the file handle structure
 * @return int Block position
 */
extern int getBlockPos (SM_FileHandle *fHandle) {
    return fHandle->curPagePos;
}


/**
 * @brief Read the first page from disc
 * 
 * @param fHandle Pointer to the file handle structure
 * @param memPage Pointer to the page content in memory
 * @return RC Return Code
 */
extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle == NULL) {
        printError(RC_FILE_HANDLE_NOT_INIT);
        return RC_FILE_HANDLE_NOT_INIT;
    }
    fHandle->curPagePos = 0;
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}


/**
 * @brief Read the next page from disc
 * 
 * @param fHandle Pointer to the file handle structure
 * @param memPage Pointer to the page content in memory
 * @return RC Return Code
 */
extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle == NULL) {
        printError(RC_FILE_HANDLE_NOT_INIT);
        return RC_FILE_HANDLE_NOT_INIT;
    }
    if (fHandle->curPagePos <= 0) {
        printError(RC_READ_NON_EXISTING_PAGE);
        return RC_READ_NON_EXISTING_PAGE;
    }
    return readBlock(--(fHandle->curPagePos), fHandle, memPage);
}


/**
 * @brief Read the next page from disc
 * 
 * @param fHandle Pointer to the file handle structure
 * @param memPage Pointer to the page content in memory
 * @return RC Return Code
 */
extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle == NULL) {
        printError(RC_FILE_HANDLE_NOT_INIT);
        return RC_FILE_HANDLE_NOT_INIT;
    }
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}


/**
 * @brief Read the next page from disc
 * 
 * @param fHandle Pointer to the file handle structure
 * @param memPage Pointer to the page content in memory
 * @return RC Return Code
 */
extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle == NULL) {
        printError(RC_FILE_HANDLE_NOT_INIT);
        return RC_FILE_HANDLE_NOT_INIT;
    }
    if (fHandle->curPagePos >= fHandle->totalNumPages - 1) {
        printError(RC_READ_NON_EXISTING_PAGE);
        return RC_READ_NON_EXISTING_PAGE;
    }
    return readBlock(++(fHandle->curPagePos), fHandle, memPage);
}


/**
 * @brief Read the last page from disc
 * 
 * @param fHandle Pointer to the file handle structure
 * @param memPage Pointer to the page content in memory
 * @return RC Return Code
 */
extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle == NULL) {
        printError(RC_FILE_HANDLE_NOT_INIT);
        return RC_FILE_HANDLE_NOT_INIT;
    }
    fHandle->curPagePos = fHandle->totalNumPages - 1;
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}


/* writing blocks to a page file */

/**
 * @brief Write a page to disc
 * 
 * @param pageNum Page number to be written
 * @param fHandle Pointer to the file handle structure
 * @param memPage Pointer to the page content in memory
 * @return RC Return Code
 */
extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (pageNum < 0 || pageNum > fHandle->totalNumPages) {
        printError(RC_WRITE_FAILED);
        return RC_WRITE_FAILED;
    }
    fseek(fHandle->mgmtInfo, pageNum * PAGE_SIZE, SEEK_SET);
    if (fwrite(memPage, PAGE_SIZE, 1, fHandle->mgmtInfo) != 1) {
        printError(RC_WRITE_FAILED);
        return RC_WRITE_FAILED;
    }
    return RC_OK;
}


/**
 * @brief Write a page to disc
 * 
 * @param fHandle Pointer to the file handle structure
 * @param memPage Pointer to the page content in memory
 * @return RC Return Code
 */
extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle == NULL) {
        printError(RC_FILE_HANDLE_NOT_INIT);
        return RC_FILE_HANDLE_NOT_INIT;
    }
    return writeBlock(fHandle->curPagePos, fHandle, memPage);
}


/**
 * @brief Append a page to disc
 * 
 * @param fHandle Pointer to the file handle structure
 * @return RC Return Code
 */
extern RC appendEmptyBlock (SM_FileHandle *fHandle) {
    if (fHandle == NULL) {
        printError(RC_FILE_HANDLE_NOT_INIT);
        return RC_FILE_HANDLE_NOT_INIT;
    }
    unsigned char zeros[PAGE_SIZE] = {0};
    if (writeBlock(fHandle->totalNumPages, fHandle, zeros) != RC_OK) {
        printError(RC_WRITE_FAILED);
        return RC_WRITE_FAILED;
    }
    fHandle->totalNumPages++;
    return RC_OK;
}


/**
 * @brief Get the total number of pages
 * 
 * @param numberOfPages Total number of pages to ensure
 * @param fHandle Pointer to the file handle structure
 * @return RC Return Code
 */
extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle) {
    if (fHandle == NULL) {
        printError(RC_FILE_HANDLE_NOT_INIT);
        return RC_FILE_HANDLE_NOT_INIT;
    }
    if (numberOfPages <= fHandle->totalNumPages) {
        return RC_OK;
    }
    int i;
    for (i = fHandle->totalNumPages; i < numberOfPages; i++) {
        if (appendEmptyBlock(fHandle) != RC_OK) {
            return RC_WRITE_FAILED;
        }
    }
    return RC_OK;
}
