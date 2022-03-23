#include "initramfs.hpp"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <utils.h>

enum class file_type: uint8_t {
	regular = '0',
	hardlink = '1',
	symlink = '2',
	character_device = '3',
	block_device = '4',
	directory = '5',
	fifo = '6'
};

struct __attribute__((packed)) tar_file_header {
	char filename[100];
	uint64_t mode;
	uint64_t uid;
	uint64_t gid;
	char size[12];
	char mtime[12];
	uint64_t checksum;
	file_type type;
	char linked_filename[100];
	char ustar[6];
	char version[2];
	char user_name[32];
	char group_name[32];
	uint64_t device_major;
	uint64_t device_minor;
	char filename_prefix[155];
};

int oct2bin(char *str, int size) {
	int n = 0;
	char *c = str;
	while (size-- > 0) {
		n *= 8;
		n += *c - '0';
		c++;
	}
	return n;
}

size_t Initramfs::locate_file(const char* filename, void **data) {
	tar_file_header *ptr = reinterpret_cast<tar_file_header*>(this->data_start);

	while (memcmp(&ptr->ustar[0], "ustar", 5) == 0) {
		int filesize = oct2bin(&ptr->size[0], 11);
		if (strcmp(&ptr->filename[0], filename) == 0) {
			*data = static_cast<void*>(ADD_BYTES(ptr, 512));
			return filesize;
		}
		ptr = ADD_BYTES(ptr, (((filesize + 511) / 512) + 1) * 512);
	}

	return 0;
}

void Initramfs::print_all_files() {
	tar_file_header *ptr = reinterpret_cast<tar_file_header*>(this->data_start);

	fprintf(stdserial, "ramfs files:\n");

	while (memcmp(&ptr->ustar[0], "ustar", 5) == 0) {
		int filesize = oct2bin(&ptr->size[0], 11);
		fprintf(stdserial, "Found initramfs file %s, size %d, type %s\n", &ptr->filename[0], filesize, ptr->type == file_type::regular ? "regular" : ptr->type == file_type::directory ? "directory" : "other");
		ptr = ADD_BYTES(ptr, (((filesize + 511) / 512) + 1) * 512);
	}
}
