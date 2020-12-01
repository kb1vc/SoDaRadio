#include "fix_gpsd_ugliness.hxx"
#include <iostream>
#include <errno.h>


#if HAVE_GPSLIB
#include <gps.h>
# if GPSD_API_MAJOR_VERSION < 9
#   define GPSLIB_MIGHT_BE_USABLE 1
# else
#   define GPSLIB_MIGHT_BE_USABLE 0
#   pragma message "Turned off GPSD support, API version is likely broken.\nThis may not be an issue, but we cannot be sure."
# endif
#else
# define GPSLIB_MIGHT_BE_USABLE 0
#endif

// Bump this each time we test against a new version of libgpsd
// The API changes in incompatible ways. We can't test it in the
// cmake script because the lib version and the API version aren't
// related in any useful way.  

SoDa::GPSDShim::GPSDShim(const std::string & hostname, const std::string & portname) {
#if GPSLIB_MIGHT_BE_USABLE

  struct gps_data_t gps_buf; 
  errno = 0;
  int stat = gps_open(hostname.c_str(), portname.c_str(), &gps_buf);
  if(stat != 0) {
    std::cerr << "GPSDShim::GPSDShim Could not open gpsd server connection. Error :" 
	      <<  gps_errstr(errno) << "\n";
    gps_enabled = false; 
  }
  else {
    (void) gps_stream(&gps_buf, WATCH_ENABLE, NULL); 
  }
#else
  gps_enabled = false; 
#endif
}

SoDa::GPSDShim::~GPSDShim() {
#if GPSLIB_MIGHT_BE_USABLE
  (void) gps_stream(&gps_buf, WATCH_DISABLE, NULL);
  (void) gps_close(&gps_buf); 
#endif  
}


bool SoDa::GPSDShim::getFix(int timeout, struct tm & utc, double & lat, double & lon) {

#if GPSLIB_MIGHT_BE_USABLE
  // if we have no GPS, we have nothing to say.   
  if(!gps_enabled) return false;  
  
  if (!gps_waiting(&gps_buf, timeout)) return false; 

  errno = 0;

  int stat;
  
# if GPSD_API_MAJOR_VERSION < 7
  stat = gps_read(&gps_buf);
# else  
  stat = gps_read(&gps_buf, NULL, 0);
# endif
  
  if(stat == -1) { 
    gps_enabled = false;
    std::cerr << "GPSDShim::getFix Could not read from gps server. Error : " 
	      << gps_errstr(errno) << "\n";
    return false; 
  }
  else {
    time_t tt; 
# if GPSD_API_MAJOR_VERSION < 9
    tt = (time_t) gps_buf.fix.time; 
# else
    tt = (time_t) gps_buf.fix.time.tv_sec;
# endif

    gmtime_r(&tt, &utc); 
    lat = gps_buf.fix.latitude;
    lon = gps_buf.fix.longitude;
    return true; 
  }
#else
  // We have no GPS, we have nothing to say.   
  return false; 
#endif  
  
  
}
