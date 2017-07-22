#include "soda_listener.h"

int SoDaListener::put(const char * buf, int len)
{
  cmd_socket->write((char*) &len, sizeof(int)); 

  unsigned int to_go = len; 
  
  const char * cp = buf; 
  while(to_go > 0) {
    int l = cmd_socket->write(cp, to_go); 
    if(l < 0) return l; 
    else {
      to_go -= l; 
      cp += l; 
    }
  }

  cmd_socket->flush();  
  return len; 
}

int SoDaListener::get(char * buf, int maxlen)
{
  int len; 
  int stat = cmd_socket->read((char*) & len, sizeof(int));

  if(stat < 0) return stat; 
 
  if(len > maxlen) len = maxlen;

  unsigned int to_go = len; 
  
  char * cp = buf; 
  while(to_go > 0) {
    int l = cmd_socket->read(cp, to_go); 
    if(l < 0) return l; 
    else {
      to_go -= l; 
      cp += l; 
    }
  }

  return len; 
}

void SoDaListener::getCmd() {
  char buf[1024];
  std::cerr << "In getCmd\n";
  int len = get(buf, 1024);
  
  std::cerr << "read message of length " << len << " [" << buf << "]\n"; 
}

void SoDaListener::setModulation(const QString & modtype)
{
  std::cerr << boost::format("got a set modulation request [%s]\n") % modtype.toStdString();
}
