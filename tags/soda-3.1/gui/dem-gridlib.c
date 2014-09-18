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

#define __NO_MATH_INLINES
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "dem-gridlib.h"
#define FALSE 0
#define TRUE 1

/*Determines bearing and distance based on*/
/*algorithm compensating for earth's shape*/
/* Modified and heavily borrowed from code that is */
/*Copyright 1993 Michael R. Owen, W9IP & */
/*               Paul Wade, N1BWT */
/*Released to the public domain Feb 23, 1993*/
    /* Used by permission of the authors. */
/* 
** 
** Translated to c via p2c  with appropriate
** post-translation hacks by Matt Reilly (KB1VC). 
*/
/* 
**
** Recoding to be compatible with the DEM/DEM library
** functions by Matt Reilly KB1VC  June, 1996 
*/

/*NOTE: NORTH latitude, EAST longitude are positive;   */
/*      South & West should be input as negative numbers
**   This is in keeping with the convention from the DEM 
** files from the USGS.  Note that the original N1BWT code
** assumed that negative longitudes were east.
*/

static double my_atan2(double Y, double X)
{
  double retval;

  retval = atan2(Y, X);
  if(retval < 0.0) retval = ((2.0 * M_PI) + retval);

  return retval;
}


/*---------------------------------------------------------------*/
int DEM_CheckGrid(const char * ing)
{
  /*verifies that the grid square is legitimate*/
  /* return 1 if it is bad. */
  int error;
  long i;
  char g[7];

  strncpy(g, ing, 6);
  
  if (strlen(g) == 4)   /*choose middle if only 4-character*/
    strcat(g, "LL");
  error = FALSE;
  i = 0;

  do {
    i++;
    if (i == 3 || i == 4) {
      if (g[i - 1] >= '0' && g[i - 1] <= '9')
	error = FALSE;   /*2nd two characters are numbers*/
      else {
	error = TRUE;

      }
    } else if (g[i - 1] >= 'A' && g[i - 1] <= 'Z')
      error = FALSE;   /*first and last 2 characters are letters*/
    else if (g[i - 1] >= 'a' && g[i - 1] <= 'z') {
      g[i - 1] = _toupper(g[i - 1]);
      error = FALSE;
    } else
      error = TRUE;
  } while (!(i == 6 || error));


  return error;

}  /* DEM_CheckGrid */

int DEM_DMStoStr(char * buf, struct DMSpoint * point) 
{
  int latd, lond;
  char latc, lonc;

  latd = point->lat.deg & ~DEM_SIGN_MASK;
  latc = (point->lat.deg & DEM_SIGN_MASK) ? 'S' : 'N';
  lond = point->lon.deg & ~DEM_SIGN_MASK;
  lonc = (point->lon.deg & DEM_SIGN_MASK) ? 'W' : 'E';
  
  sprintf(buf, "%02d %02d %02d %c %03d %02d %02d %c",
	  latd, point->lat.min, point->lat.sec, latc,
	  lond, point->lon.min, point->lon.sec, lonc);

  return 0;
}

static void DEM_cvt_dbl2dms(double loc, int * d, int * m, int * s)
{
  double partial;
  int secs; 

  *d = (int) floor(loc);

  partial = loc - floor(loc);

  secs = (int) floor(3600.0 * partial);
  *m = secs / 60;
  *s = secs % 60;

  return;
}

