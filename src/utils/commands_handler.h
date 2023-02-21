#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022-2023 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/sdk.h>

#include "utils/interactor.h"

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>

namespace dolbyio::comms::sample {

// The commnds handler class is used to provide a robust way for our sample application to
// provide different command line and interactive switches for different parts of the SDK.
class commands_handler {
  using command = std::string;
  using commands = std::vector<std::string>;
  using command_arg = std::string;
  using alias = command;
  using description = std::string;
  using action = std::function<void()>;
  using action_with_arg = std::function<void(const command_arg&)>;

 public:
  struct batch {
    command cmd;
    description desc;
    action_with_arg act;
  };

  commands_handler();

  void add_interactor(std::shared_ptr<interactor> obj);

  void add_interactive_command(const command&, const description&, action);
  enum class mandatory { no, yes };
  void add_command_line_switch(const commands&, const description&, action);
  void add_command_line_switch(const commands&, const description&, action_with_arg, mandatory = mandatory::no);

  void print_interactive_options() const;

  void handle_interactive_command(const command&);
  void handle_command_line_option(const command&, const command_arg&);

  void set_sdk(dolbyio::comms::sdk* sdk);
  void parse_command_line(int argc, char** argv);

 private:
  struct command_line_switch {
    void handle(const command_arg&);

    description description_;
    action_with_arg action_;
    bool has_argument_{};
    bool is_mandatory_{};
    bool is_set_{};
    std::set<alias> aliases_{};  // useful to map both switches to aliases (for
                                 // displaying help)
  };

  enum class has_argument { no, yes };
  void add_command_line_switch(const commands&, const description&, action_with_arg, has_argument, mandatory);

  void show_help() const;
  void show_help_and_throw(const std::string& what) const;
  std::string get_full_switch_description(const command&) const;

  commands get_interactive_actions() const;

  void verify_new_switches(const commands&) const;
  command_line_switch& find_switch(const command&);
  void verify_all_mandatory_switches_set() const;

  bool enabled_ = false;
  std::vector<std::shared_ptr<interactor>> interactors_{};

  using description_and_action = std::pair<description, action>;
  std::map<command, std::vector<description_and_action>> interactive_actions_{};

  std::map<command, command_line_switch> command_line_switches_;
  std::map<alias, command> aliases_;  // and aliases to switches (for handling)
};

}  // namespace dolbyio::comms::sample
