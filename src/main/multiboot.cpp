#include "multiboot.hpp"

multiboot::tag_header* MultibootData::find_tag(multiboot::tag_types type) {
	uintptr_t current_tag = this->mb_data_start;
	while (current_tag < this->mb_data_end) {
		if (((multiboot::tag_header*)current_tag)->type == (uint32_t)type) {
			return (multiboot::tag_header*)current_tag;
		}
		current_tag += ((multiboot::tag_header*)current_tag)->size;
	}
	return nullptr;
}