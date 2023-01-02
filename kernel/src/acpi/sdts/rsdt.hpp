
#ifndef POPCORN_KERNEL_SRC_ACPI_RSDT_HPP
#define POPCORN_KERNEL_SRC_ACPI_RSDT_HPP

#include "sdt.hpp"

#include <memory/types.hpp>

namespace acpi {
	struct [[gnu::packed]] rsdt_t : system_description_table_t {
		class iterator {
		public:
			explicit iterator(const rsdt_t& rsdt, usize idx) : rsdt(rsdt), idx(idx) {
				this->is_xsdt = this->rsdt.is_xsdt();
			}

			memory::paddr_t operator*() {
				if (this->is_xsdt) return this->rsdt.SDT_ptrs.xsdt[this->idx];
				else return this->rsdt.SDT_ptrs.rsdt[this->idx];
			}
			iterator& operator++() {
				++this->idx;
				return *this;
			}

			friend bool operator==(const rsdt_t::iterator& lhs, const rsdt_t::iterator& rhs) {
				return (&lhs.rsdt == &rhs.rsdt) && (lhs.idx == rhs.idx);
			}
			friend bool operator!=(const rsdt_t::iterator& lhs, const rsdt_t::iterator& rhs) { return !(lhs == rhs); }

		private:
			const rsdt_t& rsdt;
			usize idx;
			bool is_xsdt;
		};

		union {
			memory::paddr_t xsdt[];
			memory::paddr32_t rsdt[];
		} SDT_ptrs;

		usize ptr_count() {
			auto ptr_table_size = this->length - sizeof(system_description_table_t);
			return ptr_table_size / (this->is_xsdt() ? 8 : 4);
		}

		iterator begin() { return iterator(*this, 0); }
		iterator end() { return iterator(*this, this->ptr_count()); }
		bool is_xsdt() const { return this->has_signature("XSDT"); }
	};

	static_assert(offsetof(rsdt_t, SDT_ptrs.xsdt) == 36);
	static_assert(offsetof(rsdt_t, SDT_ptrs.rsdt) == 36);
	static_assert(offsetof(rsdt_t, SDT_ptrs) == 36);
}   // namespace acpi

#endif   //POPCORN_KERNEL_SRC_ACPI_RSDT_HPP
