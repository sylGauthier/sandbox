#include <stdlib.h>

#include "phys_solver.h"
#include "list.h"

struct Collision {
    struct PhysObject* object;
    Vec3 correction;
};

static int _get_close_objects(struct List** objects, struct PhysOctreeCell* cell, struct PhysObject* object) {
    unsigned int added = 0;
    if (cell->numObjects < 2) {
        return 0;
    } else if (!phys_cell_object_collide(cell, object->pos, object->aabbDimensions)) {
        return 0;
    } else if (cell->depth == PHYS_OCTREE_MAX_DEPTH) {
        struct List* cur = cell->objects;
        do {
            if (cur->obj == object) continue;
            if (list_find(*objects, cur->obj)) continue;
            /* FIXME: correct error reporting */
            if (!list_push(objects, cur->obj)) return 0;
            added++;
        } while ((cur = cur->tail));
    } else {
        unsigned int i;
        for (i = 0; i < 8; i++) {
            added += _get_close_objects(objects, cell->children[i], object);
            if (added >= cell->numObjects - 1) break;
        }
    }
    return added;
}

static int get_close_objects(struct List** objects, struct PhysOctree* octree, struct PhysObject* object) {
    return _get_close_objects(objects, octree->root, object);
}

int phys_solve_object_move(struct PhysOctree* octree, struct PhysObject* object, Vec3 correction) {
    struct List* objects = NULL;
    unsigned int nObj;
    char moved = 0;

    correction[0] = 0; correction[1] = 0; correction[2] = 0;
    nObj = get_close_objects(&objects, octree, object);
    if (!nObj) return 1;
    while (objects) {
        Vec3 penetration;
        if (phys_object_collide(object, objects->obj, penetration)) {
            incr3v(correction, penetration);
            moved = 1;
        }
        list_pop(&objects);
    }
    return moved;
}
