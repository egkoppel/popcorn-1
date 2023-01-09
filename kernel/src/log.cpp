
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "log.hpp"

#include <cstdio>
#include <popcorn_prelude.h>
#include <termcolor.h>

namespace Log {
	namespace {
		level_t old_level            = DEBUG;
		level_t current_level        = DEBUG;
		level_t current_screen_level = WARNING;
	}   // namespace

	constexpr const char *level_to_prefix(level_t level) {
		switch (level) {
			case OFF: return "";
			case CRITICAL: return "CRITICAL";
			case WARNING: return "WARN";
			case INFO: return "INFO";
			case DEBUG: return "DEBUG";
			case TRACE: return "TRACE";
		}
	}

	constexpr const char *level_to_color(level_t level) {
		switch (level) {
			case CRITICAL: return TERMCOLOR_RED;
			case WARNING: return TERMCOLOR_YELLOW;
			case INFO: return TERMCOLOR_CYAN;
			case OFF: [[fallthrough]];
			case DEBUG: [[fallthrough]];
			case TRACE: return TERMCOLOR_WHITE;
		}
	}

	void set_log_level(level_t level) { current_level = level; }
	void set_screen_log_level(level_t level) { current_screen_level = level; }
	void off() {
		old_level     = current_level;
		current_level = WARNING;
	}
	void on() { current_level = old_level; }
	void log(level_t level, const char *message, const char *file_name, int line, const char *func_name, ...) {
		if (level > current_level) return;

		u32 time_low, time_high;
		__asm__ volatile("rdtsc" : "=a"(time_low), "=d"(time_high));
		u64 time = (static_cast<u64>(time_high) << 32) | time_low;

		va_list format_args;
		va_list format_args2;
		va_start(format_args, func_name);
		va_copy(format_args2, format_args);

		std::fprintf(stdserial,
		             "[%llu] [%s] - [%s:%s:%u] - ",
		             time,
		             level_to_prefix(level),
		             file_name,
		             func_name,
		             line);
		std::vfprintf(stdserial, message, format_args);
		va_end(format_args);
		std::fprintf(stdserial, "\n");

		if (level <= current_screen_level) {
			std::fprintf(stdout, "[%s%s" TERMCOLOR_RESET "] ", level_to_color(level), level_to_prefix(level));
			std::vfprintf(stdout, message, format_args2);
			std::fprintf(stdout, "\n");
		}
		va_end(format_args2);
	}
}   // namespace Log
