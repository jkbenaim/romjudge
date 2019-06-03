#ifndef _MAPFILE_H_
#define _MAPFILE_H_

#ifdef __MINGW32__
#include <windows.h>
#endif
#include <stdbool.h>

struct MappedFile_s {
	void *data;
	uint64_t size;
#ifdef __MINGW32__
	HANDLE _hFile;
	HANDLE _hMapping;
#else
	int _fd;
#endif
};

struct MappedFile_s mapfile(char *filename, bool writable);
void unmapfile(struct MappedFile_s m);

/* _MAPFILE_H_ */
#endif
