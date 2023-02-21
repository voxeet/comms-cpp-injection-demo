/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022-2023 by Dolby Laboratories.
 ***************************************************************************/

#include "wrappers/mediaio.h"

namespace dolbyio::comms::sample {

void media_io_wrapper::set_sdk(dolbyio::comms::sdk* sdk) {
  std::lock_guard<std::mutex> lock(sdk_lock_);
  if (!sdk && sdk_) {
    sdk_params_.video_frame_handler = nullptr;
    wait(sdk_->video().local().stop());
  }
  sdk_ = sdk;
}

void media_io_wrapper::initialize_injection() {
  if (!media_io_)
    throw std::runtime_error(
        "Attempting to initialize the injection while not requested");

  bool audio = params_.override_inject_audio_.value_or(
      sdk_params_.conf.join_with_audio());
  bool video = params_.override_inject_video_.value_or(
      sdk_params_.conf.join_with_video());

  if (!audio && !video) {
    std::cerr << "No injection requested for audio or video, the input file "
                 "will not be used"
              << std::endl;
    return;
  }

  if (!injector_) {
    injector_ = std::make_unique<plugin::injector_paced>(
        [](const dolbyio::comms::plugin::media_injection_status& state) {
          std::cerr << "Media Injection Status Change ===> type: "
                    << state.type_ << " state: " << state.state_
                    << " desc: " << state.description_ << std::endl;
        });
    if (video)
      sdk_params_.video_frame_handler = injector_.get();
  }
  if (!source_) {
    source_ = std::make_unique<file_source>(
        std::move(params_.files), params_.loop_the_injection_, *injector_,
        [this, audio,
         video](const dolbyio::comms::sample::file_source_status& status) {
          std::cerr << "File Source Status change\n";

          std::lock_guard<std::mutex> lock(sdk_lock_);
          if (sdk_) {
            if (status.current_state ==
                dolbyio::comms::sample::source_state::STOPPED) {
              if (audio)
                sdk_->audio()
                    .local()
                    .stop()
                    .then([]() { std::cerr << "audio stopped\n"; })
                    .on_error(
                        [](auto&&) { std::cerr << "Error stopping audio\n"; });
              if (video)
                sdk_->video()
                    .local()
                    .stop()
                    .then([]() { std::cerr << "video stopped\n"; })
                    .on_error(
                        [](auto&&) { std::cerr << "Error stopping video\n"; });
            }
          }
        });
    injector_->set_has_video_sink_cb(
        [this](bool has_sink) { source_->set_video_capture(has_sink); });
  }

  // Attach injector as audio/video source if that media is to be enabled
  if (audio)
    wait(sdk_->media_io().set_audio_source(injector_.get()));
  if (video)
    wait(sdk_->video().local().start(camera_device(), injector_.get()));
}

media_io_wrapper::~media_io_wrapper() {
  set_sdk(nullptr);
}

void media_io_wrapper::set_initial_capture(bool audio, bool video) {
  if (source_) {
    source_->set_audio_capture(audio);
    source_->set_video_capture(video);
  }
}

void media_io_wrapper::new_file(bool add) {
  std::cout << "file_name:" << std::endl;
  std::string fname;
  std::cin >> fname;
  if (add)
    source_->add_file_playlist(fname);
  else
    source_->play_new_file(fname);
}

void media_io_wrapper::seek_to_in_file() {
  std::cout << "Enter the seek to time:" << std::endl;
  try {
    std::string seek_str;
    std::cin >> seek_str;
    int seek_time;
    seek_time = std::stoi(seek_str);
    if (!source_->seek(seek_time))
      std::cerr << "Failed to Seek!\n";
  } catch (const std::exception& e) {
    std::cerr << e.what();
  }
}

void media_io_wrapper::register_command_line_handlers(
    commands_handler& handler) {
  // The media io switch sets the application to run as regular sending
  // video/audio from camera
  handler.add_command_line_switch(
      {"--enable-media-io", "-enable-media-io"},
      "\n\tUse Media IO capabilities (injection/recording).", [this]() {
        media_io_ = true;
        sdk_params_.conf.default_nonlistener_join = false;
        sdk_params_.conf.default_send_audio_video.audio = false;
        sdk_params_.conf.default_send_audio_video.video = false;
      });

  handler.add_command_line_switch(
      {"-f"}, "<file_name>\n\tMedia File (mp4) to inject into conference.",
      [this](const std::string& arg) {
        cmdline_config_touched_.append("-f ");
        params_.files.push_back(arg);
      });
  handler.add_command_line_switch(
      {"-d"},
      "<output_dir>\n\tOutput directory where the recorded media will be "
      "dumped (default: tmp)",
      [this](const std::string& arg) {
        cmdline_config_touched_.append("-d ");
        params_.output_dir = arg;
      });
  handler.add_command_line_switch({"-loop", "--loop"},
                                  "\n\tLoop the media injection", [this]() {
                                    cmdline_config_touched_.append("-loop ");
                                    params_.loop_the_injection_ = true;
                                  });

  handler.add_command_line_switch(
      {"-v"},
      "<video_format>\n\tVideo dump format: YUV, NONE, ENCODED, "
      "ENCODED_OPTIMIZED (default: ENCODED_OPTIMIZED)",
      [this](const std::string& arg) {
        cmdline_config_touched_.append("-v ");
        if (arg == "NONE")
          params_.vid_config =
              dolbyio::comms::plugin::recorder::video_recording_config::NONE;
        else if (arg == "YUV")
          params_.vid_config =
              dolbyio::comms::plugin::recorder::video_recording_config::YUV;
        else if (arg == "ENCODED")
          params_.vid_config =
              dolbyio::comms::plugin::recorder::video_recording_config::ENCODED;
        else if (arg == "ENCODED_OPTIMIZED")
          params_.vid_config = dolbyio::comms::plugin::recorder::
              video_recording_config::ENCODED_OPTIMIZED;
        else
          std::cerr << "Dumping video in ENCODED_OPTMIZED format" << std::endl;
      });

  handler.add_command_line_switch(
      {"-a"},
      "<audio_format>\n\tAudio dump format: AAC, NONE, PCM (default: PCM)",
      [this](const std::string& arg) {
        cmdline_config_touched_.append("-a ");
        if (arg == "NONE")
          params_.aud_config =
              dolbyio::comms::plugin::recorder::audio_recording_config::NONE;
        else if (arg == "AAC")
          params_.aud_config =
              dolbyio::comms::plugin::recorder::audio_recording_config::AAC;
        else if (arg == "PCM")
          params_.aud_config =
              dolbyio::comms::plugin::recorder::audio_recording_config::PCM;
        else
          std::cerr << "Invalid argument, dumping audio in PCM format"
                    << std::endl;
      });

  handler.add_command_line_switch(
      {"-iv"},
      "<true|false>\n\tOverride video injection setting (true - injecting "
      "video; false - not injecting video). The "
      "media_io will by default inject video if joining the conference with "
      "video, and will not inject otherwise.",
      [this](const std::string& arg) {
        cmdline_config_touched_.append("-iv ");
        if (arg == "true")
          params_.override_inject_video_ = true;
        else if (arg == "false")
          params_.override_inject_video_ = false;
        else
          std::cerr
              << "Invalid argument for the -iv option, using the defaults."
              << std::endl;
      });

  handler.add_command_line_switch(
      {"-ia"},
      "<true|false>\n\tOverride audio injection setting (true - injecting "
      "audio; false - "
      "not injecting audio). The media_io will by default inject audio if "
      "joining the "
      "conference with audio enabled, and will not inject otherwise.",
      [this](const std::string& arg) {
        cmdline_config_touched_.append("-ia ");
        if (arg == "true")
          params_.override_inject_audio_ = true;
        else if (arg == "false")
          params_.override_inject_audio_ = false;
        else
          std::cerr
              << "Invalid argument for the -ia option, using the defaults."
              << std::endl;
      });
}

void media_io_wrapper::register_interactive_commands(
    commands_handler& handler) {
  if (!media_io_) {
    if (!cmdline_config_touched_.empty()) {
      std::cerr << "The following command-line params will be ignored, because "
                   "--enable-media-io switch was not used: "
                << cmdline_config_touched_ << std::endl;
    }
    return;
  }

  handler.add_interactive_command(
      "stop-audio", "Stop audio injection",
      [this]() { source_->set_audio_capture(false); });
  handler.add_interactive_command(
      "start-audio", "Start audio injection",
      [this]() { source_->set_audio_capture(true); });
  handler.add_interactive_command("f", "set new file to play",
                                  [this]() { new_file(false); });
  handler.add_interactive_command("F", "add new file to playlist",
                                  [this]() { new_file(true); });
  handler.add_interactive_command("s", "seek to timestamp in file",
                                  [this]() { seek_to_in_file(); });
  handler.add_interactive_command(
      "r", "resume currently paused file", [this]() {
        if (!source_->resume())
          std::cerr << "Failed to perform Resume!\n";
      });
  handler.add_interactive_command("p", "pause currently play file", [this]() {
    if (!source_->pause())
      std::cerr << "Failed to perform Pause!\n";
  });
}

};  // namespace dolbyio::comms::sample
