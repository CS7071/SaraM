#include "buffer_mgr.h"

RC create_fifo(FifoCache* fifo, char* fname, int n) {
	fifo = (FifoCache*)malloc(sizeof(FifoCache));
	fifo->fh = (SM_FileHandle*)malloc(sizeof(SM_FileHandle));
	RC rc;
	rc = openPageFile(fname, fifo->fh);
	if (rc != RC_OK)
		return rc;
	fifo->pages = (BM_PageHandle*)(malloc(n * sizeof(BM_PageHandle)));
	int i;
	for (i = 0; i < n; i++) {
		(fifo->pages + i)->pageNum = NO_PAGE;
		(fifo->pages + i)->data = NULL;
	}
	fifo->fixCount = (int*)(malloc(sizeof(int) * n));
	fifo->dirtyFlags = (bool*)(malloc(sizeof(bool) * n));
	fifo->curent = 0;
	fifo->n = n;
	fifo->reads = 0;
	fifo->writes = 0;

	return RC_OK;
}

RC remove_fifo(FifoCache* fifo) {
	closePageFile(fifo->fh);
	free(fifo->fh);
	free(fifo->pages);
	free(fifo->dirtyFlags);
	free(fifo->fixCount);
	free(fifo);

	return RC_OK;
}

RC write_fifo(FifoCache* fifo, int idx) {
	RC rc;
	BM_PageHandle* ph;
	if (fifo->dirtyFlags[idx] == FALSE)
		return RC_OK;
	ph = get_fifo(fifo, idx);
	rc = writeBlock(ph->pageNum, fifo->fh, ph->data);
	if (rc == RC_OK) {
		fifo->writes++;
		fifo->dirtyFlags[idx] = FALSE;
	}
	return rc;
}

int replacement_fifo(FifoCache* fifo) {
	int pos;
	pos = fifo->curent;
	while (fifo->fixCount[pos] > 0) {
		pos++;
		pos = pos % fifo->n;
		if (pos == fifo->curent)
			return -1;
	}
	if (fifo->dirtyFlags[pos] == TRUE)
		write_fifo(fifo, pos);
	return pos;
}

RC read_fifo(FifoCache* fifo, int key) {
	RC rc;
	BM_PageHandle* ph;
	int pos;
	pos = replacement_fifo(fifo);
	if (pos == -1)
		return RC_FAIL;
	else {
		ph = fifo->pages + pos;
		rc = readBlock(ph->pageNum, fifo->fh, ph->data);
		if (rc == RC_OK) {
			fifo->curent++;
			fifo->curent = fifo->curent % fifo->n;
			fifo->reads++;
		}
		return rc;
	}
}

BM_PageHandle* get_fifo(FifoCache* fifo, int idx) {
	if (idx < 0)
		return NULL;
	else if (idx >= fifo->n)
		return NULL;
	else
		return fifo->pages + idx;
}

int lookup_fifo(FifoCache* fifo, int key) {
	int i;
	for (i = 0; i < fifo->n; i++)
		if (fifo->pages[i].pageNum == key)
			return i;
	RC rc;
	rc = read_fifo(fifo, key);
	if (rc == RC_OK)
		return lookup_fifo(fifo, key);
	return -1;
}

RC pin_fifo(FifoCache* fifo, int key, BM_PageHandle* ph) {
	int pos;
	pos = lookup_fifo(fifo, key);
	if (pos < 0)
		return RC_FAIL;
	fifo->fixCount[pos]++;
	BM_PageHandle* th = get_fifo(fifo, lookup_fifo(fifo, key));
	ph->data = th->data;
	ph->pageNum = th->pageNum;
	return RC_OK;
}

RC unpin_fifo(FifoCache* fifo, int key) {
	int pos;
	pos = lookup_fifo(fifo, key);
	fifo->fixCount[pos]--;
	return RC_OK;
}

RC dirty_fifo(FifoCache* fifo, int key) {
	int pos;
	pos = lookup_fifo(fifo, key);
	fifo->dirtyFlags[pos] = TRUE;
	return RC_OK;
}

RC force_fifo(FifoCache* fifo, int key) {
	int pos;
	pos = lookup_fifo(fifo, key);
	return write_fifo(fifo, pos);
}

PageNumber* content_fifo(FifoCache* fifo) {
	int* res;
	res = (int*)malloc(sizeof(int) * fifo->n);
	int i;
	for (i = 0; i < fifo->n; i++) {
		res[i] = fifo->pages[i].pageNum;
	}
	return res;
}