int DEM_StrtoDMS(const char * buf, struct DMSpoint * point)
{

  int latd, latm, lats, lond, lonm, lons;
  int stat;
  double flon, flat; 
  char latc, lonc;

  stat = sscanf(buf, "%dd%2d%2d%c %dd%2d%2d%c",
	        &latd, &latm, &lats, &latc, 
                &lond, &lonm, &lons, &lonc); 
  if(stat != 8) {
    stat = sscanf(buf, "%dd%2d%c %dd%2d%c",
	          &latd, &latm, &latc, 
                  &lond, &lonm, &lonc);
    if(stat != 6) {
      stat = sscanf(buf, "%lf%c %lf%c",
		    &flat, &latc, &flon, &lonc);
      if(stat == 4) {
	DEM_cvt_dbl2dms(flat, &latd, &latm, &lats);
	DEM_cvt_dbl2dms(flon, &lond, &lonm, &lons);
      }
      else if((stat = sscanf(buf, "%lf %lf",
			     &flat, &flon)) == 2) {
	if(flon < 0.0) {
	  lonc = 'W';
	  flon = -flon;
	}
	else lonc = 'E';
	
	if(flat < 0.0) {
	  latc = 'S';
	  flat = -flat;
	}
	else latc = 'N';

	DEM_cvt_dbl2dms(flat, &latd, &latm, &lats);
	DEM_cvt_dbl2dms(flon, &lond, &lonm, &lons);
      }
      else {
	/* I don't know what this is.. */
	return 1;
      }
    }
    else {
      lats = 0;
      lons = 0;
    }
  }
  

  point->lat.deg = (unsigned short) latd;
  if ((latc == 'S') || (latc == 's')) point->lat.deg |= DEM_SIGN_MASK;

  point->lon.deg = (unsigned short) lond;
  if ((lonc == 'W') || (lonc == 'w')) point->lon.deg |= DEM_SIGN_MASK;

  point->lat.min = (unsigned short) latm;
  point->lon.min = (unsigned short) lonm;

  point->lat.sec = (unsigned short) lats;
  point->lon.sec = (unsigned short) lons;

  return 0;
}


int DEM_GPStoDMS(const char * buf, struct DMSpoint * point)
{
  double lat, lon, lats, lons, latif, lonif;
  int lati, loni;
  char latc, lonc;

  sscanf(buf, "%lf %c %lf %c", &lat, &latc, &lon, &lonc);

  lats = (int) (60.0 * modf(lat, &latif));
  lons = (int) (60.0 * modf(lon, &lonif));

  lati = (int) latif;
  loni = (int) lonif;

  point->lat.deg = (unsigned short) (lati / 100);
  if (latc == 'S') point->lat.deg |= DEM_SIGN_MASK;

  point->lon.deg = (unsigned short) (loni / 100);
  if (lonc == 'W') point->lon.deg |= DEM_SIGN_MASK;

  point->lat.min = (unsigned short) (lati % 100); 
  point->lon.min = (unsigned short) (loni % 100);

  point->lat.sec = (unsigned short) lats;
  point->lon.sec = (unsigned short) lons;

  return 0;
}


/*------------------------------------------------------------*/
int DEM_GridtoDMS(const char * in_gridsq, struct DMSpoint * point)
{
  /*finds the lat/lon of center of the sub-square*/
  /* return 1 if the grid is bad, 0 otherwise. */
  int latsec, lonsec, londeg;
  double flondeg, flonmin, ffrac, fint;
  char gridsq[7];
  int i;

  strncpy(gridsq, in_gridsq, 6);

  if(DEM_CheckGrid(in_gridsq)) return 1; 

  for(i = 0; i < 2; i++)
    {
      gridsq[i] = toupper(gridsq[i]);
      gridsq[i+4] = toupper(gridsq[i+4]);
    }
  
  flondeg = ((float)('A' - gridsq[0])) * 20.0 +
            ((float)('0' - gridsq[2])) * 2.0 + 180.0;

  flonmin = (short) ((gridsq[4] - 'A')) * 5.0 + 2.5;

  flondeg -= flonmin / 60.0;

  point->lon.deg = (flondeg > 0.0) ?
                     (((short) flondeg) | DEM_SIGN_MASK) :
		     ((short) flondeg);

  ffrac = modf(fabs(flondeg), &fint);
  
  point->lon.min = (char) (ffrac * 60.0);
  ffrac = modf(ffrac * 60.0, &fint); 
  point->lon.sec = (char) (ffrac * 60.0 + 0.5);
  
  point->lat.deg = (gridsq[1] - 'A') * 10 + gridsq[3] - '0' - 90; 
  latsec = ((double)(gridsq[5] - 'A')) * 150 + 75;   

  point->lat.deg += latsec / 3600;
  latsec = latsec % 3600; 
  
  point->lat.min = (char) (latsec / 60);
  point->lat.sec = (char) (latsec % 60); 


  return 0;
}


