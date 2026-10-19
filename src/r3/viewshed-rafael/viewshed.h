// your name 
// oct 2023

#ifndef __vis_hpp
#define __vis_hpp


#include "grid.h"

#define NOT_VISIBLE -1 


// compute viewshed of (r,c,h) and populate  vis_grid with values 
void compute_viewshed_grid(const Grid* elev_grid, Grid*  vis_grid,
			   const int vr, const int vc, const float vh);



#endif 



