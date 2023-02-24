#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022-2023 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/multimedia_streaming/injector.h>
#include <dolbyio/comms/multimedia_streaming/recorder.h>
#include <dolbyio/comms/sdk.h>

#include <iostream>
#include <optional>
#include <string>

namespace dolbyio::comms::sample {

namespace command_line {
void throw_bad_args_error(const char* option, const std::string& value);
int to_int(const std::string& value, const char* option);
double to_double(const std::string& value, const char* option);

struct sdk {
  std::string access_token{};
  dolbyio::comms::log_level sdk_log_level{log_level::INFO};
  dolbyio::comms::log_level me_log_level{log_level::OFF};
  std::string log_dir{};
  std::string user_name{};
  std::string external_id{};

  struct conf {
    std::optional<std::string> alias;
    std::optional<std::string> cat;
    std::optional<std::string> id;

    std::optional<bool> nonlistener_join{};
    bool default_nonlistener_join{true};

    struct audio_video {
      bool audio{true};
      bool video{true};
    };
    std::optional<audio_video> send_audio_video{};
    audio_video default_send_audio_video{};

    bool dolby_voice{true};
    bool send_only{false};
    bool simulcast{false};
    spatial_audio_style spatial{spatial_audio_style::shared};
    spatial_position initial_spatial_position{0, 0, 0};
    spatial_direction initial_spatial_direction{0, 0, 0};
    spatial_scale initial_scale{5, 5, 5};
    spatial_position initial_right{1, 0, 0};
    spatial_position initial_up{0, 1, 0};
    spatial_position initial_forward{0, 0, -1};

    bool join_as_user() const {
      return nonlistener_join.value_or(default_nonlistener_join);
    }
    bool join_with_audio() const {
      return send_audio_video.value_or(default_send_audio_video).audio;
    }
    bool join_with_video() const {
      return send_audio_video.value_or(default_send_audio_video).video;
    }
  };
  conf conf;
  dolbyio::comms::video_frame_handler* video_frame_handler = nullptr;
};

struct mediaio {
  std::vector<std::string> files;
  std::string output_dir{"tmp"};
  dolbyio::comms::plugin::recorder::video_recording_config vid_config{
      dolbyio::comms::plugin::recorder::video_recording_config::
          ENCODED_OPTIMIZED};
  dolbyio::comms::plugin::recorder::audio_recording_config aud_config{
      dolbyio::comms::plugin::recorder::audio_recording_config::PCM};
  std::optional<bool> override_inject_audio_{};
  std::optional<bool> override_inject_video_{};
  bool loop_the_injection_{false};
};
}  // namespace command_line
}  // namespace dolbyio::comms::sample