void DEM_FloattoDMS(double lat, double lon, struct DMSpoint * point)
{
  double alat, alon, ltpart, lnpart;
  int ltsecs, lnsecs;
  
  alat = fabs(lat);
  alon = fabs(lon);
  
  point->lat.deg = (short) floor(alat);
  if(lat < 0) point->lat.deg |= DEM_SIGN_MASK;

  point->lon.deg = (short) floor(alon);
  if(lon < 0) point->lon.deg |= DEM_SIGN_MASK;

  ltpart = alat - floor(alat);
  lnpart = alon - floor(alon);

  ltsecs = (int) (3600.0 * ltpart); 
  lnsecs = (int) (3600.0 * lnpart);
  
  point->lat.min = ltsecs / 60; 
  point->lat.sec = ltsecs  - (60 * point->lat.min); 

  point->lon.min = lnsecs / 60; 
  point->lon.sec = lnsecs  - (60 * point->lon.min); 

  return; 
}

/*------------------------------------------------------------*/
char * DEM_DMStoGrid(char * str, struct DMSpoint * point)
{
  /*converts lat & long to grid square*/
  long c;
  double g4, R, M, L4;
  
  double TEMP, TEMP1;
  double LocLat, LocLon;

  DEM_DMStoFloat(point, &LocLat, &LocLon);


  LocLon *= -1.0;

  g4 = 180 - LocLon;
  c = (long)(g4 / 20);
  str[0] = (char)(c + 65);
  R = fabs(LocLon / 20);
  modf(R, &TEMP);
  modf((R - TEMP) * 20, &TEMP1);
  R = TEMP1;
  c = (long)(R / 2);
  if (LocLon >= 0)
    c = labs(c - 9L);
  str[2] = (char)(c + 48);
  M = fabs(LocLon * 60.0) / 120.0;
  modf(M, &TEMP);
  M = (M - TEMP) * 120.0;
  modf(M, &TEMP);
  M = TEMP;
  c = (long)(M / 5.0);
  if (LocLon >= 0)
    c = labs(c - 23);
  str[4] = (char)(c + 'a');
  L4 = LocLat + 90.0;
  c = (long)(L4 / 10.0);
  str[1] = (char)(c + 65);
  R = fabs(LocLat / 10.0);
  modf(R, &TEMP);
  modf((R - TEMP) * 10.0, &TEMP1);
  R = TEMP1;
  c = (long)R;
  if (LocLat < 0)
    c = labs(c - 9);
  str[3] = (char)(c + 48);
  M = fabs(LocLat * 60.0) / 60.0;
  modf(M, &TEMP);
  M = (M - TEMP) * 60.0;
  c = (long)(M / 2.5);
  if (LocLat < 0)
    c = labs(c - 23);
  str[5] = (char)(c + 'a');
  str[6] = '\000';

  
  return str;
}

void DEM_DMStoFloat(struct DMSpoint * point, double * lat, double * lon)
{
  double latsec, lonsec;
  int latsign, lonsign, latdeg, londeg;


  
  if(latsign = point->lat.deg & DEM_SIGN_MASK) {
    latdeg = point->lat.deg ^ DEM_SIGN_MASK;
  }
  else {
    latdeg = point->lat.deg;
  }

  if(lonsign = point->lon.deg & DEM_SIGN_MASK) {
    londeg = point->lon.deg ^ DEM_SIGN_MASK;
  }
  else {
    londeg = point->lon.deg;
  }
    
  

  latsec = (double) (point->lat.min * 60 + point->lat.sec);
  *lat = ((double) latdeg) + latsec / 3600.0;
  if(latsign) *lat = -1.0 * *lat;

  lonsec = (double) (point->lon.min * 60 + point->lon.sec);
  *lon = ((double) londeg) + lonsec / 3600.0;
  if(lonsign) *lon = -1.0 * *lon;

  return;
}


