/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_PORT_HPP
#define HUGOS_PORT_HPP

#include <popcorn_prelude.h>
#include <stdint.h>

template<typename T> class Port {
public:
	Port() = delete;
};

template<> class Port<u8> {
public:
	explicit Port(u16 port) noexcept : port(port) {}
	inline void write(u8 val) noexcept { __asm__ volatile("outb %b0, %1" : : "a"(val), "Nd"(this->port)); }
	inline u8 read() const noexcept {
		u8 ret;
		__asm__ volatile("inb %1, %b0" : "=a"(ret) : "Nd"(this->port));
		return ret;
	}

private:
	u16 port;
};

template<> class Port<u16> {
public:
	explicit Port(u16 port) noexcept : port(port) {}
	inline void write(u16 val) noexcept { __asm__ volatile("out %w0, %1" : : "a"(val), "Nd"(this->port)); }
	inline u16 read() const noexcept {
		u16 ret;
		__asm__ volatile("in %1, %w0" : "=a"(ret) : "Nd"(this->port));
		return ret;
	}

private:
	u16 port;
};

template<> class Port<u32> {
public:
	explicit Port(u16 port) noexcept : port(port) {}
	inline void write(u32 val) noexcept { __asm__ volatile("out %d0, %1" : : "a"(val), "Nd"(this->port)); }
	inline u32 read() const noexcept {
		u32 ret;
		__asm__ volatile("in %1, %d0" : "=a"(ret) : "Nd"(this->port));
		return ret;
	}

private:
	u16 port;
};

inline void io_wait() noexcept { __asm__ volatile("outb %b0, $0x80" : : "a"(0)); }

#endif
