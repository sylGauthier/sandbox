#ifndef OCTREE_H
#define OCTREE_H

#include "phys_object.h"
#include "list.h"

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
struct PhysObject* phys_octree_object_collide(struct PhysOctree* octree, struct PhysObject* object);

void phys_octree_print(struct PhysOctree* octree);

#endif