/*------------------------------------------------------------*/
static int In_Range(Value, LowVal, HighVal)
double Value, LowVal, HighVal;
{
  /*checks that the value is between Lowval and Highval*/
  return (Value >= LowVal && Value <= HighVal);
}


#define D2R         (M_PI / 180.0) /*degrees to radians conversion factor*/
#define Pi2         (2.0 * M_PI)


/*------------------------------------------------------------*/

void DEM_BearingDist(double Eplat,
		  double Eplon,
		  double Stlat,
		  double Stlon,
		  double * Az,
		  double * Baz,
		  double * Dist)
{

  /*Taken directly from:       */
  /*Thomas, P.D., 1970, Spheroidal Geodesics, reference systems,*/
  /*    & local geometry, U.S. Naval Oceanographic Office SP-138,*/
  /*    165 pp.*/

  /*assumes North Latitude and East Longitude are positive*/

  /*EpLat, EpLon = MyLat, MyLon*/
  /*Stlat, Stlon = HisLat, HisLon*/
  /*Az, BAz = direct & reverse azimuith*/
  /*Dist = Dist (km);  */
  double BOA, F, P1R, P2R, L1R, L2R, DLR, T1R, T2R, TM, DTM, STM, CTM, SDTM,
	CDTM, KL, KK, SDLMR, L, CD, DL, SD, T, U, V, D, X, E, Y, A, FF64,
	TDLPM, HAPBR, HAMBR, A1M2, A2M1;

  if((fabs(Eplat - Stlat) < 1.0e-10) && (fabs(Eplon - Stlon) < 1.0e-10)) {
    *Az = 0.0;
    *Baz = 0.0;
    *Dist = 0.0;
    return;
  }

  BOA = DEM_BL / DEM_AL;
  F = 1.0 - BOA;
  P1R = Eplat * D2R;
  P2R = Stlat * D2R;
  L1R = Eplon * D2R;
  L2R = Stlon * D2R;
  DLR = L1R - L2R;
  T1R = atan(BOA * tan(P1R));
  T2R = atan(BOA * tan(P2R));
  TM = (T1R + T2R) / 2.0;
  DTM = (T2R - T1R) / 2.0;
  STM = sin(TM);
  CTM = cos(TM);
  SDTM = sin(DTM);
  CDTM = cos(DTM);
  KL = STM * CDTM;
  KK = SDTM * CTM;
  SDLMR = sin(DLR / 2.0);
  L = SDTM * SDTM + SDLMR * SDLMR * (CDTM * CDTM - STM * STM);
  CD = 1.0 - 2.0 * L;
  DL = acos(CD);
  SD = sin(DL);
  T = DL / SD;
  U = 2.0 * KL * KL / (1.0 - L);
  V = 2.0 * KK * KK / L;
  D = 4.0 * T * T;
  X = U + V;
  E = -2.0 * CD;
  Y = U - V;
  A = -D * E;
  FF64 = F * F / 64.0;
  *Dist = DEM_AL * SD * (T -
		      F / 4.0 * (T * X - Y) +
		      FF64 * (X * (A + (T - (A + E) / 2.0) * X) +
			      Y * (E * Y - 2.0 * D) + D * X * Y)) / 1000.0;

  TDLPM = tan((DLR - (E * (4.0 - X) + 2.0 * Y) * (F / 2.0 * T +
		       FF64 * (32.0 * T + (A - 20.0 * T) * X -
			       2.0 * (D + 2.0) * Y)) / 4.0 * tan(DLR)) / 2.0);

  HAPBR = my_atan2(SDTM, CTM * TDLPM); 
  HAMBR = my_atan2(CDTM, STM * TDLPM); 

  A1M2 = Pi2 + HAMBR - HAPBR;
  A2M1 = Pi2 - HAMBR - HAPBR;

  while(!((A1M2 >= 0.0) && (A1M2 < Pi2)))
    {
      if(A1M2 >= Pi2)
	{
	  A1M2 = A1M2 - Pi2; 
	}
      else
	{
	  A1M2 = A1M2 + Pi2;
	}
    }

  while(!((A2M1 >= 0) && (A2M1 < Pi2)))
    {
      if(A2M1 >= Pi2)
	{
	  A2M1 = A2M1 - Pi2;
	}
      else
	{
	  A2M1 = A2M1 + Pi2;
	}
    }

  /* these 360 degree corrections were added to
     fix a disagreement in the semantics of 
     an older implementation of ATAN2, vs
     the implementation from the gnu c math rtl. */
  if(A1M2 == 0.0) *Az = 0.0;
  else *Az = 360.0 - (A1M2 / D2R);

  
  if(A2M1 == 0.0) *Baz = 0.0;
  else *Baz = 360.0 - (A2M1 / D2R);
}


