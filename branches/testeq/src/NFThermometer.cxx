/*
  Copyright (c) 2012, Matthew H. Reilly (kb1vc)
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

#include "NFThermometer.hxx"
#include <list>

SoDa::NFThermometer::NFThermometer(Params * params, CmdMBox * _cmd_stream) :
  SoDa::SoDaThread("NFThermometer")
{
  cmd_stream = _cmd_stream;
  cmd_subs = cmd_stream->subscribe();

  // noise figure tools -- the source block thermometers
  try {
    // open the device.
    therm_port = new boost::asio::serial_port(ioser, "/dev/ttyACM0");
  
    // set the baud rate and such
    therm_port->set_option(boost::asio::serial_port_base::baud_rate(9600));
    therm_port->set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none)); // none
    therm_port->set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none)); // none
    therm_port->set_option(boost::asio::serial_port_base::character_size(8));
    therm_port->set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));

    therm_open = true;
    std::cerr << "NFTherm opened thermometer." << std::endl; 
  }
  catch ( const std::exception & e) {
    std::cerr << " Problem opening thermometer device: "
	      << "/dev/ttyACM0" << " "
	      << e.what() << " Thermometer is now disabled." << std::endl; 
    therm_open = false; 
  }

}

void SoDa::NFThermometer::run()
{
  bool exitflag = false;
  Command * cmd;
  usleep(1000000);
  std::cerr << "NFTherm started loop." << std::endl;   
  while(!exitflag) {
    while((cmd = cmd_stream->get(cmd_subs)) != NULL) {
      // process the command.
      execCommand(cmd);
      exitflag |= (cmd->target == Command::STOP);
      //      std::cerr << "NFThermometer got a message. target = " << cmd->target << std::endl; 
      cmd_stream->free(cmd);
    }

    // now get a temperature
    if(therm_open) {
      // std::cerr << "NFTherm getting temp." << std::endl;   
      char sp[] = " \n";
      char buf[256];
      boost::asio::write(*therm_port, boost::asio::buffer(sp, 2));
      usleep(100000);
      size_t len;
      int cp = 2;
      buf[0] = '!';
      buf[1] = ' ';
      while (1) {
	len = boost::asio::read(*therm_port, boost::asio::buffer(buf + cp, 1));
	if(buf[cp] == '\n') {
	  buf[cp] = '\000';
	  break;
	}
	cp++; 	 
      }
      // put the temperature out on the report bus
      cmd_stream->put(new SoDa::Command(Command::REP, Command::NF_THERM, buf));       
      // std::cerr << boost::format("NFTHERM: [%s] \n") % buf; 
    }
    
    usleep(1000);
  }
}

void SoDa::NFThermometer::execGetCommand(Command * cmd)
{
  return; 
}

void SoDa::NFThermometer::execSetCommand(Command * cmd)
{
  return; 
}

void SoDa::NFThermometer::execRepCommand(Command * cmd)
{
  return;
}
