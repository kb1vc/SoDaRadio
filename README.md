# SoDa::Format print stuff.


 SoDa::Format is a class that allows intelligent formatting of
 integer, floating point, string, and character values into
 std::string objects or for output to a stream.  The concept is
 inspired by the much more capable Qt::QString's formatting
 features. If you can afford the library dependency, use Qt. It's
 quite good. 

 SoDa::Format is meant to improve upon the tremendously awkward, antiquated, and bizarre
 stream output features of the standard template library. (C++ stream IO was a giant step 
 backward from the comparatively flexible and intuitive FORTRAN "FORMAT" scheme. There isn't
 much in the computing world that is a giant step backward from FORTRAN circa 1975.)
 
 One could use the BOOST format facility. (One could also eat
 brussels sprouts for breakfast, but it isn't *my* kind of thing.
 Is it yours?) boost::format is tremendously powerful and fabulously
 documented.  It may be easily extended by beings living in some of
 the outer reaches of the Horse Head nebula. I just don't grok it
 myself. And carrying around a boost dependency is like growing an
 extra thumb -- it is useful at times, but it becomes less and less
 so as time goes on, and you can never quite remember why you
 got it in the first place.
 
 What really motivated me to write SoDa::Format was the lack of an
 "engineering notation" format for floating point numbers in any of
 the common formatting facilities.  As a dyed in the wool MKS
 engineer, this drives me right up the wall.  Exponents should be multiples of
 three.  If Adam had done any floating point arithmetic, he would have written
 it down in engineering notation.  (Perhaps he did, and then lost it all when
 the serpent screwed everything up.)  

So with SoDa::Format you can print out

322.0e-6 Farads

instead of the grotesque

3.22e-4 Farads.

Who even *thinks* like that?
