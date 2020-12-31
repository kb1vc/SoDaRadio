#pragma once
#include <string>
#include <exception>
#include <iostream>
#include <list>

namespace SoDa {
  class Format {
  public:
    Format(const std::string & fmt_string);

    Format & addI(int v, unsigned int width = 0);
    Format & addU(unsigned int v, unsigned int width = 0);    
    Format & addD(double v, char fmt = 'f', unsigned int width = 6, unsigned int frac_precision = 3);
    Format & addF(float v, char fmt = 'f', unsigned int width = 6, unsigned int frac_precision = 3);
    Format & addS(const std::string & v, unsigned int width = 0);
    Format & addC(char v);    

    Format & reset();
    
    class BadFormat : public std::runtime_error {
    public:
      BadFormat(const std::string & problem, const Format & fmt) :
	std::runtime_error(problem + " format string was [" + fmt.getOrig() + "]") { }
    };

    const std::string & str(bool check_for_filled_out = false) const; 

    static char separator; 
    
    const std::string & getOrig() const { return orig_fmt_string; }
  protected:
    
    std::string fmt_string;
    std::string orig_fmt_string; 
    unsigned int cur_arg_number;
    unsigned int max_field_num; 

    std::list<size_t> escape_positions;

    void initialScan(); 

    std::string toEngineering(double v);
    
    void insertField(const std::string & s);
  }; 
}

std::ostream& operator<<(std::ostream & os, const SoDa::Format & f);
    
