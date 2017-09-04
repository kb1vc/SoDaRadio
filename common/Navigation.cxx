/*
  Copyright (c) 2012, Matthew H. Reilly (kb1vc)
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

#include "Navigation.hxx"
#include <iostream>

extern "C" {
#include "dem-gridlib.h"
}


int CheckGridSquare(const std::string & grid)
{
  if(DEM_CheckGrid(grid.c_str()) == 0) {
    std::cerr << "Got a good grid [" << grid << "]" << std::endl; 
    return 0; 
  }
  else {
    std::cerr << "Got a bad grid [" << grid << "]" << std::endl; 
    return -1; 
  }

  return 0; 
}

int GetBearingDistance(const std::string & from, const std::string & to,
		       float & bearing, float & rbearing, float & distance)
{
  if(DEM_CheckGrid(from.c_str())) {
    // not a reasonably formed maidenhead grid.
    return -1; 
  }
  DMSpoint from_dms; 
  DEM_GridtoDMS(from.c_str(), &from_dms); 

  if(DEM_CheckGrid(to.c_str())) {
    // not a reasonably formed maidenhead grid.
    return -1; 
  }
  DMSpoint to_dms; 
  DEM_GridtoDMS(to.c_str(), &to_dms);

  // now convert the points to lat/lon pairs.
  double flat, flon, tlat, tlon;
  DEM_DMStoFloat(&from_dms, &flat, &flon);
  DEM_DMStoFloat(&to_dms, &tlat, &tlon);

  // now get the bearing and reverse bearing and the distance
  double az, baz, dist;
  DEM_BearingDist(flat, flon, tlat, tlon, &az, &baz, &dist);

  bearing = (float) az;
  rbearing = (float) baz;
  distance = (float) dist;

  return 0; 
}


std::string GetGridSquare(double lat, double lon)
{
  char buf[1024];
  struct DMSpoint pt; 

  DEM_FloattoDMS(lat, lon, &pt); 
  DEM_DMStoGrid(buf, &pt); 
  
  return std::string(buf); 
}
