
/**
 * OS Coord: A Simple OS Coordinate Transformation Library for C
 *
 * This is a port of a the Javascript library produced by Chris Veness available
 * from http://www.movable-type.co.uk/scripts/latlong-gridref.html.
 */

#include <math.h>

#include "os_coord.h"
#include "os_coord_ordinance_survey.h"

/**
 * Convert an upper-case grid-ref character to the index within a 5x5 grid
 * (skipping 'I').
 */
#define C2I(c) ((c) - 'A' - (((c) > 'I') ? 1 : 0))

/**
 * Convert an index within a 5x5 grid into an upper-case letter (skipping 'I').
 */
#define I2C(i) ('A' + (((i) >= 8) ? (i)+1 : (i)))

/**
 * Convert an index within a 5x5 grid into the x-offset into the grid.
 */
#define I2X(i) ((i)%5)

/**
 * Convert an index within a 5x5 grid into the y-offset into the grid.
 */
#define I2Y(i) (4 - ((i)/5))

/**
 * Convert an x,y pair into an index into a 5x5 grid.
 */
#define XY2I(x,y) ((x) + ((4-(y)) * 5))


os_grid_ref_t
os_eas_nor_to_grid_ref( os_eas_nor_t point
                      , os_grid_t grid)
{
	// An invalid grid reference (to start with)
	os_grid_ref_t grid_ref = {.code={'\0','\0','\0'}};
	
	// Figure out the coordinate of the square in the grid
	unsigned int sq_x = point.e / 100000;
	unsigned int sq_y = point.n / 100000;
	
	// Check that the point is within the range covered by the grid-reference, if
	// not return an invalid grid reference.
	if (sq_x < 0 || sq_y < 0 || sq_x >= grid.width || sq_y >= grid.height) {
		return grid_ref;
	}
	
	// Work out each digit of the grid code
	for (int i = grid.num_digits - 1; i >= 0; i--) {
		// First digit may be offset in its grid
		int off_x = (i == 0) ? I2X(C2I(grid.bottom_left_first_char)) : 0;
		int off_y = (i == 0) ? I2Y(C2I(grid.bottom_left_first_char)) : 0;
		
		// Figure out the letter for this digit
          
		grid_ref.code[i] = I2C(XY2I(off_x + (sq_x%5), off_y + (sq_y%5)));		
		// "Shift" off the digit
		sq_x /= 5;
		sq_y /= 5;
	}
	
	// Work out the other fields
	grid_ref.e = fmod(point.e, 100000.0);
	grid_ref.n = fmod(point.n, 100000.0);
	grid_ref.h = point.h;
	
	return grid_ref;
}

