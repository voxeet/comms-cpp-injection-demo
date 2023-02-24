#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022-2023 by Dolby Laboratories.
 ***************************************************************************/

#include "utils/interactor.h"

#include "wrappers/command_line_params.h"

#include <optional>
#include <string>

namespace dolbyio::comms::sample {

class sdk_wrapper : public interactor {
 public:
  ~sdk_wrapper() override;

  services::session::user_info session_options() const;
  services::conference::conference_options conference_options() const;
  services::conference::join_options join_options() const;
  services::conference::listen_options listen_options() const;
  const command_line::sdk& get_params() const { return params_; }
  command_line::sdk& get_params() { return params_; }

  void set_sdk(dolbyio::comms::sdk* sdk) override { sdk_ = sdk; }
  void register_command_line_handlers(commands_handler& handler) override;
  void register_interactive_commands(commands_handler& handler) override;

  std::future<void> set_local_spatial_position();
  dolbyio::comms::spatial_audio_batch_update set_spatial_environment();

  async_result<void> open_session();
  async_result<void> create_and_or_join_conference();
  async_result<void> set_audio_processing(bool off = true);
  async_result<void> leave_conference();
  async_result<void> close_session();
  async_result<void> apply_spatial_audio_configuration();

  // Conference Info helpers
  dolbyio::comms::conference_info conference_info() { return conf_info_; }
  void set_conference_info(const dolbyio::comms::conference_info& info);
  void update_conference_id(const std::string& id);
  dolbyio::comms::services::session::user_info session_info() const {
    return user_info_;
  }

 private:
  async_result<void> set_spatial_configuration(
      dolbyio::comms::spatial_audio_batch_update&& batch_update);
  void check_if_sdk_set();

  dolbyio::comms::sdk* sdk_{nullptr};
  command_line::sdk params_{};
  dolbyio::comms::conference_info conf_info_{};
  dolbyio::comms::services::session::user_info user_info_{};
  std::mutex sdk_lock_{};
};

};  // namespace dolbyio::comms::sample
