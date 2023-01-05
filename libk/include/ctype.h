/*
 * Copyright (c) 2023 Oliver Hiorns.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_LIBK_INCLUDE_CTYPE_H
#define POPCORN_LIBK_INCLUDE_CTYPE_H

__attribute__((unused)) static inline int islower(int c) { return 'a' <= c && c <= 'z'; }
__attribute__((unused)) static inline int isupper(int c) { return 'A' <= c && c <= 'Z'; }
__attribute__((unused)) static inline int isdigit(int c) { return '0' <= c && c <= '9'; }
__attribute__((unused)) static inline int isalpha(int c) { return islower(c) || isupper(c); }
__attribute__((unused)) static inline int isalnum(int c) { return isalpha(c) || isdigit(c); }
__attribute__((unused)) static inline int isblank(int c) { return c == ' ' || c == '\t'; }
__attribute__((unused)) static inline int iscntrl(int c) { return (0x00 <= c && c <= 0x1F) || c == 0x7F; }
__attribute__((unused)) static inline int isprint(int c) { return !iscntrl(c); }
__attribute__((unused)) static inline int isgraph(int c) { return isprint(c) && c != ' '; }
__attribute__((unused)) static inline int ispunct(int c) { return isgraph(c) && !isalnum(c); }
__attribute__((unused)) static inline int isspace(int c) { return c == ' ' || (0x09 <= c && c <= 0x0D); }
__attribute__((unused)) static inline int isxdigit(int c) { return isdigit(c) || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F'); }
__attribute__((unused)) static inline int tolower(int c) { return isupper(c) ? c - 'A' + 'a' : c; }
__attribute__((unused)) static inline int toupper(int c) { return islower(c) ? c - 'a' + 'A' : c; }

#endif
