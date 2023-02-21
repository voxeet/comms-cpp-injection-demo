/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022-2023 by Dolby Laboratories.
 ***************************************************************************/

#include "wrappers/command_line_params.h"

namespace dolbyio::comms::sample::command_line {
void throw_bad_args_error(const char* option, const std::string& value) {
  std::cerr << "Invalid value for " << option << " argument: " << value
            << std::endl;
  throw std::runtime_error("Bad cmdline arguments");
}

int to_int(const std::string& value, const char* option) {
  int ret;
  try {
    ret = std::stoi(value);
  } catch (const std::exception& ex) {
    throw_bad_args_error(option, value + " " + ex.what());
  }
  if (std::to_string(ret) != value)
    throw_bad_args_error(option, value);
  return ret;
}

double to_double(const std::string& value, const char* option) {
  return static_cast<double>(to_int(value, option));
}
}  // namespace dolbyio::comms::sample::command_line
