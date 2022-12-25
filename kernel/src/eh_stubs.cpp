/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

extern "C" {
	int sched_yield() { return 0; }

	int dladdr(void *, void *) { return 0; }

	int isdigit(int ch) { return ch >= '0' && ch <= '9' ? 1 : 0; }
}
