#ifndef PHYS_SOLVER_H
#define PHYS_SOLVER_H

#include "phys_octree.h"

int phys_solve_object_move(struct PhysOctree* octree, struct PhysObject* object, Vec3 correction);

#endif
