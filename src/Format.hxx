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
 * So once we've invoked the first addX method on a format, all the
 * "%0" markers have been replaced.  But we can "re-use" a format
 * object by calling its SoDa::Format::reset method.  as we did here
 * \snippet Format_Test.cxx reset the format 
 * Now the format string is
 * restored to its original state with all the "%0" and "%1" and
 * whatever markers back in place.
 * 
 * Each of the addX methods returns a reference to the format object
 * they just fiddled with.  The reset method does too.  That's how
 * this snippet works:
 *
 * \snippet Format_Test.cxx reusing a format and multiple adds
 * 
 * The various methods provide arguments to set the field width and
 * the number of digits after the decimal point.  When I get *really*
 * focused, I'll add the ability to specify the number of significant
 * figures instead.  This is really far more useful than the "%fN.P"
 * scheme that we've been living with all these years.  But don't get
 * me started on significant digits.
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
 * Not much else is spelled that way, so we're probably not going to
 * have too many collisions with code written by other people.
 *
 * SoDa is the namespace I use in code like <a
 * href="https://kb1vc.github.io/SoDaRadio/">SoDaRadio</a> (The S D
 * and R stand for Software Defined Radio.  The o,a,d,i,o don't stand
 * for anything in particular.)  But this code has nothing to do with
 * software defined radios or any of that stuff.
 */
namespace SoDa {
  
    /**
     * @class Format 
     * @brief A format object that may be "filled in" with
     * integer, float, double, string, or character values.
     * 
     * # The format string
     * 
     * The format string supplied here contains value placeholders of
     * the form "%[0-9]*" That is, a percent sign "%" followed by one
     * or more decimal digits. Any placeholder may occur at any place
     * in the string. A placeholder may appear more than once.  So
     * this string 
     * \code "first val %0 third val %2 second val %1 third val all over again %2\n" \endcode
     * is just fine.
     * 
     * Values are numbered starting at zero, because that's the way we roll 
     * here.  Values are supplied by invocations of the Format::addX methods, 
     * as in 
     * - addI (integer)  
     * - addU (unsigned integer) 
     * - addF (float or double) 
     * - addS (string)
     * - addC (character)
     * 
     * Most of these provide for additional format directives that determine 
     * minimum field with and decimal precision. 
     *
     * As each .addX method is invoked, it fills in the associated
     * placeholders.  The format string, thus evolves as the
     * placeholders are replaced with the formatted representation of
     * the supplied values.
     * 
     * The format string, like any other string object, follows the "\" escape
     * conventions.  In addition, since "%" is used as a special character, 
     * the string "%%" may be used to insert a single "%" character. 
     * 
     * # Other Places, Other Norms
     * 
     * Those in areas where the decimal radix point is *not* "." or those
     * who simply wish for an alternative may set the radix separator by 
     * assigning to the static variable SoDa::Format::separator like this
     * \code SoDa::Format::separator = ','; \endcode
     * 
     * # Friendship and Strings
     *
     * The output stream operator "<<" is a friend of the SoDa::Format 
     * class.  
     * 
     * The SoDa::Format::str() method will return a reference to the current
     * state of the format string (with all the placeholders filled in, as
     * far as the process has progressed.)
     * 
     * # When good code goes bad
     * 
     * SoDa::Format methods don't return success/failure values to distinguish
     * between good outcomes and errors.  If something goes wrong that 
     * SoDa::Format feels bad about, it will throw a SoDa::Format::BadFormat
     * exception.  This inherits from "std::runtime_exception" so it is 
     * moderately well behaved. 
     * 
     * SoDa::Format uses exceptions because, after years of reading my code
     * and the code of others, it is a rare and disciplined individual who 
     * checks every return value for errors.  And their code looks like 
     * crap. 
     * 
     * ### Inside Baseball note
     * More skillful programmers might have overloaded
     * one method name for all five types.  However, making a C++ compiler 
     * distinguish between add(int v) and add(unsigned int v) -- though it
     * seems obvious that this should work -- proved beyond my ability, and
     * may be outside of the C++11 definition.  LLVM warned about it loudly. 
     */
  
  class Format {
  public:
    /**
     * @brief create a format object with placeholders and all that stuff.
     * 
     * @param fmt_string the format string with placeholders and stuff. 
     */
    Format(const std::string & fmt_string);

    /**
     * @brief insert a signed integer into the format string
     * 
     * @param v the signed integer value
     * @param width the minimum width of the field. 
     * @return a reference to this SoDa::Format object to allow 
     * chaining of method invocations. 
     * 
     * The width is passed along to the setw I/O manipulator, so it
     * is no more useful than that.  If the value takes more room than 
     * the specified width, the width parameter will be ignored and 
     * the field will be as wide as necessary to accommodate the value. 
     */
    Format & addI(int v, unsigned int width = 0);

    /**
     * @brief insert an unsigned integer into the format string
     * 
     * @param v the unsigned integer value
     * @param width the minimum width of the field. 
     * @return a reference to this SoDa::Format object to allow 
     * chaining of method invocations. 
     * 
     * The width is passed along to the setw I/O manipulator, so it
     * is no more useful than that.  If the value takes more room than 
     * the specified width, the width parameter will be ignored and 
     * the field will be as wide as necessary to accommodate the value. 
     */
    Format & addU(unsigned int v, unsigned int width = 0);


