/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022-2023 by Dolby Laboratories.
 ***************************************************************************/

#include "wrappers/sdk.h"
#include "utils/commands_handler.h"

#include <sstream>

namespace dolbyio::comms::sample {

sdk_wrapper::~sdk_wrapper() {
  sdk_wrapper::set_sdk(nullptr);
}

void sdk_wrapper::check_if_sdk_set() {
  if (!sdk_)
    throw dolbyio::comms::async_operation_canceled("SDK has not been set!");
}

async_result<void> sdk_wrapper::open_session() {
  check_if_sdk_set();

  return sdk_->session().open(session_options()).then([this](auto&& info) {
    std::lock_guard<std::mutex> lock(sdk_lock_);
    user_info_ = std::move(info);
  });
}

async_result<void> sdk_wrapper::create_and_or_join_conference() {
  check_if_sdk_set();

  if (get_params().conf.id) {
    update_conference_id(get_params().conf.id.value());
    return sdk_->conference()
        .join(conference_info(), join_options())
        .then([this](auto&& info) { set_conference_info(info); });
  } else if (get_params().conf.alias) {
    if (get_params().conf.alias->compare("DEMO") == 0) {
      return sdk_->conference()
          .demo(conference_options().params.spatial_audio_style)
          .then([this](auto&& info) { set_conference_info(info); });
    } else {
      return sdk_->conference()
          .create(conference_options())
          .then([this](auto&& info) {
            set_conference_info(info);
            if (get_params().conf.join_as_user())
              return sdk_->conference().join(conference_info(), join_options());
            else
              return sdk_->conference().listen(conference_info(),
                                               listen_options());
          })
          .then([this](auto&& info) { set_conference_info(info); });
    }
  }
  throw dolbyio::comms::exception("Neither conference ID nor Alias set!");
}

async_result<void> sdk_wrapper::set_audio_processing(bool off) {
  check_if_sdk_set();

  if (off) {
    return sdk_->audio().local().set_capture_mode(
        dolbyio::comms::audio_capture_mode::unprocessed());
  } else {
    return sdk_->audio().local().set_capture_mode(
        dolbyio::comms::audio_capture_mode::standard{noise_reduction::high});
  }
}

async_result<void> sdk_wrapper::leave_conference() {
  check_if_sdk_set();
  return sdk_->conference().leave();
}

async_result<void> sdk_wrapper::close_session() {
  check_if_sdk_set();
  return sdk_->session().close();
}

async_result<void> sdk_wrapper::set_spatial_configuration(
    dolbyio::comms::spatial_audio_batch_update&& batch_update) {
  check_if_sdk_set();

  return sdk_->conference().update_spatial_audio_configuration(
      std::move(batch_update));
}

void sdk_wrapper::register_command_line_handlers(commands_handler& handler) {
  handler.add_command_line_switch(
      {"-u", "--user_name"}, "<name>\n\tUser name to use in conferences.",
      [this](const std::string& arg) { params_.user_name = arg; },
      commands_handler::mandatory::yes);

  handler.add_command_line_switch(
      {"-e"}, "<id>\n\tUser external ID.",
      [this](const std::string& arg) { params_.external_id = arg; });

  handler.add_command_line_switch(
      {"-k"},
      "<token>\n\tAccess token required to connect to the DolbyIo backend.",
      [this](const std::string& arg) { params_.access_token = arg; },
      commands_handler::mandatory::yes);

  handler.add_command_line_switch(
      {"-l"},
      "[0..5]\n\tC++ SDK logging level (0=off, 1=error, 2=warning, 3=info, "
      "4=debug, 5=verbose; default: 3).",
      [this](const std::string& arg) {
        const auto ll = command_line::to_int(arg, "-l");
        if (ll < static_cast<int>(dolbyio::comms::log_level::OFF) ||
            ll > static_cast<int>(dolbyio::comms::log_level::VERBOSE))
          command_line::throw_bad_args_error("-l", arg);
        params_.sdk_log_level = static_cast<dolbyio::comms::log_level>(ll);
      });

  handler.add_command_line_switch(
      {"-ml"},
      "[0..5]\n\tMedia Engine logging level (0=off, 1=error, 2=warning, "
      "3=info, 4=debug, 5=verbose; default: 0)",
      [this](const std::string& arg) {
        const auto ll = command_line::to_int(arg, "-ml");
        if (ll < static_cast<int>(dolbyio::comms::log_level::OFF) ||
            ll > static_cast<int>(dolbyio::comms::log_level::VERBOSE))
          command_line::throw_bad_args_error("-ml", arg);
        params_.me_log_level = static_cast<dolbyio::comms::log_level>(ll);
      });

  handler.add_command_line_switch(
      {"-ld", "--log_dir"}, "<dir>\n\tLog to file in directory.",
      [this](const std::string& arg) { params_.log_dir = arg; });

  handler.add_command_line_switch(
      {"-i"},
      "<id>\n\tJoin conference with ID (no conference creation attempt).",
      [this](const std::string& arg) { params_.conf.id = arg; });

  handler.add_command_line_switch(
      {"-c"},
      "<alias>\n\tJoin conference with alias (create if no such "
      "conference).\n\tUse -c DEMO to create and join a demo conference.",
      [this](const std::string& arg) { params_.conf.alias = arg; });

  handler.add_command_line_switch(
      {"-t"}, "<token>\n\tCAT token.",
      [this](const std::string& arg) { params_.conf.cat = arg; });

  handler.add_command_line_switch(
      {"-m"},
      "[AV|A|V]\n\tInitial send media enabled "
      "(AV=audio+video, A=audio, V=video).",
      [this](const std::string& arg) {
        command_line::sdk::conf::audio_video av{};
        av.audio = arg.find('A') != std::string::npos;
        av.video = arg.find('V') != std::string::npos;
        params_.conf.send_audio_video = av;
      });

  handler.add_command_line_switch(
      {"-opus", "--opus"},
      "\n\tCreate an opus conference. Remember spatial audio only works with "
      "Dolby Voice.",
      [this]() { params_.conf.dolby_voice = false; });

  handler.add_command_line_switch({"-s", "--send_only"},
                                  "\n\tJoin as send-only user.",
                                  [this]() { params_.conf.send_only = true; });

  handler.add_command_line_switch(
      {"-p"},
      "[user|listener|rts-listener]\n\tParticipant type (user=active, "
      "listener=inactive, rts-listener=RTS listener"
      "default: listener)",
      [this](const std::string& arg) {
        if (arg == "user")
          params_.conf.nonlistener_join = true;
        else if (arg == "listener")
          params_.conf.nonlistener_join = false;
        else
          command_line::throw_bad_args_error("-p", arg);
      });

  handler.add_command_line_switch(
      {"-spatial"},
      "[shared|individual|disabled]\n\tEnable spatial audio (default: "
      "disabled).",
      [this](const std::string& arg) {
        if (arg == "shared")
          params_.conf.spatial = spatial_audio_style::shared;
        else if (arg == "individual")
          params_.conf.spatial = spatial_audio_style::individual;
        else if (arg == "disabled")
          params_.conf.spatial = spatial_audio_style::disabled;
        else
          command_line::throw_bad_args_error("-spatial", arg);
      });

  handler.add_command_line_switch(
      {"-initial-yaw-rotation"}, "The yaw rotation angle 0-360 degrees",
      [this](const std::string& arg) {
        auto yaw = command_line::to_double(arg, "initial-yaw-rotation");
        params_.conf.initial_spatial_direction =
            dolbyio::comms::spatial_direction{0, yaw, 0};
      });

  handler.add_command_line_switch(
      {"-initial-spatial-position"},
      "<x;y;z>\n\tInitial spatial position in shared scene, default \"0;0;0\". "
      "Note: the separator between x, y and z "
      "is semicolon, and the argument should be passes in quotation marks.",
      [this](const std::string& arg) {
        auto pos1 = arg.find(';');
        if (pos1 == arg.npos) {
          command_line::throw_bad_args_error("-initial-spatial-position", arg);
        }
        auto pos2 = arg.find(';', pos1 + 1);
        if (pos2 == arg.npos) {
          command_line::throw_bad_args_error("-initial-spatial-position", arg);
        }
        auto x_str = arg.substr(0, pos1);
        auto y_str = arg.substr(pos1 + 1, pos2 - pos1 - 1);
        auto z_str = arg.substr(pos2 + 1);
        auto x = std::stod(x_str);
        auto y = std::stod(y_str);
        auto z = std::stod(z_str);
        params_.conf.initial_spatial_position =
            dolbyio::comms::spatial_position{x, y, z};
      });
}

void sdk_wrapper::register_interactive_commands(commands_handler& handler) {}

dolbyio::comms::services::session::user_info sdk_wrapper::session_options()
    const {
  dolbyio::comms::services::session::user_info participant{};
  participant.externalId = params_.external_id;
  participant.name = params_.user_name;
  return participant;
}

dolbyio::comms::services::conference::conference_options
sdk_wrapper::conference_options() const {
  dolbyio::comms::services::conference::conference_options create{};
  create.alias = params_.conf.alias;
  create.params.spatial_audio_style = params_.conf.spatial;
  create.params.dolby_voice = params_.conf.dolby_voice;
  return create;
}

dolbyio::comms::services::conference::join_options sdk_wrapper::join_options()
    const {
  dolbyio::comms::services::conference::join_options join{};
  join.constraints.audio = params_.conf.join_with_audio();
  join.constraints.video = params_.conf.join_with_video();
  join.constraints.send_only = params_.conf.send_only;
  join.connection.conference_access_token = params_.conf.cat;
  join.connection.spatial_audio =
      (params_.conf.spatial != spatial_audio_style::disabled);
  join.connection.simulcast = params_.conf.simulcast;
  return join;
}

dolbyio::comms::services::conference::listen_options
sdk_wrapper::listen_options() const {
  dolbyio::comms::services::conference::listen_options listen{};
  listen.connection.conference_access_token = params_.conf.cat;
  listen.connection.spatial_audio =
      (params_.conf.spatial != spatial_audio_style::disabled);
  listen.connection.simulcast = params_.conf.simulcast;
  return listen;
}

void sdk_wrapper::update_conference_id(const std::string& id) {
  std::lock_guard<std::mutex> lock(sdk_lock_);
  conf_info_.id = id;
}

void sdk_wrapper::set_conference_info(
    const dolbyio::comms::conference_info& info) {
  std::lock_guard<std::mutex> lock(sdk_lock_);
  conf_info_ = info;
}

};  // namespace dolbyio::comms::sample
