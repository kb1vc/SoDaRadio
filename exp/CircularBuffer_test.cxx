#include <boost/format.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/program_options.hpp>
#include "CircularBuffer.hxx"
#include <string>
#include <iostream>
#include <random>
#include <cstdlib>

int write_count = 0; 
int read_count = 0; 

size_t getCB(boost::circular_buffer<unsigned int> & cb, 
	     unsigned int * buf, size_t len) {
  size_t nret = 0;
  while(!cb.empty() && (len > 0)) {
    *buf = cb.front(); 
    cb.pop_front(); 
    buf++; 
    len--; 
    nret++; 
  }
  return nret;
}

size_t putCB(boost::circular_buffer<unsigned int> & cb, 
	     unsigned int * buf, size_t len) {
  size_t nret = 0; 
  while(len > 0) {
    cb.push_back(*buf); 
    buf++; 
    len--; 
    nret++; 
  }
  return nret; 
}

void doWrite(
	     SoDa::CircularBuffer<unsigned int> & test, 
	     boost::circular_buffer<unsigned int> & ref, 
	     std::uniform_int_distribution<unsigned int> & idist,
	     std::default_random_engine & gen)
{
  // pick a buffer len; 
  // make some random stuff; 
  unsigned int len = idist(gen); 
  unsigned int start = idist(gen); 
  unsigned int buf[len];

  
  for(unsigned int i = 0; i < len; i++) {
    buf[i] = write_count + i; 
  }

  write_count += len; 
  // std::cerr << boost::format("Writing %d elements total_write = %d total_read = %d\n") % len % write_count % read_count; 
  
  test.put(buf, (size_t) len); 
  putCB(ref, buf, len); 
}

void doRead(
	    SoDa::CircularBuffer<unsigned int> & test, 
	    boost::circular_buffer<unsigned int> & ref, 
	    std::uniform_int_distribution<unsigned int> & idist,
	    std::default_random_engine & gen)
{
  // pick a buffer len; 
  // make some random stuff; 
  
  unsigned int len = idist(gen); 
  unsigned int tbuf[len];
  unsigned int rbuf[len];

  // std::cerr << boost::format("Reading %d elements total_write = %d total_read = %d\n") % len % write_count % read_count; 
  
  size_t tlen = test.get(tbuf, (size_t) len); 
  size_t rlen = getCB(ref, rbuf, len); 

  read_count += rlen;   
  
  bool fail = false; 

  if(tlen != rlen) {
    std::cerr << boost::format("Attempted to retrieve %d elements, got %d from test, %d from ref\n") % len % tlen % rlen; 
    fail = true; 
  }

  for(unsigned int i = 0; i < rlen; i++) {
    if(tbuf[i] != rbuf[i]) {
      std::cerr << boost::format("mismatch in test test[%d] = %d  ref[%d] = %d\n")
	% i % tbuf[i] % i % rbuf[i];
      fail = true; 
    }
  }

  if(fail) {
    std::cerr << "Is now:\n"; 
    test.dump(std::cerr); 
    exit(-1);
  }
}


int main(int argc, char **argv)
{
  unsigned int blen;
  unsigned int num_trials;
  unsigned int len_range;
  bool print_progress; 
  namespace po = boost::program_options; 
  po::options_description opts("Test CircularBuffer class"); 
  opts.add_options()
    ("buffer_len", po::value<unsigned int>(&blen)->default_value(103), 
     "Length of circular buffer")
    ("trials", po::value<unsigned int>(&num_trials)->default_value(1000), 
     "Number of test loop iterations to run")
    ("max_xfer", po::value<unsigned int>(&len_range)->default_value(83), 
     "Maximum length of block to read/write")
    ("print_progress", po::value<bool>(&print_progress)->default_value(false));

  po::variables_map pmap; 
  po::store(po::parse_command_line(argc, argv, opts), pmap);
  po::notify(pmap);

  std::cerr << boost::format("buffer length = %d  trials = %d  allocation max %d\n")
    % blen % num_trials % len_range; 
  
  SoDa::CircularBuffer<unsigned int> test_cb(blen);
  boost::circular_buffer<unsigned int> ref_cb(blen);

  std::default_random_engine gen(3235123); 
  std::uniform_int_distribution<unsigned int> idist(1, len_range); 

  // now do some puts and gets, comparing each. 
  // first put some stuff to start the buffers
  doWrite(test_cb, ref_cb, idist, gen); 
  doWrite(test_cb, ref_cb, idist, gen);
  doWrite(test_cb, ref_cb, idist, gen); 
  
  for(int i = 0; i < num_trials; i++) {
    unsigned int ch = idist(gen); 
    if(print_progress && ((i % 10000) == 0)) std::cerr << boost::format("Trial number %d\n") % i;
    // if(i > 83) {
    //   test_cb.dump(std::cerr); 
    // }
    if((ch & 1) == 0) doWrite(test_cb, ref_cb, idist, gen);
    if((ch & 2) == 0) doRead(test_cb, ref_cb, idist, gen);
  }

  std::cerr << "Test passed!\n";
  exit(0);
}
