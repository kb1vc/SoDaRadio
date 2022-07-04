#pragma once
/*
Copyright (c) 2022 Matthew H. Reilly (kb1vc)
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

/**
 * @file MailBoxTypes.hxx Type definitions for the mailboxes 
 */
#include <SoDa/MailBox.hxx>
#include "Buffer.hxx"
#include "Command.hxx"

namespace SoDa {
  typedef MailBoxPtr<Command> MsgMBoxPtr;
  typedef MailBoxPtr<std::vector<std::complex<float>>> CFMBoxPtr;
  typedef MailBoxPtr<std::vector<std::complex<double>>> CDMBoxPtr;
  typedef MailBoxPtr<std::vector<float>> FMBoxPtr;
  typedef MailBoxPtr<std::vector<double>> DMBoxPtr; 

  typedef MailBox<Command> MsgMBox;
  typedef MailBox<std::vector<std::complex<float>>> CFMBox;
  typedef MailBox<std::vector<std::complex<double>>> CDMBox;
  typedef MailBox<std::vector<float>> FMBox;
  typedef MailBox<std::vector<double>> DMBox; 
  
  typedef MailBox<Command>::Subscription MsgSubs;
  typedef MailBox<std::vector<std::complex<float>>>::Subscription CFSubs;
  typedef MailBox<std::vector<std::complex<double>>>::Subscription CDSubs;
  typedef MailBox<std::vector<float>>::Subscription FSubs;
  typedef MailBox<std::vector<float>>::Subscription DSubs; 
  
}