/* forward.f -- translated by f2c (version 19960717).
   then hacked up a bit by hand to remove the dependence on 
   the f2c libraries and such. */ 
void DEM_Bearing2LL(
      double flat, double flon,   /* lat and lon (degrees) N,E are positive. */
      double *tlat, double *tlon, /* end-point lat and lon  (returned) */
      double az,                  /* bearing (degrees from N) */
      double *baz,                /* reverse bearing (returned) */
      double dist)                /* distance (in Km) */
{
    /* Initialized data */

    static double eps = 5e-14;
    double a = DEM_AL; /* (meters) */
    double f = 1.0/298.25722210088; 

    /* System generated locals */
    double d1;

    /* Builtin functions */
    double sin(), cos(), atan2(), sqrt();

    /* Local variables */
    static double c, d, e, r, x, y, cf, sa, cu, sf, cy, cz, su, tu, 
	    sy, c2a, s, faz, rbaz;

    double glat1, glon1, glat2, glon2; 

/* *** SOLUTION OF THE GEODETIC DIRECT PROBLEM AFTER T.VINCENTY */
/* *** MODIFIED RAINSFORD'S METHOD WITH HELMERT'S ELLIPTICAL TERMS */
/* *** EFFECTIVE IN ANY AZIMUTH AND AT ANY DISTANCE SHORT OF ANTIPODAL */

/* *** A IS THE SEMI-MAJOR AXIS OF THE REFERENCE ELLIPSOID */
/* *** F IS THE FLATTENING OF THE REFERENCE ELLIPSOID */
/* *** LATITUDES AND LONGITUDES IN RADIANS POSITIVE NORTH AND EAST */
/* *** AZIMUTHS IN RADIANS CLOCKWISE FROM NORTH */
/* *** GEODESIC DISTANCE S ASSUMED IN UNITS OF SEMI-MAJOR AXIS A */

/* *** PROGRAMMED FOR CDC-6600 BY LCDR L.PFEIFER NGS ROCKVILLE MD 20FEB75 
*/
/* *** MODIFIED FOR SYSTEM 360 BY JOHN G GERGEN NGS ROCKVILLE MD 750608 */
/* *** HACKED TO HELL AND GONE BY F2C (July-17-96 version) and by 
   *** Matt Reilly (KB1VC) to make it fit with the dem-gridlib routines.
   *** Feb 24, 1997. */

    /* distance is in Km... */ 
    s = dist * 1000.0;
    faz = az * M_PI / 180.0;

    glat1 = flat * M_PI / 180.0;
    glon1 = flon * M_PI / 180.0; 
    
    r = 1.0 - f;
    
    tu = r * sin(glat1) / cos(glat1);
    
    sf = sin(faz);
    cf = cos(faz);
    rbaz = 0.0;
    if (cf != 0.0) {
	rbaz = atan2(tu, cf) * 2.0;
    }
    cu = 1.0 / sqrt(tu * tu + 1.0);
    su = tu * cu;
    sa = cu * sf;
    c2a = -sa * sa + 1.0;
    x = sqrt((1.0 / r / r - 1.0) * c2a + 1.0) + (float)
	    1.;
    x = (x - 2.0) / x;
    c = 1.0 - x;
    c = (x * x / 4.0 + 1) / c;
    d = (x * .375 * x - 1.0) * x;
    tu = s / r / a / c;
    y = tu;
L100:
    sy = sin(y);
    cy = cos(y);
    cz = cos(rbaz + y);
    e = cz * cz * 2.0 - 1.0;
    c = y;
    x = e * cy;
    y = e + e - 1.0;
    y = (((sy * sy * 4.0 - 3.0) * y * cz * d / 6.0 + x) * 
	    d / (float)4. - cz) * sy * d + tu;
    if (fabs(y - c) > eps) {
	goto L100;
    }
    rbaz = cu * cy * cf - su * sy;
    c = r * sqrt(sa * sa + rbaz * rbaz);
    d = su * cy + cu * sy * cf;
    glat2 = atan2(d, c);
    c = cu * cy - su * sy * cf;
    x = atan2(sy * sf, c);
    c = ((c2a * -3.0 + 4.0) * f + 4.0) * c2a * 
	    f / 16.0;
    d = ((e * cy * c + cz) * sy * c + y) * sa;
    glon2 = glon1 + x - (1.0 - c) * d * f;
    rbaz = atan2(sa, *baz) + M_PI;

    *tlat = glat2 * 180.0 / M_PI;
    *tlon = glon2 * 180.0 / M_PI; 
    *baz = rbaz * 180.0 / M_PI;
    
    return; 
}


