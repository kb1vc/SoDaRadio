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

/**
 * @file DDC_Post.cxx
 *
 * @brief Post process the output of DDC_Test to convert the binary
 * file format into a more compact form (resampling the data) that can
 * be plotted using gnuplot
 *
 * @author Matt Reilly (kb1vc)
 */
#include <fstream>
#include <math.h>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options; 

class Params {
public:
  Params(int argc, char * argv[]) {
    init();
    parse(argc, argv); 
  }

private:
  boost::program_options::options_description * desc;
  
  void init() {
    binary_mode = false; 
    desc = new boost::program_options::options_description("Allowed options");
    desc->add_options()
      ("help", "help message")
      ("bucket", po::value<unsigned int>(&new_bucket_width)->default_value(4),
       "Aggregate <N> frequency bins into one")
      ("scanlines", po::value<unsigned int>(&scan_target)->default_value(4),
       "Aggregate <N> scan lines into one")
      ("in", po::value<std::string>(&in_filename)->default_value(std::string("sweep.bin")),
       "Input filename")
      ("out", po::value<std::string>(&out_filename)->default_value(std::string("sweep.pdat")),
       "Output filename")
      ("fmin", po::value<double>(&fddc_min)->default_value(-10e6),
       "Minimum ddc frequency")
      ("fmax", po::value<double>(&fddc_max)->default_value(10e6),
       "Maximum ddc frequency")
      ("binary", po::bool_switch(&binary_mode), "Select binary output")
       ;
  }

  void parse(int argc, char * argv[]) {
    po::store(po::parse_command_line(argc, argv, *desc), pmap);
    po::notify(pmap);

    if(pmap.count("help")) {
      std::cout << "DDC_Post -- post process binary output from DDC_Test"
		<< *desc << std::endl; 
      exit(-1); 
    }
  }

public:
  po::variables_map pmap;

  double fddc_min, fddc_max;
  
  unsigned int new_bucket_width;
  unsigned int scan_target; 
  std::string in_filename;
  std::string out_filename;
  bool binary_mode; 
};

int main(int argc, char * argv[])
{
  Params p(argc, argv);

  char bmode = p.binary_mode ? 'T' : 'F'; 
  std::cout << boost::format("b = %d in = %s out = %s bmode = %c\n")
    % p.new_bucket_width % p.in_filename % p.out_filename % bmode;

  std::ifstream inf(p.in_filename.c_str(), std::ios::binary | std::ios::in);
  std::ofstream outf;
  if(p.binary_mode) {
    outf.open(p.out_filename.c_str(), std::ios::binary | std::ios::out);
  }
  else {
    outf.open(p.out_filename.c_str(), std::ios::out);
  }

  if(!inf.is_open()) {
    std::cerr << boost::format("Failed to open infile = %s\n") % p.in_filename;
    exit(-1); 
  }
  
  double samp_freq = 625e3;
  float * inbuckets = NULL;
  float * sumbuckets = NULL;
  int inbucket_size = 0;
  int scan_count = 0;

  int loop_count = 0; 
  while(!inf.eof()) {
    double f_1stlo, f_ddc;
    int pts;
    inf.read((char*) &f_1stlo, sizeof(double));
    inf.read((char*) &f_ddc, sizeof(double));
    inf.read((char*) &pts, sizeof(int));

    if(pts > inbucket_size) {
      if(inbuckets != NULL) delete [] inbuckets;
      inbuckets = new float[pts];
      inbucket_size = pts;
      if(sumbuckets != NULL) delete [] sumbuckets;
      sumbuckets = new float[1 + pts / p.new_bucket_width];
      for(int i = 0; i < (1 + pts/p.new_bucket_width); i++) sumbuckets[i] = 0.0; 
    }

    if(inf.eof()) continue; 

    loop_count++;
    
    inf.read((char*) inbuckets, sizeof(float) * pts);

    if(inf.eof()) continue; 
    
    if((f_ddc < p.fddc_min) || (f_ddc > p.fddc_max)) continue;
    
    // now dump the buckets
    float bsum = 0.0;
    float bmax; 
    int bcount = 0;

    float start_freq = samp_freq * -0.5;
    double scale = 1.0 / ((double) p.new_bucket_width);

    int sbi = 0;
    int i;
    bsum = 0.0;
    bmax = 0.0; 
    // accumulate this scanline into the sumbuckets. 
    for(i = 0, sbi = 0; i < pts; i++) {
      bsum += inbuckets[i];
      if(inbuckets[i] > bmax) bmax = inbuckets[i]; 
      bcount++;
      if(bcount == p.new_bucket_width) {
	double bval = bsum * scale;
	// sumbuckets[sbi++] = bval;
	if(bmax > sumbuckets[sbi]) sumbuckets[sbi] = bmax;
	sbi++; 
	bcount = 0;
	bsum = 0.0;
	bmax = 0.0; 
      }
    }

    scan_count++; 
    // have we accumulated N scans worth ? 
    float fbucket = samp_freq / ((float) sbi);
    if(scan_count == p.scan_target) {
      for(int i = 0; i < sbi; i++) {
	float fft_freq = start_freq + ((float) i) * fbucket;
	sumbuckets[i] = 10.0 * log10(sumbuckets[i]);
	if(p.binary_mode) {
	  outf.write((char*) &f_ddc, sizeof(double));
	  outf.write((char*) &fft_freq, sizeof(float));
	  outf.write((char*) &(sumbuckets[i]), sizeof(float));
	  outf.flush();
	}
	else {
	  outf << boost::format("%g %g %g\n") % f_ddc % fft_freq % sumbuckets[i];
	  outf.flush();
	}
	sumbuckets[i] = 0.0; 
      }
      scan_count = 0;
    }
  }

  outf.close();
  inf.close(); 
}
