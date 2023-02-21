#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022-2023 by Dolby Laboratories.
 ***************************************************************************/
#include "wrappers/command_line_params.h"

#include "dolbyio/comms/sample/media_source/file/source_capture.h"

#include "utils/commands_handler.h"
#include "utils/interactor.h"
#include "wrappers/sdk.h"

#include <string>
#include <vector>

namespace dolbyio::comms::sample {

class media_io_wrapper : public interactor {
 public:
  media_io_wrapper(command_line::sdk& sdk_params) : sdk_params_(sdk_params) {}
  ~media_io_wrapper() override;

  void set_initial_capture(bool audio, bool video);

  // interactor interface
  void set_sdk(dolbyio::comms::sdk* sdk) override;
  void initialize_injection();
  void register_command_line_handlers(commands_handler& handler) override;
  void register_interactive_commands(commands_handler& handler) override;

  bool media_io_enabled() const { return media_io_; }

  const command_line::mediaio& get_params() const { return params_; }

 private:
  void new_file(bool add);
  void seek_to_in_file();

  std::unique_ptr<plugin::injector_paced> injector_{};
  std::unique_ptr<file_source> source_{};
  std::mutex sdk_lock_{};
  dolbyio::comms::sdk* sdk_;
  command_line::sdk& sdk_params_;
  command_line::mediaio params_;

  bool media_io_{false};
  std::string cmdline_config_touched_{};
};

};  // namespace dolbyio::comms::sample