RC create_lru(LruCache* lru, char* fname, int n) {
	lru = (LruCache*)malloc(sizeof(LruCache));
	lru->fh = (SM_FileHandle*)malloc(sizeof(SM_FileHandle));
	RC rc;
	rc = openPageFile(fname, lru->fh);
	if (rc != RC_OK)
		return rc;
	lru->pages = (BM_PageHandle*)(malloc(n * sizeof(BM_PageHandle)));
	int i;
	for (i = 0; i < n; i++) {
		(lru->pages + i)->pageNum = NO_PAGE;
		(lru->pages + i)->data = NULL;
	}
	lru->timestamps = (int*)malloc(sizeof(int) * n);
	for (i = 0; i < n; i++)
		lru->timestamps[i] = -1;
	lru->fixCount = (int*)(malloc(sizeof(int) * n));
	lru->ts = 0;
	lru->dirtyFlags = (bool*)(malloc(sizeof(bool) * n));
	lru->n = n;
	lru->reads = 0;
	lru->writes = 0;

	return RC_OK;
}

RC remove_lru(LruCache* lru) {
	closePageFile(lru->fh);
	free(lru->timestamps);
	free(lru->fh);
	free(lru->pages);
	free(lru->dirtyFlags);
	free(lru->fixCount);
	free(lru);

	return RC_OK;
}

RC write_lru(LruCache* lru, int idx) {
	RC rc;
	BM_PageHandle* ph;
	if (lru->dirtyFlags[idx] == FALSE)
		return RC_OK;
	ph = get_lru(lru, idx);
	rc = writeBlock(ph->pageNum, lru->fh, ph->data);
	if (rc == RC_OK) {
		lru->writes++;
		lru->dirtyFlags[idx] = FALSE;
	}
	return rc;
}

int replacement_lru(LruCache* lru) {
	int pos;
	pos = 0;
	int i;
	for (i = 0; i < lru->n; i++) {
		if (lru->timestamps[i] < lru->timestamps[pos])
			if (lru->fixCount[i] == 0)
				pos = i;
	}
	if (lru->fixCount[pos] > 0)
		return -1;
	if (lru->dirtyFlags[pos] == TRUE)
		write_lru(lru, pos);
	return pos;
}

RC read_lru(LruCache* lru, int key) {
	RC rc;
	BM_PageHandle* ph;
	int pos;
	pos = replacement_lru(lru);
	if (pos == -1)
		return RC_FAIL;
	else {
		ph = lru->pages + pos;
		rc = readBlock(ph->pageNum, lru->fh, ph->data);
		if (rc == RC_OK) {
			lru->timestamps[pos] = lru->ts++;
		}
		return rc;
	}
}

BM_PageHandle* get_lru(LruCache* lru, int idx) {
	if (idx < 0)
		return NULL;
	else if (idx >= lru->n)
		return NULL;
	else {
		lru->timestamps[idx] = lru->ts++;
		return lru->pages + idx;
	}
}

int lookup_lru(LruCache* lru, int key) {
	int i;
	for (i = 0; i < lru->n; i++)
		if (lru->pages[i].pageNum == key) {
			lru->timestamps[i] = lru->ts++;
			return i;
		}
	RC rc;
	rc = read_lru(lru, key);
	if (rc == RC_OK)
		return lookup_lru(lru, key);
	return -1;
}

RC pin_lru(LruCache* lru, int key, BM_PageHandle* ph) {
	int pos;
	pos = lookup_lru(lru, key);
	if (pos < 0)
		return RC_FAIL;
	lru->fixCount[pos]++;
	BM_PageHandle* th = get_lru(lru, lookup_lru(lru, key));
	ph->data = th->data;
	ph->pageNum = th->pageNum;
	return RC_OK;
}

RC unpin_lru(LruCache* lru, int key) {
	int pos;
	pos = lookup_lru(lru, key);
	lru->fixCount[pos]--;
	return RC_OK;
}

RC dirty_lru(LruCache* lru, int key) {
	int pos;
	pos = lookup_lru(lru, key);
	lru->dirtyFlags[pos] = TRUE;
	return RC_OK;
}

RC force_lru(LruCache* lru, int key) {
	int pos;
	pos = lookup_lru(lru, key);
	return write_lru(lru, pos);
}

PageNumber* content_lru(LruCache* lru) {
	int* res;
	res = (int*)malloc(sizeof(int) * lru->n);
	int i;
	for (i = 0; i < lru->n; i++) {
		res[i] = lru->pages[i].pageNum;
	}
	return res;
}


RC initBufferPool(BM_BufferPool* const bm, const char* const pageFileName,
	const int numPages, ReplacementStrategy strategy,
	void* stratData) {
	bm->pageFile = pageFileName;
	bm->numPages = numPages;
	bm->strategy = strategy;
	switch (bm->strategy)
	{
	case RS_FIFO:
		return create_fifo((FifoCache*)bm->mgmtData, bm->pageFile, bm->numPages);
	case RS_LRU:
		return create_lru((LruCache*)bm->mgmtData, bm->pageFile, bm->numPages);
	default:
		break;
	}
}

RC shutdownBufferPool(BM_BufferPool* const bm) {
	int* fixCounts;
	fixCounts = getFixCounts(bm);
	int i;
	for (i = 0; i < bm->numPages; i++)
		if (fixCounts[i] > 0)
			return RC_FAIL;
	bool* dirtyFlags;
	dirtyFlags = getDirtyFlags(bm);
	for (i = 0; i < bm->numPages; i++) {
		if (dirtyFlags[i] == TRUE) {
			forceFlushPool(bm);
		}
	}
	switch (bm->strategy)
	{
	case RS_FIFO:
		remove_fifo((FifoCache*)(bm->mgmtData));
		break;
	case RS_LRU:
		remove_lru((LruCache*)(bm->mgmtData));
		break;
	default:
		break;
	}
}

