//
//  GridMesh.h
//  PolyTest
//
//  Created by Jonathan deWerd on 6/20/14.
//  Copyright (c) 2014 a.b.c. All rights reserved.
//

#ifndef __PolyTest__GridMesh__
#define __PolyTest__GridMesh__

#define ENABLE_GLUT_DEMO

#include <iostream>
#include <vector>
#if defined(ENABLE_GLUT_DEMO)
#include <GLUT/glut.h>
#endif

struct GreinerV2f {
	float x,y;
	int first, prev, next; // First,prev,next verts in the *same* polygon
	int next_poly;   // First vertex of the *next* polygon
	float alpha; // If this vertex came from an affine comb, this is the mixing factor
	bool is_intersection:1; // True if this vertex was added at an intersection
	bool is_interior:1;
	bool is_entry:1;
	bool is_used:1;
	char corner; // 1=ll, 2=lr, 3=ur, 4=ul, 0 = none
	short tmp;
	int neighbor; // Corresp. vertex at same {x,y} in different polygon
	
	GreinerV2f() :	next(0), prev(0),
					next_poly(0), neighbor(0), first(0),
					is_intersection(false), is_interior(true), is_entry(false),
					is_used(false), corner(0), tmp(0) {};
};

struct IntersectingEdge {
	double x,y,alpha1;
	int e2; // e2 and v[e2].next make up the intersecting edge
	int cellidx; // index of cell along the
	IntersectingEdge(double x_, double y_, double a_, int v_, int ci_) : x(x_), y(y_), alpha1(a_), e2(v_), cellidx(ci_) {}
};


// 13
// 02
#define KNOWN_CORNER(i) (1<<((i)*2))
#define KNOWN_CORNER_EXTERIOR(i) (1<<((i)*2+1))
#define KNOWN_CORNER_LL (1<<0)
#define KNOWN_CORNER_LL_EXTERIOR (1<<1)
#define KNOWN_CORNER_UL (1<<2)
#define KNOWN_CORNER_UL_EXTERIOR (1<<3)
#define KNOWN_CORNER_LR (1<<4)
#define KNOWN_CORNER_LR_EXTERIOR (1<<5)
#define KNOWN_CORNER_UR (1<<6)
#define KNOWN_CORNER_UR_EXTERIOR (1<<7)
#define KNOWN_CORNER_NEXTX(kc) ((kc)>>4)
#define KNOWN_CORNER_NEXTY(kc) ((kc)>>2)&0x33
typedef unsigned char known_corner_t;

struct GridMesh {
	static float tolerance;
	// Vertex storage. Example: "int prev" in a GreinerV2f refers to v[prev].
	// v[0] is defined to be invalid and filled with the telltale location (-1234,-1234)
	GreinerV2f *v;
	long v_capacity;
	long v_count; // Includes the "bad" vertex #0
	double llx, lly, urx, ury; // Coordinates of lower left and upper right grid corners
	double dx, dy; // Width of a cell in the x, y directions
	double inv_dx, inv_dy; // 1/(width of a cell), 1/(height of a cell)
	int nx, ny; // Number of cells in the x and y directions
	
	GridMesh(double lowerleft_x, double lowerleft_y,
			 double upperright_x, double upperright_y,
			 int num_x_cells, int num_y_cells);
	void set_ll_ur(double lowerleft_x, double lowerleft_y,
				   double upperright_x, double upperright_y);
	
	// Basic vertex and polygon manipulation
	int vert_new();
	int vert_new(int prev, int next); // Make a new vert in the middle of an existing poly
	int vert_id(GreinerV2f *vert) {return vert?int(vert-v):0;}
	int vert_neighbor_on_poly(int vert, int poly);
	void vert_add_neighbor(int vert, int new_neighbor);
	std::pair<int,int> vert_grid_cell(int vert);
	int poly_for_cell(int x, int y);
	int poly_for_cell(float x, float y);
	int poly_first_vert(int anyvert);
	int poly_last_vert(int anyvert);
	int poly_next(int anyvert);
	int poly_vert_at(int anyvert, float x, float y);
	int poly_num_edges(int poly);
	bool poly_is_cyclic(int poly);
	void poly_set_cyclic(int poly, bool cyclic);
	void poly_set_interior(int poly, bool interior);
	void poly_grid_BB(int poly, int *bb);  //int bb[] = {minx,maxx,miny,maxy}
	
	// Trimming
	bool point_in_polygon(double x, double y, int poly);
	std::vector<IntersectingEdge> edge_poly_intersections(int e1, int p);
	int insert_vert(int poly1left,
					int poly1right,
					int poly2left,
					int poly2right,
					double x1, double y1
					);
	void find_cell_line_intersections(double x0, double y0, double x1, double y1,
									  std::vector<std::pair<int,int>> *bottom_edges,
									  std::vector<std::pair<int,int>> *left_edges,
									  std::vector<std::pair<int,int>> *integer_cells);
	// High level booleans
	void bool_AND(int poly2); // gridmesh -> gridmesh (intersection) poly2
	void bool_SUB(int poly2); // gridmesh -> gridmesh (intersection) ~poly2
	// Low level boolean support algorithms
	// Step 1: insert verts at intersections
	int insert_vert_poly_gridmesh(int poly); // Returns # of vertices inserted.
	// Step 2: find mutual entry/exit points
	void label_interior_AND(int poly2, bool invert_poly2=false, int *bb=NULL);
	void label_interior_SUB(int poly2, int *bb=NULL);
	void label_exterior_cells(int poly, bool interior_lbl, int* bb=NULL);
	known_corner_t label_interior_cell(int cell, int poly2, bool bool_SUB, known_corner_t kin);
	void label_interior_freepoly(int poly);
	// Step 3: perform the actual trim
	void trim_to_odd(int *bb=NULL);
	void trim_to_odd(int poly); // Trim one grid poly, leaving behind parts w/odd winding# in .next_poly linked list
#if defined(ENABLE_GLUT_DEMO)
	// Draw
	void poly_center(int poly, float *cx, float *cy);
	void poly_draw(int poly, float shrinkby, int maxedges=false);
#endif
};

// Backend
void find_integer_cell_line_intersections(double x0, double y0, double x1, double y1,
										  std::vector<std::pair<int,int>> *bottom_edges,
										  std::vector<std::pair<int,int>> *left_edges,
										  std::vector<std::pair<int,int>> *integer_cells);

int line_line_intersection(double ax, double ay, // Line 1, vert 1 A
						   double bx, double by, // Line 1, vert 2 B
						   double cx, double cy, // Line 2, vert 1 C
						   double dx, double dy, // Line 2, vert 2 D
						   double *ix, double *iy, // Intersection point
						   double *alpha1
						   );


#endif
