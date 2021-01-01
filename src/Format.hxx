#pragma once
#include <string>
#include <exception>
#include <iostream>
#include <list>

/**
 * @file Format.hxx
 * @author Matt Reilly (kb1vc)
 * @date December 31, 2020
 */

/**
 * @mainpage SoDa::Format print stuff.
 * 
 * SoDa::Format is a class that allows intelligent formatting of
 * integer, floating point, string, and character values into
 * std::string objects or for output to a stream.  The concept is
 * inspired by the much more capable Qt::QString's formatting
 * features. If you can afford the library dependency, use Qt. It's
 * quite good. 
 *
 * SoDa::Format is meant to improve upon the tremendously awkward, antiquated, and bizarre
 * stream output features of the standard template library. (C++ stream IO was a giant step 
 * backward from the comparatively flexible and intuitive FORTRAN "FORMAT" scheme. There isn't
 * much in the computing world that is a giant step backward from FORTRAN circa 1975.)
 * 
 * One could use the BOOST format facility. (One could also eat
 * brussels sprouts for breakfast, but it isn't *my* kind of thing.
 * Is it yours?) boost::format is tremendously powerful and fabulously
 * documented.  It may be easily extended by beings living in some of
 * the outer reaches of the Horse Head nebula. I just don't grok it
 * myself. And carrying around a boost dependency is like growing an
 * extra thumb -- it is useful at times, but it becomes less and less
 * so as time goes on, and you can never quite remember why you
 * got it in the first place.
 * 
 * What really motivated me to write SoDa::Format was the lack of an
 * "engineering notation" format for floating point numbers in any of
 * the common formatting facilities.  As a dyed in the wool MKS
 * engineer, this drives me right up the wall.  Exponents should be multiples of
 * three.  If Adam had done any floating point arithmetic, he would have written
 * it down in engineering notation.  (Perhaps he did, and then lost it all when
 * the serpent screwed everything up.)  
 *
 * ## Enough of the ranting, let's look at an example. 
 * 
 * We'll look at segments of the test program Format_Test.cxx.  It is worth 
 * looking at the whole file, just not all at once.  This text is the doxygen 
 * equivalent of touring the Louvre on a motorcycle. 
 * 
 * To jump right in, let's imagine that we've got a set of variables that we want
 * to write to std::cout.  
 * - i an integer
 * - ef a double precision floating point number
 * - c a character
 * - fred a string
 * 
 * We could print all of those out like this: 
 * 
 * \snippet Format_Test.cxx printing some stuff
 * 
 * The result looks like this: 
 * 
 * \verbatim
print 5 like this:  5
let's print 5 again 5
32157.500 looks much better in engineering notation   32.158e3
Sometimes I just want to print a % sign like this 5%
I think that shirt he's wearing fits Fred to a T
 \endverbatim
 * 
 * Let's tease things apart here. 
 * 
 * The story starts with the creation of a format object like this
 * 
 * \snippet Format_Test.cxx create a format object
 * 
 * (In the first demo, we created the object and used it all on the same line. 
 * we can do that, or we can create a format and keep it around for a while.)
 * 
 * The format class has a bunch of methods that "fill in" parts of the
 * format string.  The methods: SoDa::Format::addI,
 * SoDa::Format::addU, SoDa::Format::addF, SoDa::Format::addS,
 * SoDa::Format::addC print integers, unsigned integers, floats or
 * doubles, strings, and characters. 
 * 
 * As each "addX" method is processed in turn, it fills in its placeholders. 
 * The first addX call fills in all the %0 placeholders *no matter how many
 * of them are in the string*.  The second fills in the %1 placeholders, and
 * so on. 
 * 
 * The floating point print method -- SoDa::Format::addF -- can print its
 * value in one of four formats
 * - f -- corresponding to printf's "%f" specifier (more or less)
 * - g -- .... "%g"
 * - s -- in scientific notation with the integer part between 1 and 9 inclusive. 
 * - e -- in engineering notation with the integer part between 1 and 999 inclusive, 
 * and the exponent a multiple of 3. 
 * 
 * So we could print Avogadro's number in two ways: the normal way, and the right way. 
 * 
 * \snippet Format_Test.cxx print avogadro's number
 * which produces
\verbatim
Avogadro's number: 6.022e+23
Here's how right thinking people write Avogadro's number:  602.214e21
\endverbatim
 * 
 * 
 * So once we've invoked the first addX method on a format, all the "%0" markers have been
 * replaced.  But we can "re-use" a format object by calling its SoDa::Format::reset method. 
 * as we did here
 * \snippet Format_Test.cxx reset the format
 * Now the format string is restored to its original state with all the "%0" and "%1" and
 * whatever markers back in place. 
 * 
 * Each of the addX methods returns a reference to the format object they just fiddled with. 
 * The reset method does too. 
 * That's how this snippet works: 
 * \snippet Format_Test.cxx reusing a format and multiple adds
 * 
 * The various methods provide arguments to set the field width and the number of digits after 
 * the decimal point.  When I get *really* focused, I'll add the ability to specify the number
 * of significant figures instead.  This is really far more useful than the "%fN.P" scheme that
 * we've been living with all these years.  But don't get me started on significant digits. 
 * 
 * Oh what the hell, we've got me started. 
 * 
 * For my money, it is rarely important to know anything to more than one significant digit. 
 *
 * Perhaps even one binary digit. 
 * 
 * ## Namespace
 * 
 * SoDa::Format is enclosed in the SoDa namespace because it is
 * inevitiable that there are lots of classes out there called
 * "Format."  Perhaps you have written one of them.  Naming a class "Format"
 * is like naming a street "Oak:" It might make lots of sense, but
 * you're going to have to reconcile yourself that there's a street 
 * with the same name one town over and sometimes your pizza is going
 * to get mis-routed. 
 * 
 * So Format is in the SoDa namespace.  SoDa is from 
 * <a href="https://kb1vc.github.io/SoDaRadio/">SoDaRadio</a> though the SoDa::Format 
 * class is a completely independent chunk 'o code. Most of the code I'll release
 * will be in the SoDa namespace, just to avoid the Oak Street problem.
 */

/**
 * @namespace SoDa
 * 
 * Not much else is spelled that way, so we're probably not going to have too many collisions
 * with code written by other people.  
 *
 * SoDa is the namespace I use in code like  <a href="https://kb1vc.github.io/SoDaRadio/">SoDaRadio</a>
 * (The S D and R stand for Software Defined Radio.  The o,a,d,i,o don't stand for anything in particular.)
 * But this code has nothing to do with software defined radios or any of that stuff. 
 */
namespace SoDa {
  class Format {
  public:
    Format(const std::string & fmt_string);

    Format & addI(int v, unsigned int width = 0);
    Format & addU(unsigned int v, unsigned int width = 0);    
    Format & addF(double v, char fmt = 'f', unsigned int width = 0, unsigned int frac_precision = 3);
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
    
