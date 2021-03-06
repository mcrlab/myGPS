/**
 * OS Coord: A Simple OS Coordinate Transformation Library for C
 *
 * This is a port of a the Javascript library produced by Chris Veness available
 * from http://www.movable-type.co.uk/scripts/latlong-gridref.html.
 *
 * These functions are largely based on the guide in "A guide to coordinate
 * systems in Great Britain", Section 6.
 */

#include <math.h>

#include "os_coord.h"
#include "os_coord_transform.h"
#include "os_coord_math.h"

os_cartesian_t
os_lat_lon_to_cartesian( os_lat_lon_t   point 
                       , os_ellipsoid_t ellipsoid
                       )
{
	double sinPhi = sin(point.lat);
	double cosPhi = cos(point.lat);
	double sinLambda = sin(point.lon);
	double cosLambda = cos(point.lon);
	
	double eSq = ((ellipsoid.a*ellipsoid.a) - (ellipsoid.b*ellipsoid.b))
	             / (ellipsoid.a*ellipsoid.a);
	double nu = ellipsoid.a / sqrt(1.0 - (eSq*(sinPhi*sinPhi)));
	
	os_cartesian_t cart_point;
	cart_point.x = (nu+point.eh) * cosPhi * cosLambda;
	cart_point.y = (nu+point.eh) * cosPhi * sinLambda;
	cart_point.z = ((1.0-eSq)*nu + point.eh) * sinPhi;
	
	return cart_point;
}


os_lat_lon_t
os_cartesian_to_lat_lon( os_cartesian_t point
                       , os_ellipsoid_t ellipsoid
                       )
{
	// results accurate to around the given number of metres
	double precision = OS_CART_TO_LAT_LON_PRECISION / ellipsoid.a;
	
	double eSq = ((ellipsoid.a*ellipsoid.a) - (ellipsoid.b*ellipsoid.b))
	             / (ellipsoid.a*ellipsoid.a);
	double p = sqrt((point.x*point.x) + (point.y*point.y));
	double phi  = atan2(point.z, p*(1.0-eSq));
	double phiP = 2.0*PI;
	double nu;
	while (fabs(phi-phiP) > precision) {
	  nu   = ellipsoid.a / sqrt(1.0 - eSq*(sin(phi)*sin(phi)));
	  phiP = phi;
	  phi  = atan2(point.z + eSq*nu*sin(phi), p);
	}
	
	os_lat_lon_t lat_lon;
	lat_lon.lat = phi;
	lat_lon.lon = atan2(point.y, point.x);
	lat_lon.eh  = p/cos(phi) - nu;
	
	return lat_lon;
}




os_cartesian_t
os_helmert_transform( os_cartesian_t point 
                    , os_helmert_t   helmert
                    )
{
	// Normalise seconds to radians
	double rx = DEG_2_RAD(helmert.rx/3600.0);
	double ry = DEG_2_RAD(helmert.ry/3600.0);
	double rz = DEG_2_RAD(helmert.rz/3600.0);
	// Normalise ppm to (1+s)
	double s1 = 1+ (helmert.s/1000000.0);
	
	// Apply transform
	os_cartesian_t new_point;
	new_point.x = helmert.tx + point.x*s1 - point.y*rz + point.z*ry;
	new_point.y = helmert.ty + point.x*rz + point.y*s1 - point.z*rx;
	new_point.z = helmert.tz - point.x*ry + point.y*rx + point.z*s1;
	
	return new_point;
}


os_eas_nor_t
os_lat_lon_to_tm_eas_nor( os_lat_lon_t       point
                        , os_tm_projection_t projection
                        )
{
	// Convert to radians
	double lat0 = DEG_2_RAD(projection.lat0);
	double lon0 = DEG_2_RAD(projection.lon0);
	
	// Shorter-named alias
	double lat = point.lat;
	double lon = point.lon;
	double a = projection.ellipsoid.a;
	double b = projection.ellipsoid.b;
	
	double e2 = 1.0 - (b*b)/(a*a);
	
	double n = (a-b)/(a+b);
	double n2 = n*n;
	double n3 = n*n*n;
	
	double cosLat = cos(lat);
	double sinLat = sin(lat);
	
	// Transverse radius of curvature
	double nu = a*projection.f0/sqrt(1.0-e2*sinLat*sinLat);
	// Meridional radius of curvature
	double rho = a*projection.f0*(1.0-e2)/pow(1.0-e2*sinLat*sinLat, 1.5);
	double eta2 = nu/rho-1.0;
	
	double Ma = (1.0 + n + (5.0/4.0)*n2 + (5.0/4.0)*n3) * (lat-lat0);
	double Mb = (3.0*n + 3.0*n*n + (21.0/8.0)*n3) * sin(lat-lat0) * cos(lat+lat0);
	double Mc = ((15.0/8.0)*n2 + (15.0/8.0)*n3) * sin(2.0*(lat-lat0)) * cos(2.0*(lat+lat0));
	double Md = (35.0/24.0)*n3 * sin(3.0*(lat-lat0)) * cos(3.0*(lat+lat0));
	// Meridional arc
	double M = b * projection.f0 * (Ma - Mb + Mc - Md);
	
	double cos3lat = cosLat*cosLat*cosLat;
	double cos5lat = cos3lat*cosLat*cosLat;
	double tan2lat = tan(lat)*tan(lat);
	double tan4lat = tan2lat*tan2lat;
	
	double I = M + projection.n0;
	double II = (nu/2.0)*sinLat*cosLat;
	double III = (nu/24.0)*sinLat*cos3lat*(5.0-tan2lat+9.0*eta2);
	double IIIA = (nu/720.0)*sinLat*cos5lat*(61.0-58.0*tan2lat+tan4lat);
	double IV = nu*cosLat;
	double V = (nu/6.0)*cos3lat*(nu/rho-tan2lat);
	double VI = (nu/120.0) * cos5lat * (5.0 - 18.0*tan2lat + tan4lat + 14.0*eta2 - 58.0*tan2lat*eta2);
	
	double dLon = lon-lon0;
	double dLon2 = dLon*dLon;
	double dLon3 = dLon2*dLon;
	double dLon4 = dLon3*dLon;
	double dLon5 = dLon4*dLon;
	double dLon6 = dLon5*dLon;
	
	os_eas_nor_t eas_nor;
	eas_nor.n = I + II*dLon2 + III*dLon4 + IIIA*dLon6;
	eas_nor.e = projection.e0 + IV*dLon + V*dLon3 + VI*dLon5;
	eas_nor.h = point.eh;
	
	return eas_nor;
}

