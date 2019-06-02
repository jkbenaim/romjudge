#ifndef _MAPFILE_H_
#define _MAPFILE_H_

#ifdef __MINGW32__
#include <windows.h>
#endif

struct MappedFile_s {
	void *data;
	uint64_t size;
#ifdef __MINGW32__
	HANDLE _hFile;
	HANDLE _hMapping;
#endif
};


#ifdef __MINGW32__
#include <stdbool.h>

#define MAXFILESIZE (256*1024*1024)
struct MappedFile_s mapfile(char *filename, bool writable);
void unmapfile(struct MappedFile_s m);

/* __MINGW32__ */
#endif

/* _MAPFILE_H_ */
#endif
