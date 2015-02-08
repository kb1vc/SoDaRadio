/*
  Copyright (c) 2015, Matthew H. Reilly (kb1vc)
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

#include "USRPSNACtrl.hxx"

SoDa::USRPSNACtrl::USRPSNACtrl(Params * _params, CmdMBox * _cmd_stream) : 
  SoDa::USRPCtrl(_params, _cmd_stream)
{
}


void SoDa::USRPSNACtrl::run()
{
  uhd::set_thread_priority_safe(); 
  // now do the event loop.  we watch
  // for commands and responses on the command stream.
  
  // do the initial commands
  cmd_stream->put(new Command(Command::SET, Command::RX_SAMP_RATE,
			     params->getRXRate())); 
  cmd_stream->put(new Command(Command::SET, Command::TX_SAMP_RATE,
			     params->getTXRate()));

  cmd_stream->put(new Command(Command::SET, Command::RX_ANT, 
			      "RX2")); 
  cmd_stream->put(new Command(Command::SET, Command::TX_ANT,
			      "TX/RX"));
  cmd_stream->put(new Command(Command::SET, Command::CLOCK_SOURCE,
			     params->getClockSource())); 

  cmd_stream->put(new Command(Command::SET, Command::TX_RF_GAIN, 0.0)); 
  cmd_stream->put(new Command(Command::SET, Command::RX_RF_GAIN, 0.0));

  // transmitter is off
  tx_on = false; 
  cmd_stream->put(new Command(Command::SET, Command::TX_STATE, 0)); 
  
  bool exitflag = false;
  unsigned int cmds_processed = 0;
  unsigned int loopcount = 0; 
  while(!exitflag) {
    loopcount++; 
    Command * cmd = cmd_stream->get(subid);
    // we don't wait long here -- we need to go through 100 tics per second
    if(cmd == NULL) {
      boost::this_thread::sleep(boost::posix_time::milliseconds(1));
    }
    else {
      // process the command.
      if((cmds_processed & 0xff) == 0) {
	debugMsg(boost::format("USRPSNACtrl processed %d commands") % cmds_processed);
      }
      cmds_processed++; 
      execCommand(cmd);
      exitflag |= (cmd->target == Command::STOP); 
      cmd_stream->free(cmd); 
    }
  }
}

void SoDa::USRPSNACtrl::doStep() 
{
  double current_time; 
  // the RX unit completed its collection, now we move on. 
  // set the TX freq
  // set the RX freq
  // find the time of the last command completion
  current_time = usrp->get_time_now().get_real_secs() + 100e-6;
  cmd_stream->put(new Command(Command::SET, Command::SNAI_SCAN_READY, 
			      current_time, current_time + time_per_step));
  sweep_state = WAIT_FOR_RX;    
  current_sweep_freq += step_sweep_freq; 

  return; 
}

void SoDa::USRPSNACtrl::execCommand(Command * cmd)
{
  switch (cmd->cmd) {
  case Command::GET:
    execGetCommand(cmd); 
    break;
  case Command::SET:
    execSetCommand(cmd); 
    break; 
  case Command::REP:
    execRepCommand(cmd); 
    break;
  default:
    break; 
  }
}


void SoDa::USRPSNACtrl::execSetCommand(Command * cmd)
{
  switch (cmd->target) {
  case Command::TX_STATE:
    // ignore this command -- but don't pass it on to USRPCtrl...
    break; 

  case Command::SNA_SCAN_START:
    start_sweep_freq = cmd->dparms[0];
    current_sweep_freq = start_sweep_freq; 
    end_sweep_freq = cmd->dparms[1];
    step_sweep_freq = cmd->dparms[2];
    time_per_step = cmd->dparms[3]; 
    doStep();
    break; 
  default:
    USRPCtrl::execSetCommand(cmd); 
    break; 
  }
}

void SoDa::USRPSNACtrl::execGetCommand(Command * cmd)
{
  switch (cmd->target) {
  default:
    USRPCtrl::execGetCommand(cmd); 
    break; 
  }
}

void SoDa::USRPSNACtrl::execRepCommand(Command * cmd)
{
  switch (cmd->target) {
  case Command::SNA_SCAN_REPORT:
    if(current_sweep_freq == end_sweep_freq) {
      // we just completed the last step in the sweep. 
      sweep_state = IDLE; 
      cmd_stream->put(new Command(Command::REP, Command::SNA_SCAN_END));
    }
    else {
      doStep();
    }
    break; 
  default:
    USRPCtrl::execRepCommand(cmd); 
    break; 
  }
}

