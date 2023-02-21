/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022-2023 by Dolby Laboratories.
 ***************************************************************************/

#include "daemonize.h"

#include <fstream>

#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace dolbyio::comms::sample {

daemonize::daemonize(const std::string &log_dir) {
  // Initialize semaphore with 0 used for indefinite wait
  sem_init(&semaphore_, 0, 0);

  pid_t pid;
  pid = fork();
  parent_exit(pid);

  // Make the child the new session leader
  if (setsid() < 0)
    throw daemon::failure_exception();

  pid = fork();
  parent_exit(pid);

  // Now we are in the child solely, close all descriptiors
  int open_max;
  for (open_max = sysconf(_SC_OPEN_MAX); open_max >= 0; open_max--)
    close(open_max);

  // Make sure all std fd's are at dev null
  open("/dev/null", O_RDONLY);
  open("/dev/null", O_WRONLY);
  open("/dev/null", O_WRONLY);

  pid_ = getpid();
  std::ofstream pid_file(log_dir + "/pid");
  pid_file << std::to_string(pid_);
}

daemonize::~daemonize() {
  close(0);
  close(1);
  close(2);
}

void daemonize::wait_indefinitely() {
  sem_wait(&semaphore_);
}

void daemonize::unblock_indefinte_wait() {
  sem_post(&semaphore_);
}

void daemonize::parent_exit(pid_t pid) {
  if (pid < 0)
    throw daemon::failure_exception();
  // Parent exit with success
  if (pid > 0)
    throw daemon::parent_process_exception();
}

}  // namespace cppinjection