    /**
     * @brief insert a float or double into the format string
     * 
     * @param v the double precision value (floats will be promoted)
     * @param fmt a choice of representations (see below)
     * @param width the minimum width of the field. 
     * @param frac_precision the number of decimal positions to the
     * right of the radix point. 
     * @return a reference to this SoDa::Format object to allow 
     * chaining of method invocations. 
     * 
     *
     * The default value for the width parameter will take up only 
     * as much room as the value requires. 
     * 
     * 
     * ### representation choice 'f'
     * 
     * The value is converted to a string in the manner of the 
     * C++ stream "std::fixed" format.  The width parameter sets
     * the minimum field width, and the frac_precision parameter
     * determines the number of places to the right of the radix point. 
     * 
     *
     * The default value for the width parameter will take up only 
     * as much room as the value requires. 
     * 
     * ### representation choice 's'
     * 
     * The value is converted to a string in the manner of the 
     * C++ stream "std::scientific" format.  The width parameter sets
     * the minimum field width, and the frac_precision parameter
     * determines the number of places to the right of the radix point. 
     * THe number will be represented as a floating string value in the
     * range 1.0 to 1.999999999...999 followed by an exponent specifier
     * like "e+" or "e-" followed by one or more digits. 
     * 
     * The exponent may take on just about any value in the range of
     * the machine's double precision datatype.
     *
     * The default value for the width parameter will take up only 
     * as much room as the value requires. 
     * 
     * ### representation choice 'g'
     * 
     * If you just want to get the job done and don't care how your
     * values appear, the 'g' format will use the default  C++ 
     * stream conversion to float. It could be fixed.  It could
     * be scientific. 
     * The width parameter will set the minimum field width. 
     * 
     * The default value for the width parameter will take up only 
     * as much room as the value requires. 
     * 
     * ### representation choice 'e'
     * 
     * This is similar to the 's' format, but the exponent (power of 10) 
     * will always be a multiple of 3.  That makes the value useful to 
     * those who think in Peta, Tera, Giga, Mega, kilo, milli, micro, nano, 
     * and pico.  Those left in the CGS world can write their own code. 
     * 
     * The width parameter is ignored.  The field width is determined by 
     * the fmt choice and the frac_precision. The field contain a leading
     * sign, up to three decimal digits, a radix point, frac_precision digits, 
     * "e+" or "e-" and one or more exponent digits. 
     * 
     * The leading (whole) part of the value will be padded with spaces
     * to the left of the sign. 
     * 
     */
    Format & addF(double v, char fmt = 'f', unsigned int width = 0, unsigned int frac_precision = 3);

    /**
     * @brief insert a string into the format string
     * 
     * @param v the string value
     * @param width the minimum width of the field. 
     * @return a reference to this SoDa::Format object to allow 
     * chaining of method invocations. 
     * 
     * The width is passed along to the setw I/O manipulator, so it
     * is no more useful than that.  If the string is wider than the
     * width, the field will expand to fit the string.  If the string
     * is smaller than the width the field will be padded to the 
     * left with spaces. 
     *
     * The default value for the width parameter will take up only 
     * as much room as the value requires. 
     * 
     */
    Format & addS(const std::string & v, unsigned int width = 0);
    
    /**
     * @brief insert a character into the format string
     * 
     * @param v the character value
     * @return a reference to this SoDa::Format object to allow 
     * chaining of method invocations. 
     * 
     * Not much to say here. 
     */
    Format & addC(char v);    

    /**
     * @brief reset the format string to its original value, with all
     * the placeholders restored. 
     * 
     * This method allows a format object to be re-used.  
     * 
     * @return a reference to a format object to allow chaining. 
     */
    Format & reset();
    
    /**
     * @brief Exception to announce that there was something wrong with the 
     * format string, or that too many values were supplied. 
     * 
     * Inherits from std::runtime_error so the catcher may use "what" to
     * find out what went wrong. 
     */
    class BadFormat : public std::runtime_error {
    public:
      /**
       * @brief build a BadFormat exception object. 
       * 
       * @param problem What went wrong
       * @param fmt a reference to the format so we can print the
       * original format spec. 
       */
      BadFormat(const std::string & problem, const Format & fmt) :
	std::runtime_error(problem + " format string was \n" + fmt.getOrig()) { }
    };

    /**
     * @brief provide a string representing the current state of the
     * format string with all placeholders "filled in" as far as 
     * the last invocation of an add method. 
     * 
     * @param check_for_filled_out If a placeholder has not yet been 
     * replaced, this method *may* throw a BadFormat exception. 
     * It doesn't yet. 
     * 
     * @return a reference to the format string. 
     */
    const std::string & str(bool check_for_filled_out = false) const; 

    /**
     * @brief the radix separator character. 
     * 
     * For those in locales where pi is 3,14159 or so, the separater
     * may be replaced, like this: 
     * 
     * \code SoDa::Format::separator = ','; \endcode
     * 
     */
    static char separator; 
    
  protected:

    const std::string & getOrig() const { return orig_fmt_string; }
    
    std::string fmt_string;
    std::string orig_fmt_string; 
    unsigned int cur_arg_number;
    int max_field_num; 

    std::list<size_t> escape_positions;

    void initialScan(); 

    void insertField(const std::string & s);
  }; 
}

std::ostream& operator<<(std::ostream & os, const SoDa::Format & f);
    
