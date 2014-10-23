/*
  Copyright (c) 2014, Matthew H. Reilly (kb1vc)
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in
  the documentation and/or other materials provided with the
  distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COMMAND_INTERP_HDR
#define COMMAND_INTERP_HDR

#include <string>
#include <vector>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>

namespace SoDa {
  template<typename T> class Command {
  public:
    Command(const std::string & _short, const std::string & _long,
	    bool (T::* _func)(std::vector<std::string> & cmd_line)) {
      short_c = _short;
      long_c = _long;
      func = _func;
    }

    /**
     * @brief parse the command (that has been tokenized by the Command Interpreter object. 
     */
    bool parse(std::vector<std::string> & cmd_line, T * obj) {
      if((cmd_line[0] == short_c) || (cmd_line[0] == long_c)) {
	(obj->*func)(cmd_line); 
	return true; 
      }
      else return false; 
    }

  protected:
    std::string short_c; ///< the short version (one char, f'rinstance) of the command verb
    std::string long_c; ///< the long version (full name) of the command verb
    bool (T::* func)(std::vector<std::string> & cmd_line);
  };

  /**
   * Base class for command interpreter objects.
   *
   * A command interpreter binds a set of command strings to a set of methods
   * belonging to the interpreter. 
   */
  template <typename T> class CommandInterpreter {
  public:
    CommandInterpreter(T * _obj) {
      setHandlerObj(_obj);
    }

    CommandInterpreter() {
      obj = NULL;
    }

    void setHandlerObj(T * _obj) {
      obj = _obj; 
    }

    void makeCommand(const std::string & _short, const std::string & _long, 
		     bool (T::* _func)(std::vector<std::string> & cmd_line)) {
      commands.push_back(new Command<T>(_short, _long, _func));
    }

    bool parse(std::string command) {
      if(obj == NULL) return false; 
      // first tokenize the command line.
      boost::char_separator<char> sep(",= \t");
      boost::tokenizer< boost::char_separator<char> > tokens(command, sep);
      std::vector< std::string > cmd_line; 
      BOOST_FOREACH(const std::string & t, tokens) cmd_line.push_back(t); 

      // now go through the commands.
      BOOST_FOREACH(Command<T> * cmd, commands) {
	if(cmd->parse(cmd_line, obj)) return true;  
      }

      return false; 
    }

  protected:
    std::vector<Command<T> *> commands;
    T * obj; ///< the object that will handle all the commands. 
  };
}


#endif
