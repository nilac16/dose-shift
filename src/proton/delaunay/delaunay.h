#pragma once

#ifndef DELAUNAY_H
#define DELAUNAY_H

/** TODO: URGENT: Inspect the code that calculates the lower common tangent
 *  I was lazy with it and never proved it to be correct
 * 
 *  NOTURGENT: Does this terminate for point sets with non-unique 
 *  triangulations */

#if __cplusplus
#   error("Delaunay header cannot be #included in C++ files")
#endif

/** This algorithm was written in June of 2021. I have heavily modified it 
 *  from its original form to work with Microsoft's outdated C compiler. It 
 *  will not be declared extern "C", because I have no intention of invoking 
 *  this code from any C++ files
 */

/** A complete triangulation will only ever have 2 * n - 2 triangles. Since this 
 *  algorithm never actually deletes them, I use a stack allocator here.
 * 
 *  Consider a triangulation embedded on the surface of a closed manifold. If we 
 *  include manifold triangles ("ghost" triangles) then each edge is bordered by 
 *  two triangles, guaranteed. Since each triangle is guaranteed 
 *  to contain three edges:
 *                                 2e = 3t.
 * 
 *  A triangulation on a closed manifold forms a spherical polyhedron. The 
 *  Euler characteristic
 *                          (n + 1) - e + t = 2
 * 
 *  shows this triangle limit immediately (Dwyer, R. A. (1986)). In Dwyer's 
 *  paper, he added 1 to t to account for the manifold face. In this case, I 
 *  add 1 to n instead, to account for the manifold vertex.
 */


struct delaunay_triangle_pool {
    struct delaunay_triangle *end;
    struct delaunay_triangle {
        const double *vertices[3];
        double circumcenter[3];
        struct {
            struct delaunay_triangle *next;
            int next_side;
        } adjacents[3];
    } start[];
};


/// Forms a triangulation using the @p N nodes in the array at @p nodes. The 
/// array is assumed to contain 2-tuples of nodes, so every 16 bytes 
/// corresponds to a new node.
///
/** CHANGES:
 *   - Remove threading, it's not portable and pollutes the source file
 *   - Remove size_t, use unsigned ints. Detector is not that big
 *   - Remove static array initializers (thanks micro$oft)
 *   - Each node shall be 24 bytes, containing an additional double with 
 *     dose information
 */
struct delaunay_triangle_pool *triangulate(unsigned int N,
                                           const double nodes[/* static 3 * N */]/* ,
                                           unsigned int n_threads */);


void free_triangle_pool(struct delaunay_triangle_pool *p);

int delaunay_is_manifold_tri(const struct delaunay_triangle *t);


#endif //DELAUNAY_H
