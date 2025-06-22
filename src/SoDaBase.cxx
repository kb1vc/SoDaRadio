/*
  Copyright (c) 2014, 2025 Matthew H. Reilly (kb1vc)
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

#include "SoDaBase.hxx"
#include <string>
#include <map>

namespace SoDa {
  std::map<std::string, BasePtr> Base::object_directory; 

  Base::Base(const std::string & oname)
  {
    objname = oname;
  }

  void Base::registerSelf(BasePtr me) {
    if(object_directory.find(objname) == object_directory.end()) {
      object_directory[objname] = me;
    }
    
    self = me; 
  }
  
  BasePtr Base::findSoDaObject(const std::string & oname) {
    std::map<std::string, BasePtr>::iterator mi;
    mi = object_directory.find(oname);
    if(mi != object_directory.end()) {
      return mi->second;
    }
    else {
      return NULL; 
    }
  }

  double Base::getTime() {
    struct timespec tp; 
    clock_gettime(CLOCK_MONOTONIC, &tp); // 60nS average in tight loops, 160nS cold.
    double ret = ((double) tp.tv_sec) + (1.0e-9 * ((double) tp.tv_nsec)); 
    if(first_time) {
      base_first_time = ret; 
      first_time = false; 
    }
    return ret - base_first_time; 
  }


  bool Base::first_time = true;
  double Base::base_first_time;

  Buf::Buf(unsigned int size) : r_size(size) {
      cdat.resize(0); // both vectors are resized to 0 at the start.
      fdat.resize(0); 
  }

  BufPtr Buf::make(unsigned int _size) {
    return std::make_shared<Buf>(_size); 
  }


  unsigned int Buf::size() { 
    if(cdat.size() > 0) return cdat.size();
    if(fdat.size() > 0) return fdat.size();
    else return r_size; 
  }
      
  void Buf::copy(BufPtr src) {
      cdat = src->cdat;
      fdat = src->fdat; 
      r_size = src->r_size;
  }
    
    /**
     * set the length of the buffer (in number of complex floats.)
     * @param nl new length
     */
  bool Buf::setComplexLen(unsigned int nl) {
    if(nl > cdat.size()) {
	cdat.resize(nl);
	return true; 
      }
      else return false; 
    }
    
    /**
     * set the length of the buffer (in number of floats.)
     * @param nl new length
     */
  bool Buf::setFloatLen(unsigned int nl) {
    if(nl > fdat.size()) {
      cdat.resize(nl);
      return true; 
    }
    else return false; 
  }

    /**
     * Return the reference to the storage buffer of complex floats
     *
     * Note that this is a reference.  Take care as to how it is consumed.
     *
     * ~~~~
     *     std::vector<std::complex<float>> foo = bp->getComplexBuf();
     * ~~~~
     *
     * will cause a *copy* to be made of the complex buffer. To get what you
     * probably want, you should do this:
     * ~~~~
     *     std::vector<std::complex<float>> & foo = bp->getComplexBuf();
     * ~~~~     
     *     
     * 
     */
  std::vector<std::complex<float>> & Buf::getComplexBuf() { 
      if(cdat.size() == 0) cdat.resize(r_size);
      return cdat; 
    }
    /**
     * Return the reference to the storage buffer of floats
     * ~~~~
     *     std::vector<float> foo = bp->getFloatBuf();
     * ~~~~
     *
     * will cause a *copy* to be made of the complex buffer. To get what you
     * probably want, you should do this:
     * ~~~~
     *     std::vector<float> & foo = bp->getFloatBuf();
     * ~~~~     
     */
  std::vector<float> & Buf::getFloatBuf() { 
      if(fdat.size() == 0) fdat.resize(r_size);    
      return fdat;
    }

  FBuf::FBuf(unsigned int size) : Buf(size) {
    fdat.resize(size);
    r_size = size; 
  }

  FBufPtr FBuf::make(unsigned int _size) {
    return std::shared_ptr<FBuf>(new FBuf(_size));
  }

  bool FBuf::setComplexLen(unsigned int nl) {
    throw Radio::Exception("Float buffer (FBuf) received getComplexBuf() request.");    
  }

  std::vector<std::complex<float>> & FBuf::getComplexBuf() {
    throw Radio::Exception("Float buffer (FBuf) received getComplexBuf() request.");
  }
  
  float & FBuf::operator[](size_t index) {
    if(index < fdat.size()) {
      return fdat[index];
    }
    else {
      throw Radio::Exception(SoDa::Format("Float buffer (FBuf) index %0 is out of range (>= %1)")
			     .addI(index).addI(fdat.size()). str());
    }
  }


  CBuf::CBuf(unsigned int size) : Buf(size) {
    cdat.resize(size);
    r_size = size; 
  }

  CBufPtr CBuf::make(unsigned int _size) {
    return std::shared_ptr<CBuf>(new CBuf(_size));
  }

  bool CBuf::setFloatLen(unsigned int nl) {
    throw Radio::Exception("Float buffer (CBuf) received getComplexBuf() request.");    
  }

  std::vector<float> & CBuf::getFloatBuf() {
    throw Radio::Exception("Float buffer (CBuf) received getComplexBuf() request.");
  }
  
  std::complex<float> & CBuf::operator[](size_t index) {
    if(index < cdat.size()) {
      return cdat[index];
    }
    else {
      throw Radio::Exception(SoDa::Format("Complex Float buffer (CBuf) index %0 is out of range (>= %1)")
			     .addI(index).addI(cdat.size()). str());
    }
  }

}
