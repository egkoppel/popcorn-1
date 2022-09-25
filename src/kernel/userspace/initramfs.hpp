#ifndef _HUGOS_INITRAMFS_H
#define _HUGOS_INITRAMFS_H

#include <stdint.h>
#include <stddef.h>

class Initramfs {
private:
	uint64_t data_start;
	uint64_t data_end;
public:
	explicit Initramfs() : data_start(0), data_end(0) {};
	Initramfs(uint64_t data_start, uint64_t data_end) : data_start(data_start), data_end(data_end) {};
	size_t locate_file(const char *filename, void **data);
};

#endif
