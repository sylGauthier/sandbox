#ifndef RENDER_OCTREE_H
#define RENDER_OCTREE_H

#include "phys_octree.h"

void render_octree_init();
void render_octree(struct PhysOctree* octree);
void render_octree_cleanup();

#endif
