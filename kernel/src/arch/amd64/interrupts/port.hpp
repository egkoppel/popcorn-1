/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_PORT_HPP
#define HUGOS_PORT_HPP

#include <stdint.h>

template<typename T> class Port {
public:
	Port() = delete;
};

template<> class Port<uint8_t> {
public:
	explicit Port(uint16_t port) noexcept : port(port) {}
	inline void write(uint8_t val) noexcept { __asm__ volatile("outb %b0, %1" : : "a"(val), "Nd"(this->port)); }
	inline uint8_t read() const noexcept {
		uint8_t ret;
		__asm__ volatile("inb %1, %b0" : "=a"(ret) : "Nd"(this->port));
		return ret;
	}

private:
	uint16_t port;
};

template<> class Port<uint16_t> {
public:
	explicit Port(uint16_t port) noexcept : port(port) {}
	inline void write(uint16_t val) noexcept { __asm__ volatile("out %w0, %1" : : "a"(val), "Nd"(this->port)); }
	inline uint16_t read() const noexcept {
		uint16_t ret;
		__asm__ volatile("in %1, %w0" : "=a"(ret) : "Nd"(this->port));
		return ret;
	}

private:
	uint16_t port;
};

template<> class Port<uint32_t> {
public:
	explicit Port(uint16_t port) noexcept : port(port) {}
	inline void write(uint32_t val) noexcept { __asm__ volatile("out %d0, %1" : : "a"(val), "Nd"(this->port)); }
	inline uint32_t read() const noexcept {
		uint32_t ret;
		__asm__ volatile("in %1, %d0" : "=a"(ret) : "Nd"(this->port));
		return ret;
	}

private:
	uint16_t port;
};

inline void io_wait() noexcept { __asm__ volatile("outb %b0, $0x80" : : "a"(0)); }

#endif
