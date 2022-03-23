#ifndef _HUGOS_INITRAMFS_H
#define _HUGOS_INITRAMFS_H

#include <stdint.h>
#include <stddef.h>

class Initramfs {
	private:
		uint64_t data_start;
		uint64_t data_end;
	public:
		Initramfs(uint64_t data_start, uint64_t data_end): data_start(data_start), data_end(data_end) {};
		size_t locate_file(const char* filename, void **data);
		void print_all_files();
};

#endif
