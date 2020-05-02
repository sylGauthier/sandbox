#ifndef OCTREE_H
#define OCTREE_H

#include "phys_object.h"
#include "../utils/list.h"

#define PHYS_OCTREE_MAX_DEPTH 6

struct PhysOctreeCell {
    struct PhysOctreeCell* children[8];
    struct PhysOctreeCell* father;
    struct List* objects;
    unsigned int numObjects;
    unsigned int depth;
    Vec3 pos;
    float radius;
};

struct PhysOctree {
    struct PhysOctreeCell* root;
};

int phys_octree_init(struct PhysOctree* octree, Vec3 pos, float radius);
int phys_octree_object_add(struct PhysOctree* octree, struct PhysObject* object);
int phys_octree_object_move(struct PhysOctree* octree, struct PhysObject* object, Vec3 newPos);
int phys_octree_object_translate(struct PhysOctree* octree, struct PhysObject* object, Vec3 dir);

int phys_cell_object_collide(struct PhysOctreeCell* cell, Vec3 pos, Vec3 dims);
int phys_cell_object_cover(struct PhysOctreeCell* cell, Vec3 pos, Vec3 dims);

void phys_octree_print(struct PhysOctree* octree);

#endif
