#pragma once
#include <string>
#include <time.h>

#if HAVE_GPSLIB
#include <gps.h>
#endif

/**
 * @file fix_gpsd_ugliness.hxx
 *
 * @brief libgpsd changes the API and other important things in ways
 * that ignore any thought of backward compatibility. I guess that's
 * something that you can do, but it really makes actual client code
 * that needs to be portable across platforms accrete all kinds of
 * workarounds and other barnacles. We can rail against sociopathic
 * behavior all we want, but we need the eggs.
 *
 * A guy walks into a clinic with his brother in tow. He says to 
 * the doctor, "Hey doc! You gotta help my brother here. He thinks
 * he's a chicken."
 *
 * "How long has this been going on?" asks the doctor. 
 * 
 * "About three years now," says the guy, "We would have brought him
 * in sooner, but we needed the eggs."
 * 
 */

namespace SoDa {

  /**
   * @class GPSDShim
   * 
   * @brief insulate the otherwise clean code in GPSmon from the
   * habitually backward incompatible changes from the GPSD folks.
   */
  class GPSDShim {
  public:
    /**
     * @brief setup a gps object if that's possible. Otherwise just smile politely. 
     * 
     * @param hostname the hostname for the gpsd server
     * @param portname the port number for the gps server
     */
    GPSDShim(const std::string & hostname, const std::string & portname);

    /**
     * @brief shut down the connection to gpsd.
     */
    ~GPSDShim();
    
    /**
     * @brief return true if we have a gps connection. 
     * 
     * @return true if we have a gps connection.  Did I already say that? 
     */
    bool isEnabled() { return gps_enabled; }
    
    /**
     * @brief get a fix (time and location) from the gps. 
     * return false if we have no time to report. (the GPS
     * device is disabled, or we don't have gps support, 
     * or the gpsd request timed out.)
     * 
     * This is a blocking call. It will return after a timeout. 
     * 
     * @param timeout time to wait (in microseconds)
     * @param utc time in hours, min, sec, ...
     * @param lat latitude
     * @param lon longitude
     * @return true if we got a fix. 
     * 
     */
    bool getFix(int timeout, struct tm & utc, double & lat, double & lon); 
    
  private:
#if HAVE_GPSLIB
    struct gps_data_t gps_buf; 
#endif    
    bool gps_enabled; 
  }; 
}