char * DEM_DisplayPoint(char * buf, struct DMSpoint * point)
{
  DEM_DisplayCoord(buf, &(point->lat), "NS");
  buf[strlen(buf)] = ' ';
  DEM_DisplayCoord(buf + strlen(buf), &(point->lon), "EW");

  return buf;
}


char * DEM_DisplayCoord(char * buf, struct DMScoord * coord, const char * dir)
{
  sprintf(buf, "%3dd%02d\'%02d\''%c",
          coord->deg & ~DEM_SIGN_MASK,
          coord->min, coord->sec, 
          (coord->deg & DEM_SIGN_MASK) ? dir[1] : dir[0]);

  return buf;
}

void DEM_PrintCoord(FILE * out, struct DMScoord * coord, const char * dir) 
{
  char buf[20];

  fprintf(out, "%s", DEM_DisplayCoord(buf, coord, dir));
}

void DEM_PrintPoint(FILE * out, struct DMSpoint * point)
{
  DEM_PrintCoord(out, &(point->lat), "NS");
  fprintf(out, " ");
  DEM_PrintCoord(out, &(point->lon), "EW");
}


double DEM_DMSCtoFloat(struct DMScoord * coord)
{
  double ret;

  int sign;

  sign = ((coord->deg & DEM_SIGN_MASK) != 0);

  ret = (float) (coord->deg & ~DEM_SIGN_MASK);
  ret += ((float) coord->min) / 60.0;
  ret += ((float) coord->sec) / 3600.0;

  if(sign) return -ret;
  else return ret;
}



void DEM_DsecstoDMS(struct DMScoord * coord, double secs)
{
  int sign;
  double mysecs;
  double fdegs, fmins, fsecs;

  if(secs < 0.0) {
    mysecs = -secs;
    sign = 1;
  }
  else {
    mysecs = secs;
    sign = 0;
  }

  fdegs = mysecs / 3600.0;

  fmins = (fdegs - floor(fdegs)) * 60.0;

  fsecs = (fmins - floor(fmins)) * 60.0;

  coord->deg = (unsigned short) floor(fdegs);
  coord->min = (unsigned char) floor(fmins);
  coord->sec = (unsigned char) floor(fsecs);

  if(sign) {
    coord->deg |= DEM_SIGN_MASK;
  }

  return;
    
}




