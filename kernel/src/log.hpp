
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_LOG_HPP
#define POPCORN_KERNEL_SRC_LOG_HPP

#include <type_traits>

namespace Log {
	enum level_t { OFF = -1, CRITICAL, WARNING, INFO, DEBUG, TRACE };

	void set_log_level(level_t level);
	void set_screen_log_level(level_t level);
	void log(level_t level, const char *message, const char *file_name, int line, const char *func_name, ...);
	void off();
	void on();
};   // namespace Log

#define LOG(level, fmt, ...)                                                                                           \
	{ Log::log(level, fmt, __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__); }

#endif   //POPCORN_KERNEL_SRC_LOG_HPP
