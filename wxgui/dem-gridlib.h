/*
Copyright (c) 2012,2013,2014 Matthew H. Reilly (kb1vc)
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

#ifndef DEM_GRIDLIB_DEF
#define DEM_GRIDLIB_DEF

#include <stdio.h>

    
#define DEM_AL         6378206.4   /*Clarke 1866 ellipsoid*/
#define DEM_BL         6356583.8

#define BITS_PER_BYTE 8
#define DEM_SIGN_MASK ((unsigned short) (1 << (BITS_PER_BYTE * sizeof(unsigned short) - 1)))

struct DMScoord {
  unsigned short deg;             /* msb is sign bit for coordinate. */
  unsigned char min;
  unsigned char sec;
};

struct DMSpoint {
  struct DMScoord lat, lon; 
};

int DEM_GridtoDMS(const char * in_gridsq, struct DMSpoint * point); 

char * DEM_DMStoGrid(char * str, struct DMSpoint * point);

void DEM_FloattoDMS(double lat, double lon, struct DMSpoint * point);

void DEM_DMStoFloat(struct DMSpoint * point, double * lat, double * lon);

void DEM_BearingDist(double Eplat,
		  double Eplon,
		  double Stlat,
		  double Stlon,
		  double * Az,
		  double * Baz,
		  double * Dist);

void DEM_Bearing2LL(
      double flat, double flon,   /* lat and lon (degrees) N,E are positive. */
      double *tlat, double *tlon, /* end-point lat and lon  (returned) */
      double az,                  /* bearing (degrees from N) */
      double *baz,                /* reverse bearing (returned) */
      double dist);                /* distance (in Km) */

int DEM_CheckGrid(const char * ing); 

void DEM_PrintCoord(FILE * out, struct DMScoord * coord, const char * dir);
void DEM_PrintPoint(FILE * out, struct DMSpoint * point);
char * DEM_DisplayCoord(char * buf, struct DMScoord * coord, const char * dir);
char * DEM_DisplayPoint(char * buf, struct DMSpoint * point);

double DEM_DMSCtoFloat(struct DMScoord * coord);
void DEM_DsecstoDMS(struct DMScoord * coord, double secs);



#endif



