#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022-2023 by Dolby Laboratories.
 ***************************************************************************/

namespace dolbyio::comms {

class sdk;

namespace sample {

class commands_handler;

class interactor {
 public:
  virtual ~interactor() {}
  virtual void register_command_line_handlers(commands_handler& handler) = 0;
  virtual void register_interactive_commands(commands_handler& handler) = 0;
  virtual void set_sdk(dolbyio::comms::sdk* sdk) = 0;
};

}

}  // namespace dolbyio::comms::sample
