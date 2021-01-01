#include "Format.hxx"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <ctype.h>
#include <regex>

namespace SoDa {
  char Format::separator = '.';
  
  Format::Format(const std::string & fmt_string) : 
    orig_fmt_string(fmt_string), cur_arg_number(0) {

    // turn all %% pairs into % and mark the position as *not* being the start
    // of an insertion marker. 
    initialScan(); 
  }


  Format & Format::addI(int v, unsigned int w) {
    std::stringstream ss;
    if(w != 0) {
      ss << std::setw(w); 
    }
    ss << v; 
    insertField(ss.str()); 
    return * this; 
  }

  Format & Format::addU(unsigned int v, unsigned int w) {
    std::stringstream ss;
    if(w != 0) {
      ss << std::setw(w); 
    }
    ss << v; 
    insertField(ss.str()); 
    return *this; 
  }

  Format & Format::addF(double v, char fmt, unsigned int width, unsigned int frac_precision) {
    std::stringstream ss;    
    switch (fmt) {
    case 'f':
      // fixed floating point format
      if(width) ss << std::setw(width); 
      ss << std::fixed << std::setprecision(frac_precision) << v;  
      break; 
    case 's':
      // scientific (who cares what the exponent is? format)
      if(width) ss << std::setw(width);       
      ss << std::scientific << std::setprecision(frac_precision) << v;
      break; 
    case 'g':
      // general (who cares what the exponent is format, or how this looks)
      ss.unsetf(std::ios::fixed | std::ios::scientific);
      if(width) ss << std::setw(width);             
      ss << std::setprecision(frac_precision) << v;  				    
      break; 
    case 'e':
      // now this is a tough one.
      // the object is to print this number as xxx.fffeN where N is a multiple of 3.
      // (engineering notation).  It is beyond me how C and C++ have continued to print
      // crap floating point formats that are only suited to imperial units (black and
      // white TV) or astronomers.  It is 2021 -- time to use the metric system, even
      // in backwaters that still cling to the 16th century.

      {
	// get the sign
	bool is_neg = (v < 0.0);
	double av = fabs(v); 
	// first find the log base 10.	
	double log_10 = log10(av);
	// now remember the sign
	double exp_sign_cor = (log_10 < 0.0) ? -1.0 : 1.0; 
	// truncate it toward 0
	log_10 = floor(fabs(log_10));
	double scale = pow(10, -1.0 * exp_sign_cor * log_10);
	// scale the "mantissa" part
	double mant = av * scale; 
	// now get the integer version of the exponent
	int ilog_10 = rint(log_10);
	// now we want to adjust ilog_10 until it is a multiple of 3
	// we're always going to *decrease* the exponent, so we should
	// always multiply the mantissa by 10
	ilog_10 = ilog_10 * ((exp_sign_cor < 0.0) ? -1 : 1);
	
	while(((ilog_10 % 3) != 0) || (mant < 1.0)) {
	  mant = 10.0 * mant;
	  ilog_10 -= 1; 
	}
	// now we have a mantissa in the range 1 to 999.99...
	int whole = rint(floor(mant));
	int frac = rint((mant - floor(mant)) * pow(10.0, frac_precision));
	ss << (is_neg ? '-' : ' ') << std::setw(3) << whole << separator << std::setfill('0') << std::setw(frac_precision) << frac << 'e' << ilog_10 ; 
      }
    }

    insertField(ss.str());
    return *this;     
  }

  Format & Format::addS(const std::string & v, unsigned int width) {
    std::stringstream ss;
    if(width > 0) {
      ss << std::setw(width);
    }
    ss << v; 
    insertField(ss.str()); 
    return *this;     
  }

  Format & Format::addC(char c) {
    std::stringstream ss;
    ss << c; 
    insertField(ss.str());
    return *this;
  }

  void Format::insertField(const std::string & s) {
    // scan the format string to find all the instances of "%n" where n is the current argument number.
    if(cur_arg_number > max_field_num) {
      std::stringstream ss;
      ss << "Too many arguments (" << (cur_arg_number + 1) << ") passed to Format.";
      throw BadFormat(ss.str(), *this);
    }

    // build a regular expression from the pattern number
    std::stringstream fpat;
    fpat << "%" << cur_arg_number << "\\D";  
    std::smatch m;
    std::regex re(fpat.str());
    int pattern_length = fpat.str().size() - 2; 

    std::list<int> match_positions;
    for(auto sri  = std::sregex_iterator(fmt_string.begin(), fmt_string.end(), re); 
	sri != std::sregex_iterator(); 
	++sri) {
      std::smatch m = *sri; 
      int fpos = m.position();
      
      if(find(escape_positions.begin(), escape_positions.end(), fpos) == escape_positions.end()) {
	// insert the field.
	match_positions.push_front(fpos); 
      }
    }

    for(auto p : match_positions) {
      fmt_string.replace(p, pattern_length, s); 
    }
    cur_arg_number++;
  }

  void Format::initialScan() {
    int i, j;
    max_field_num = -1; 
    for(i = 0, j = 0; i < (orig_fmt_string.size() - 1); i++, j++) {
      if(orig_fmt_string[i] == '%') {
	if(orig_fmt_string[i+1] == '%') {
	  escape_positions.push_back(fmt_string.size()); // This character in the fmt string is a literal '%'
	  fmt_string.push_back('%');
	  i++; // bump I an extra bit. 
	}
	else if(isdigit(orig_fmt_string[i+1])) {
	  int fnum = atoi(orig_fmt_string.substr(i+1).c_str());
	  if(fnum > max_field_num) max_field_num = fnum;
	  fmt_string.push_back('%');	  
	  //	  fmt_string.push_back(orig_fmt_string[i+1]);
	}
      }
      else {
	fmt_string.push_back(orig_fmt_string[i]);
      }
    }
    fmt_string.push_back(orig_fmt_string[orig_fmt_string.size() - 1]);
  }
  
  Format & Format::reset() {
    fmt_string = orig_fmt_string;
    cur_arg_number = 0; 
    return *this;
  }
  
  const std::string & Format::str(bool check_for_filled_out) const {
    return fmt_string; 
  }
}

std::ostream & operator<<(std::ostream & os, const SoDa::Format & f) {
  os << f.str(true);
  return os; 
}
