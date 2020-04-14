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

static int object_list_delete(struct PhysObjectList** list, struct PhysObject* obj) {
    struct PhysObjectList* prev = NULL;
    struct PhysObjectList* cur = *list;
    while (cur && cur->obj != obj) {
        prev = cur;
        cur = cur->tail;
    }
    if (!cur) return 0;
    if (!prev) {
        object_list_pop(list);
        return 1;
    }
    prev->tail = cur->tail;
    free(cur);
    return 1;
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

static int cell_object_collide(struct PhysOctreeCell* cell, Vec3 pos, Vec3 dims) {
    return fabs(cell->pos[0] - pos[0]) <= cell->radius + dims[0]
        && fabs(cell->pos[1] - pos[1]) <= cell->radius + dims[1]
        && fabs(cell->pos[2] - pos[2]) <= cell->radius + dims[2];
}

static int cell_object_cover(struct PhysOctreeCell* cell, Vec3 pos, Vec3 dims) {
    return fabs(cell->pos[0] - pos[0]) + dims[0] <= cell->radius
        && fabs(cell->pos[1] - pos[1]) + dims[1] <= cell->radius
        && fabs(cell->pos[2] - pos[2]) + dims[2] <= cell->radius;
}

static int cell_add_object(struct PhysOctreeCell* cell, struct PhysObject* object);

/* cell should contain exactly one object (only reason why we would split would be to add another object) */
static int cell_split(struct PhysOctreeCell* cell) {
    struct PhysObject* obj;
    int i;
    if (!(obj = object_list_pop(&cell->objects))) {
        fprintf(stderr, "Error: cell_split: inconsistent state\n");
        return 0;
    }
    for (i = 0; i < 8; i++) {
        Vec3 pos;
        SUB_CELL_POS(pos, cell->pos, cell->radius / 2, i);
        if (!(cell->children[i] = new_octree_cell(pos, cell->radius / 2, cell->depth + 1))) return 0;
        cell->children[i]->father = cell;
        if (cell_object_collide(cell->children[i], obj->pos, obj->aabbDimensions)
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
        if (cell_object_collide(cell->children[i], object->pos, object->aabbDimensions)) {
            if (!cell_add_object(cell->children[i], object)) {
                return 0;
            }
            if (cell_object_cover(cell->children[i], object->pos, object->aabbDimensions)) {
                return 1;
            }
        }
    }
    return 1;
}

static int cell_remove_object(struct PhysOctreeCell* cell, struct PhysObject* object) {
    cell->numObjects--;
    if (cell->depth >= PHYS_OCTREE_MAX_DEPTH) {
        object_list_delete(&cell->objects, object);
        return 1;
    } else if (cell->numObjects == 0) {
        object_list_pop(&cell->objects);
        return 1;
    } else if (cell->numObjects == 1) {
        unsigned int i;
        struct PhysObject* remain = NULL;
        for (i = 0; i < 8; i ++) {
            if (cell->children[i]->numObjects == 1) {
                remain = object_list_pop(&cell->children[i]->objects);
                if (!cell->objects && remain != object) {
                    if (!object_list_push(&cell->objects, remain)) {
                        return 0;
                    }
                }
            } else if (cell->children[i]->numObjects == 2) {
                cell_remove_object(cell->children[i], object);
                remain = object_list_pop(&cell->children[i]->objects);
                if (!cell->objects && remain != object) {
                    if (!object_list_push(&cell->objects, remain)) {
                        return 0;
                    }
                }
            }
            free(cell->children[i]);
            cell->children[i] = NULL;
        }
    } else {
        unsigned int i;
        for (i = 0; i < 8; i++) {
            if (!cell->children[i]) printf("%d\n", cell->numObjects);
            if (cell_object_collide(cell->children[i], object->pos, object->aabbDimensions)
                    && !cell_remove_object(cell->children[i], object)) {
                return 0;
            }
        }
    }
    return 1;
}

/* static int cell_move_object(struct PhysOctreeCell* cell, struct PhysObject* object, Vec3 prevPos) {
    char wasHere, isHere;
    wasHere = cell_object_collide(cell, prevPos, object->aabbDimensions);
    isHere = cell_object_collide(cell, object->pos, object->aabbDimensions);

    if (!wasHere && !isHere) {
        return 1;
    } else if (!wasHere && isHere) {
        return cell_add_object(cell, object);
    } else if (wasHere && !isHere) {
        return cell_remove_object(cell, object);
    } else if (wasHere && isHere) {
        if (cell->children[0]) {
            unsigned int i;
            for (i = 0; i < 8; i++) {
                if (!cell_move_object(cell->children[i], object, prevPos)) {
                    return 0;
                }
            }
        }
    }
    return 1;
} */

int phys_octree_object_add(struct PhysOctree* octree, struct PhysObject* object) {
    if (!cell_object_cover(octree->root, object->pos, object->aabbDimensions)) {
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

int phys_octree_object_move(struct PhysOctree* octree, struct PhysObject* object, Vec3 newPos) {
    if (!cell_object_cover(octree->root, newPos, object->aabbDimensions)) {
        return 0;
    }
    if (!cell_remove_object(octree->root, object)) {
        return 0;
    }
    memcpy(object->pos, newPos, sizeof(Vec3));
    if (cell_add_object(octree->root, object)) {
        phys_object_update_transform(object);
        return 1;
    }
    return 0;
}

int phys_octree_init(struct PhysOctree* octree, Vec3 pos, float radius) {
    if ((octree->root = new_octree_cell(pos, radius, 0))) {
        return 1;
    } else {
        return 0;
    }
}
