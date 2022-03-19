#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include <stdlib.h>

// Include return codes and methods for logging errors
#include "dberror.h"
#include "storage_mgr.h"

// Include bool DT
#include "dt.h"

// Replacement Strategies
typedef enum ReplacementStrategy {
	RS_FIFO = 0,
	RS_LRU = 1,
	RS_CLOCK = 2,
	RS_LFU = 3,
	RS_LRU_K = 4
} ReplacementStrategy;

// Data Types and Structures
typedef int PageNumber;
#define NO_PAGE -1

typedef struct BM_BufferPool {
	char *pageFile;
	int numPages;
	ReplacementStrategy strategy;
	void *mgmtData; // use this one to store the bookkeeping info your buffer
	// manager needs for a buffer pool
} BM_BufferPool;

typedef struct BM_PageHandle {
	PageNumber pageNum;
	char *data;
} BM_PageHandle;


// convenience macros
#define MAKE_POOL()					\
		((BM_BufferPool *) malloc (sizeof(BM_BufferPool)))

#define MAKE_PAGE_HANDLE()				\
		((BM_PageHandle *) malloc (sizeof(BM_PageHandle)))

// Buffer Manager Interface Pool Handling
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, 
		const int numPages, ReplacementStrategy strategy,
		void *stratData);
RC shutdownBufferPool(BM_BufferPool *const bm);
RC forceFlushPool(BM_BufferPool *const bm);

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page);
RC unpinPage(BM_BufferPool* const bm, BM_PageHandle* const page);
RC forcePage(BM_BufferPool* const bm, BM_PageHandle* const page);
RC pinPage(BM_BufferPool* const bm, BM_PageHandle* const page,
	const PageNumber pageNum);

// Statistics Interface
PageNumber* getFrameContents(BM_BufferPool* const bm);
bool* getDirtyFlags(BM_BufferPool* const bm);
int* getFixCounts(BM_BufferPool* const bm);
int getNumReadIO(BM_BufferPool* const bm);
int getNumWriteIO(BM_BufferPool* const bm);


typedef struct FifoCache {
	SM_FileHandle* fh;
	BM_PageHandle* pages;
	int* fixCount;
	bool* dirtyFlags;
	int curent;
	int n;
	int reads;
	int writes;
} FifoCache;

RC create_fifo(FifoCache* fifo, char* fname, int n);
RC remove_fifo(FifoCache* fifo);
RC write_fifo(FifoCache* fifo, int idx);
int replacement_fifo(FifoCache* fifo);
RC read_fifo(FifoCache* fifo, int key);
BM_PageHandle* get_fifo(FifoCache* fifo, int idx);
int lookup_fifo(FifoCache* fifo, int key);
RC pin_fifo(FifoCache* fifo, int key, BM_PageHandle* ph);
RC unpin_fifo(FifoCache* fifo, int key);
RC dirty_fifo(FifoCache* fifo, int key);
RC force_fifo(FifoCache* fifo, int key);
PageNumber* content_fifo(FifoCache* fifo);



typedef struct LruCache {
	SM_FileHandle* fh;
	BM_PageHandle* pages;
	int* timestamps;
	int* fixCount;
	bool* dirtyFlags;
	int ts;
	int n;
	int reads;
	int writes;
} LruCache;

RC create_lru(LruCache* lru, char* fname, int n);
RC remove_lru(LruCache* lru);
RC write_lru(LruCache* lru, int idx);
int replacement_lru(LruCache* lru);
RC read_lru(LruCache* lru, int key);
BM_PageHandle* get_lru(LruCache* lru, int idx);
int lookup_lru(LruCache* lru, int key);
RC pin_lru(LruCache* lru, int key, BM_PageHandle* ph);
RC unpin_lru(LruCache* lru, int key);
RC dirty_lru(LruCache* lru, int key);
RC force_lru(LruCache* lru, int key);
PageNumber* content_lru(LruCache* lru);


#endif
