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
 * This program produces two types of output lines:
 *
 * ERMS __fDDC__ __rmsDeviation__ __spikeCount__
 * rmsDeviation is the RMS difference between the mean of the
 * spectrum (from -200kHz to 200kHz) and each bucket in dB.
 * spikeCount is the number of peaks above the specified spike_threshold
 *
 * and then a plot of actual power at each offset from the DDC
 * setting for the best DDC setting and the worst (in terms of spur count)
 * Low_Spur ddc_setting bucket_offset amp
 * 
 *
 * @author Matt Reilly (kb1vc)
 */


#include <fstream>
#include <math.h>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <list>

namespace po = boost::program_options; 

class Params {
public:
  Params(int argc, char * argv[]) {
    init();
    parse(argc, argv); 
  }

private:
  po::options_description * desc;
  po::positional_options_description * posopts;
  void init() {
    desc = new po::options_description("Allowed options");
    desc->add_options()
      ("help", "help message")
      ("in", po::value<std::string>(&in_filename)->default_value(std::string("sweep.bin")),
       "Input filename")
      ("out", po::value<std::string>(&out_filename)->default_value(std::string("sweep.pdat")),
       "Output filename")
      ("fmin", po::value<double>(&fddc_min)->default_value(-10e6),
       "Minimum ddc frequency")
      ("fmax", po::value<double>(&fddc_max)->default_value(10e6),
       "Maximum ddc frequency")
      ("spike", po::value<float>(&spike_threshold)->default_value(1.0),
       "threshold (in dB) defining a \"birdie\"")
       ;

    posopts = new po::positional_options_description;
    posopts->add("in",1);
    posopts->add("out", 1);
  }

  void parse(int argc, char * argv[]) {
    po::store(po::command_line_parser(argc, argv).options(*desc)
	      .positional(*posopts).run(), pmap);
    po::notify(pmap);

    if(pmap.count("help")) {
      std::cout << "DDC_Analyze -- post process binary output from DDC_Test "
		<< std::endl
		<< "\nDDC_Analyze [options] [input-file] [output-file]\n" << std::endl
		<< *desc <<  std::endl;
      exit(-1); 
    }
  }

public:
  po::variables_map pmap;

  double fddc_min, fddc_max;
  
  std::string in_filename;
  std::string out_filename;

  float spike_threshold;
};

template<typename Tk, typename Tv> bool compare_pair(const std::pair<Tk, Tv> & a,
				   const std::pair<Tk, Tv> & b)
{
  return a.second < b.second;
}
				   
void scanForRMS(Params & p, std::ifstream & inf, std::ofstream & outf)
{
  double samp_freq = 625e3;
  float * inbuckets = NULL;
  int inbucket_size = 0;
  std::map<double, float> result_map; 
  std::list<std::pair<double, int> > result_list; 
  int i; 

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
    }

    if(inf.eof()) continue; 

    inf.read((char*) inbuckets, sizeof(float) * pts);

    if(inf.eof()) continue; 
    
    if((f_ddc < p.fddc_min) || (f_ddc > p.fddc_max)) continue;

    // convert to dB
    for(i = 0; i < pts; i++) {
      inbuckets[i] = 10.0 * log10(inbuckets[i]);
    }
    
    // what is the mean squared and the mean?
    // Only look at the middle buckets.
    int sstart = 6 * pts / 16;
    int send = pts - sstart; 
    float sum, sumsq;
    sum = 0.0;
    for(i = sstart; i < send; i++) {
      sum += inbuckets[i]; 
    }

    float mean = sum / ((float) (send - sstart));

    // now what is the sum of the differences?
    // and how many are at least 1dB above the mean
    sum = 0.0;
    int spike_count = 0; 
    for(i = sstart; i < send; i++) {
      float diff = mean - inbuckets[i]; 
      sum += (diff * diff);
      if(diff > p.spike_threshold) spike_count++; 
    }

    float sdev = sqrt(sum / ((float) pts));

    result_map[f_ddc] = sdev; 
    result_list.push_back(std::pair<double, int>(f_ddc, spike_count)); 
    outf << boost::format("ERMS %g %g %d\n") % f_ddc % sdev % spike_count;
  }

  result_list.sort(compare_pair<double, int>);
  std::cout << boost::format("first = %g %d\n") % result_list.front().first % result_list.front().second;
  std::cout << boost::format("last = %g %d\n") % result_list.back().first % result_list.back().second;

  double t1 = result_list.front().first;
  double t2 = result_list.back().first;
  std::cout << boost::format("Looking for %g or %g\n") % t1 % t2; 
  // now sweep through and print the good stuff.
  inf.clear(std::ios::goodbit);
  inf.seekg(0, inf.beg);
  if(!inf.good()) {
    std::cerr << "Instream is in bad state\n";
    exit(-1);
  }
  
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
    }

    if(inf.eof()) continue; 

    inf.read((char*) inbuckets, sizeof(float) * pts);

    if(inf.eof()) continue; 

    if((f_ddc != t1) && (f_ddc != t2)) continue;

    std::cerr << "Got one! " << f_ddc << std::endl; 
    float fbucket = samp_freq / ((float) pts);
    float start_freq = samp_freq * -0.5;

    std::string tag; 
    if(f_ddc == t1) tag = "Low_Spur";
    else if(f_ddc == t2) tag = "Hi_Spur";
    else tag = "S?";

    for(i = 0; i < pts; i++) {
      outf << boost::format("%s %14.9g %14.9g %g\n")
	% tag % f_ddc % start_freq % inbuckets[i];
      start_freq += fbucket;
    }
  }  
}

int main(int argc, char * argv[])
{
  Params p(argc, argv);

  std::ifstream inf(p.in_filename.c_str(), std::ios::binary | std::ios::in);
  std::ofstream outf;
  outf.open(p.out_filename.c_str(), std::ios::out);

  if(!inf.is_open()) {
    std::cerr << boost::format("Failed to open infile = %s\n") % p.in_filename;
    exit(-1); 
  }

  scanForRMS(p, inf, outf); 
  
  outf.close();
  inf.close(); 
}