RC forceFlushPool(BM_BufferPool* const bm) {
	int i;
			RC rc;
	for (i = 0; i < bm->numPages; i++) {
		switch (bm->strategy)
		{
		case RS_FIFO:
			rc = write_fifo((FifoCache*)(bm->mgmtData), i);
			if (rc != RC_OK)
				return rc;
		case RS_LRU:
			rc = write_lru((LruCache*)(bm->mgmtData), i);
			if (rc != RC_OK)
				return rc;
			break;
		default:
			break;
		}
	}
}

RC markDirty(BM_BufferPool* const bm, BM_PageHandle* const page) {
	int idx;
	FifoCache* fifo;
	LruCache* lru;
	switch (bm->strategy)
	{
	case RS_FIFO:
		fifo = (FifoCache*)(bm->mgmtData);
		idx = lookup_fifo(fifo, page->pageNum);
		return dirty_fifo(fifo, idx);
	case RS_LRU:
		lru = (LruCache*)(bm->mgmtData);
		idx = lookup_lru(lru, page->pageNum);
		return dirty_lru(lru, idx);
	default:
		break;
	}
}

RC unpinPage(BM_BufferPool* const bm, BM_PageHandle* const page) {
	FifoCache* fifo;
	LruCache* lru;
	switch (bm->strategy)
	{
	case RS_FIFO:
		fifo = (FifoCache*)(bm->mgmtData);
		return unpin_fifo(fifo, page->pageNum);
	case RS_LRU:
		lru = (LruCache*)(bm->mgmtData);
		return unpin_lru(lru, page->pageNum);
	default:
		break;
	}
}

RC forcePage(BM_BufferPool* const bm, BM_PageHandle* const page) {
	FifoCache* fifo;
	LruCache* lru;
	switch (bm->strategy)
	{
	case RS_FIFO:
		fifo = (FifoCache*)(bm->mgmtData);
		return force_fifo(fifo, page->pageNum);
	case RS_LRU:
		lru = (LruCache*)(bm->mgmtData);
		return force_lru(lru, page->pageNum);
	default:
		break;
	}
}

RC pinPage(BM_BufferPool* const bm, BM_PageHandle* const page,
	const PageNumber pageNum) {
	FifoCache* fifo;
	LruCache* lru;
	switch (bm->strategy)
	{
	case RS_FIFO:
		fifo = (FifoCache*)(bm->mgmtData);
		return pin_fifo(fifo, pageNum, page);
	case RS_LRU:
		lru = (LruCache*)(bm->mgmtData);
		return pin_lru(lru, pageNum, page);
	default:
		break;
	}
}

PageNumber* getFrameContents(BM_BufferPool* const bm) {
	FifoCache* fifo;
	LruCache* lru;
	switch (bm->strategy)
	{
	case RS_FIFO:
		fifo = (FifoCache*)(bm->mgmtData);
		return content_fifo(fifo);
	case RS_LRU:
		lru = (LruCache*)(bm->mgmtData);
		return content_lru(lru);
	default:
		break;
	}
}

bool* getDirtyFlags(BM_BufferPool* const bm) {
	FifoCache* fifo;
	LruCache* lru;
	switch (bm->strategy)
	{
	case RS_FIFO:
		fifo = (FifoCache*)(bm->mgmtData);
		return fifo->dirtyFlags;
	case RS_LRU:
		lru = (LruCache*)(bm->mgmtData);
		return lru->dirtyFlags;
	default:
		break;
	}
}

int* getFixCounts(BM_BufferPool* const bm) {
	FifoCache* fifo;
	LruCache* lru;
	switch (bm->strategy)
	{
	case RS_FIFO:
		fifo = (FifoCache*)(bm->mgmtData);
		return fifo->fixCount;
	case RS_LRU:
		lru = (LruCache*)(bm->mgmtData);
		return lru->fixCount;
	default:
		break;
	}
}

int getNumReadIO(BM_BufferPool* const bm) {
	FifoCache* fifo;
	LruCache* lru;
	switch (bm->strategy)
	{
	case RS_FIFO:
		fifo = (FifoCache*)(bm->mgmtData);
		return fifo->reads;
	case RS_LRU:
		lru = (LruCache*)(bm->mgmtData);
		return lru->reads;
	default:
		break;
	}
}

int getNumWriteIO(BM_BufferPool* const bm) {
	FifoCache* fifo;
	LruCache* lru;
	switch (bm->strategy)
	{
	case RS_FIFO:
		fifo = (FifoCache*)(bm->mgmtData);
		return fifo->writes;
	case RS_LRU:
		lru = (LruCache*)(bm->mgmtData);
		return lru->writes;
	default:
		break;
	}
}
