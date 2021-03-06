#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "memfile.h"

#define INCREMENT 131072

struct memfile *memfile_open(int fd) {
	if (ftruncate(fd, INCREMENT) != 0) {
		return NULL;
	}

	char *map = mmap(NULL, INCREMENT, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED) {
		return NULL;
	}

	struct memfile *mf = malloc(sizeof(struct memfile));
	if (mf == NULL) {
		munmap(map, INCREMENT);
		return NULL;
	}

	mf->fd = fd;
	mf->map = map;
	mf->len = INCREMENT;
	mf->off = 0;

	return mf;
}

int memfile_close(struct memfile *file) {
	if (munmap(file->map, file->len) != 0) {
		return -1;
	}

	if (file->fd >= 0) {
		if (close(file->fd) != 0) {
			return -1;
		}
	}

	free(file);
	return 0;
}

int memfile_write(struct memfile *file, void *s, long long len) {
	if (file->off + len > file->len) {
		if (munmap(file->map, file->len) != 0) {
			return -1;
		}

		file->len += INCREMENT;

		if (ftruncate(file->fd, file->len) != 0) {
			return -1;
		}

		file->map = mmap(NULL, file->len, PROT_READ | PROT_WRITE, MAP_SHARED, file->fd, 0);
		if (file->map == MAP_FAILED) {
			return -1;
		}
	}

	memcpy(file->map + file->off, s, len);
	file->off += len;
	return len;
}
