#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "phys_octree.h"

#define SUB_CELL_POS(pos, fpos, size, i) (pos)[0] = (fpos)[0] + (2 * (((i) >> 2) & 1) - 1) * (size); \
                                         (pos)[1] = (fpos)[1] + (2 * (((i) >> 1) & 1) - 1) * (size); \
                                         (pos)[2] = (fpos)[2] + (2 * (((i) >> 0) & 1) - 1) * (size); \

static void print_cell(struct PhysOctreeCell* cell) {
    unsigned int i;
    if (!cell) return;
    for (i = 0; i < cell->depth; i++) printf("    ");
    printf("pos: %.2f %.2f %.2f | radius: %.2f | num objects: %d",
            cell->pos[0], cell->pos[1], cell->pos[2], cell->radius, cell->numObjects);
    if (cell->numObjects == 1) {
        struct PhysObject* obj = cell->objects->obj;
        printf(" | ");
        switch (obj->type) {
            case PHYS_EMPTY:
                printf("empty");
                break;
            case PHYS_BOX:
                printf("box");
                break;
            case PHYS_SPHERE:
                printf("sphere");
                break;
        }
    }
    printf("\n");
    for (i = 0; i < 8; i++) print_cell(cell->children[i]);
}

void phys_octree_print(struct PhysOctree* octree) {
    print_cell(octree->root);
}

static struct PhysObjectList* object_list_new() {
    struct PhysObjectList* ret;
    if ((ret = malloc(sizeof(*ret)))) {
        ret->obj = NULL;
        ret->tail = NULL;
    }
    return ret;
}

static int object_list_push(struct PhysObjectList** list, struct PhysObject* obj) {
    struct PhysObjectList* newHead;
    if ((newHead = object_list_new())) {
        newHead->obj = obj;
        newHead->tail = *list;
        *list = newHead;
        return 1;
    }
    return 0;
}

static struct PhysObject* object_list_pop(struct PhysObjectList** list) {
    struct PhysObject* ret = NULL;
    struct PhysObjectList* tmp;
    if (*list) {
        tmp = *list;
        ret = tmp->obj;
        *list = tmp->tail;
        free(tmp);
    }
    return ret;
}

static struct PhysOctreeCell* new_octree_cell(Vec3 pos, float radius, unsigned int depth) {
    struct PhysOctreeCell* ret;

    if ((ret = malloc(sizeof(*ret)))) {
        unsigned int i;
        for (i = 0; i < 8; i++) ret->children[i] = NULL;
        ret->father = NULL;
        ret->objects = NULL;
        ret->numObjects = 0;
        ret->depth = depth;
        memcpy(ret->pos, pos, sizeof(Vec3));
        ret->radius = radius;
    }
    return ret;
}

static int cell_object_collide(struct PhysOctreeCell* cell, struct PhysObject* object) {
    return fabs(cell->pos[0] - object->pos[0]) <= cell->radius + object->aabbDimensions[0]
        && fabs(cell->pos[1] - object->pos[1]) <= cell->radius + object->aabbDimensions[1]
        && fabs(cell->pos[2] - object->pos[2]) <= cell->radius + object->aabbDimensions[2];
}

static int cell_object_cover(struct PhysOctreeCell* cell, struct PhysObject* object) {
    return fabs(cell->pos[0] - object->pos[0]) + object->aabbDimensions[0] <= cell->radius
        && fabs(cell->pos[1] - object->pos[1]) + object->aabbDimensions[1] <= cell->radius
        && fabs(cell->pos[2] - object->pos[2]) + object->aabbDimensions[2] <= cell->radius;
}

static int cell_add_object(struct PhysOctreeCell* cell, struct PhysObject* object);

/* cell should contain exactly one object (only reason why we would split would be to add another object) */
static int cell_split(struct PhysOctreeCell* cell) {
    struct PhysObject* obj;
    int i;
    obj = object_list_pop(&cell->objects);
    for (i = 0; i < 8; i++) {
        Vec3 pos;
        SUB_CELL_POS(pos, cell->pos, cell->radius / 2, i);
        if (!(cell->children[i] = new_octree_cell(pos, cell->radius / 2, cell->depth + 1))) return 0;
        cell->children[i]->father = cell;
        if (cell_object_collide(cell->children[i], obj)
                && !cell_add_object(cell->children[i], obj)) return 0;
    }
    return 1;
}

static int cell_add_object(struct PhysOctreeCell* cell, struct PhysObject* object) {
    unsigned int i;
    if (cell->numObjects == 0 || cell->depth >= PHYS_OCTREE_MAX_DEPTH) {
        if (!object_list_push(&cell->objects, object)) return 0;
        cell->numObjects++;
        return 1;
    } else if (cell->numObjects == 1) {
        if (!cell_split(cell)) return 0;
    }
    cell->numObjects++;
    for (i = 0; i < 8; i++) {
        if (cell_object_collide(cell->children[i], object) && !cell_add_object(cell->children[i], object)) {
            return 0;
        }
    }
    return 1;
}

int phys_octree_object_add(struct PhysOctree* octree, struct PhysObject* object) {
    if (!cell_object_cover(octree->root, object)) {
        fprintf(stderr, "Error: object outside of octree\n");
        printf("Cell pos:\n");
        print3v(octree->root->pos);
        printf("Cell radius: %f\n", octree->root->radius);
        printf("Object pos:\n");
        print3v(object->pos);
        printf("Object dimensions:\n");
        print3v(object->aabbDimensions);
        return 0;
    }

    return cell_add_object(octree->root, object);
}

int phys_octree_init(struct PhysOctree* octree, Vec3 pos, float radius) {
    if ((octree->root = new_octree_cell(pos, radius, 0))) {
        return 1;
    } else {
        return 0;
    }
}
