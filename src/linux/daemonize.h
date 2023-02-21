#pragma once

#include <semaphore.h>
#include <exception>
#include <string>

namespace dolbyio::comms::sample {

constexpr char default_pid_file[] = "/tmp/cpp-injection.pid";

namespace daemon {
class parent_process_exception : std::exception {};

class failure_exception : std::exception {};
}  // namespace daemon

class daemonize {
 public:
  explicit daemonize(const std::string& log_dir);
  ~daemonize();

  void wait_indefinitely();
  void unblock_indefinte_wait();

 private:
  void parent_exit(pid_t pid);
  pid_t pid_{-1};
  sem_t semaphore_{};
};

}  // namespace dolbyio::comms::sample
