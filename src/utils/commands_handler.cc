/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022-2023 by Dolby Laboratories.
 ***************************************************************************/

#include "utils/commands_handler.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace dolbyio::comms::sample {

commands_handler::commands_handler() {
  add_command_line_switch({"-h", "--help"}, "\n\tShow help and exit.",
                          [this]() {
                            show_help();
                            std::exit(EXIT_SUCCESS);
                          });
}

void commands_handler::add_interactor(std::shared_ptr<interactor> obj) {
  if (enabled_)
    throw std::runtime_error("SDK is already set");

  obj->register_command_line_handlers(*this);
  interactors_.push_back(std::move(obj));
}

void commands_handler::add_interactive_command(const command& command,
                                               const description& description,
                                               action action) {
  if (enabled_)
    throw std::runtime_error("SDK is already set");

  auto res = interactive_actions_.find(command);
  if (res == interactive_actions_.end())
    interactive_actions_.insert(std::make_pair(
        command, std::vector<description_and_action>{
                     std::make_pair(description, std::move(action))}));
  else
    res->second.push_back(std::make_pair(description, std::move(action)));
}

void commands_handler::add_command_line_switch(const commands& commands,
                                               const description& description,
                                               action action) {
  add_command_line_switch(
      commands, description,
      [a = std::move(action)](const command_arg&) { a(); }, has_argument::no,
      mandatory::no);
}

void commands_handler::add_command_line_switch(const commands& commands,
                                               const description& description,
                                               action_with_arg action,
                                               mandatory is_mandatory) {
  add_command_line_switch(commands, description, std::move(action),
                          has_argument::yes, is_mandatory);
}

void commands_handler::add_command_line_switch(const commands& commands,
                                               const description& description,
                                               action_with_arg action,
                                               has_argument has_argument,
                                               mandatory is_mandatory) {
  if (enabled_)
    throw std::runtime_error("SDK is already set");

  if (commands.empty())
    return;
  verify_new_switches(commands);

  // first command is the main switch
  auto it = commands.cbegin();
  const auto& sw_name = *it;
  command_line_switch sw_data{description, std::move(action),
                              has_argument == has_argument::yes,
                              is_mandatory == mandatory::yes};

  // all other commands are aliases
  for (++it; it != commands.cend(); ++it) {
    sw_data.aliases_.insert(*it);
    aliases_.emplace(*it, sw_name);
  }

  command_line_switches_.emplace(sw_name, std::move(sw_data));
}

commands_handler::command_line_switch& commands_handler::find_switch(
    const command& command) {
  auto cmd = command;

  const auto it_alias = aliases_.find(command);
  if (it_alias != aliases_.cend())
    cmd = it_alias->second;

  const auto it = command_line_switches_.find(cmd);
  if (it == command_line_switches_.end())
    show_help_and_throw("Invalid command line option: " + cmd);
  return it->second;
}

void commands_handler::print_interactive_options() const {
  std::cerr << "Possible interactive options are:\n";
  for (const auto& option : get_interactive_actions())
    std::cerr << "    " << option << std::endl;
}

void commands_handler::handle_interactive_command(const command& command) {
  if (!enabled_)
    return;  // no commands when SDK is not set

  auto it = interactive_actions_.find(command);
  if (it == interactive_actions_.end()) {
    std::cerr << "Unknown command: " << command << std::endl;
    return;
  }
  for (const auto& iter : it->second) {
    try {
      iter.second();
    } catch (const std::exception& ex) {
      std::cerr << "Command: " << command << " Failed: " << ex.what()
                << std::endl;
    }
  }
}

void commands_handler::handle_command_line_option(const command& option,
                                                  const command_arg& arg) {
  if (enabled_)
    throw std::runtime_error("SDK is already set");

  find_switch(option).handle(arg);
}

void commands_handler::set_sdk(sdk* sdk) {
  enabled_ = (sdk != nullptr);
  for (auto& interactor : interactors_)
    interactor->set_sdk(sdk);
}

void commands_handler::parse_command_line(int argc, char** argv) {
  if (enabled_)
    throw std::runtime_error("SDK is already set");

  for (int i = 1; i < argc; ++i) {
    const command cmd = argv[i];
    auto& sw = find_switch(cmd);
    if (sw.has_argument_ && ++i == argc)
      show_help_and_throw("No value provided for option: " + cmd);
    sw.handle(argv[i]);
  }

  verify_all_mandatory_switches_set();
  for (auto& obj : interactors_) {
    obj->register_interactive_commands(*this);
  }
}

void commands_handler::show_help() const {
  std::vector<std::string> mandatory_switches, optional_switches;
  for (const auto& s : command_line_switches_)
    (s.second.is_mandatory_ ? mandatory_switches : optional_switches)
        .push_back(get_full_switch_description(s.first));

  std::cerr << "Mandatory parameters:\n";
  for (const auto& option : mandatory_switches)
    std::cerr << "    " << option << std::endl;
  std::cerr << "Optional parameters:\n";
  for (const auto& option : optional_switches)
    std::cerr << "    " << option << std::endl;
}

void commands_handler::show_help_and_throw(const std::string& what) const {
  show_help();
  throw std::runtime_error{what};
}

std::string commands_handler::get_full_switch_description(
    const command& command) const {
  std::ostringstream oss;

  const auto& s = command_line_switches_.at(command);
  oss << command;
  for (const auto& alias : s.aliases_)
    oss << ", " << alias;
  oss << " " << s.description_;

  return oss.str();
}

commands_handler::commands commands_handler::get_interactive_actions() const {
  commands actions{};
  for (const auto& a : interactive_actions_)
    for (const auto& vec : a.second)
      actions.push_back(a.first + " - " + vec.first);
  return actions;
}

void commands_handler::verify_new_switches(const commands& commands) const {
  std::set<command> cmds_uniq;
  for (const auto& cmd : commands)
    if (!cmds_uniq.insert(cmd).second || command_line_switches_.count(cmd) ||
        aliases_.count(cmd))
      throw std::runtime_error{"Duplicate definition of command line switch " +
                               cmd};
}

void commands_handler::verify_all_mandatory_switches_set() const {
  std::vector<command> unset_switches;
  for (const auto& s : command_line_switches_)
    if (s.second.is_mandatory_ && !s.second.is_set_)
      unset_switches.push_back(s.first);

  if (unset_switches.empty())
    return;

  std::ostringstream oss;
  for (const auto& s : unset_switches)
    oss << "\n    " << get_full_switch_description(s);
  show_help_and_throw("The following mandatory arguments were not set:" +
                      oss.str());
}

void commands_handler::command_line_switch::handle(const command_arg& arg) {
  action_(arg);
  is_set_ = true;
}

}  // namespace dolbyio::comms::sample
