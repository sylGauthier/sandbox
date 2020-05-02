#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>

#include "phys_object.h"

void phys_object_init(struct PhysObject* object) {
    zero3v(object->aabbDimensions);
    zero3v(object->pos);
    quaternion_load_id(object->orientation);
    load_id4(object->transform);
    object->dynamic = 0;
    object->type = PHYS_EMPTY;
}

void phys_object_update_transform(struct PhysObject* obj) {
    quaternion_to_mat4(obj->transform, obj->orientation);
    memcpy(obj->transform[3], obj->pos, sizeof(Vec3));
}

static void update_box_aabb(struct PhysObject* box) {
    int i, j;
    Vec3 tmp1, tmp2;
    zero3v(box->aabbDimensions);
    for (i = 0; i < 8; i++) {
        tmp1[0] = (2 * ((i >> 0) & 1) - 1) * box->data.dimensions[0];
        tmp1[1] = (2 * ((i >> 1) & 1) - 1) * box->data.dimensions[1];
        tmp1[2] = (2 * ((i >> 2) & 1) - 1) * box->data.dimensions[2];
        mul4m3v(tmp2, MAT_CONST_CAST(box->transform), tmp1);
        for (j = 0; j < 3; j++) {
            box->aabbDimensions[j] = fabs(tmp2[j] - box->pos[j]) > box->aabbDimensions[j]
                                   ? fabs(tmp2[j] - box->pos[j]) : box->aabbDimensions[j];
        }
    }
}

static void update_sphere_aabb(struct PhysObject* sphere) {
    sphere->aabbDimensions[0] = sphere->data.radius;
    sphere->aabbDimensions[1] = sphere->data.radius;
    sphere->aabbDimensions[2] = sphere->data.radius;
}

void phys_object_update_aabb(struct PhysObject* obj) {
    switch (obj->type) {
        case PHYS_EMPTY:
            break;
        case PHYS_BOX:
            update_box_aabb(obj);
            break;
        case PHYS_SPHERE:
            update_sphere_aabb(obj);
            break;
    }
}

struct PhysObject* phys_object_new() {
    struct PhysObject* ret = NULL;
    if ((ret = malloc(sizeof(*ret)))) {
        phys_object_init(ret);
    }
    return ret;
}

struct PhysObject* phys_object_new_box(Vec3 dimensions) {
    struct PhysObject* ret;
    if ((ret = malloc(sizeof(*ret)))) {
        phys_object_init(ret);
        ret->type = PHYS_BOX;
        memcpy(ret->data.dimensions, dimensions, sizeof(Vec3));
        update_box_aabb(ret);
    }
    return ret;
}

struct PhysObject* phys_object_new_sphere(float radius) {
    struct PhysObject* ret;
    if ((ret = malloc(sizeof(*ret)))) {
        phys_object_init((void*)ret);
        ret->type = PHYS_SPHERE;
        ret->data.radius = radius;
        update_sphere_aabb(ret);
    }
    return ret;
}

void phys_object_set_scale(struct PhysObject* obj, const Vec3 scale) {
    switch (obj->type) {
        case PHYS_EMPTY:
            break;
        case PHYS_BOX:
            obj->data.dimensions[0] *= scale[0];
            obj->data.dimensions[1] *= scale[1];
            obj->data.dimensions[2] *= scale[2];
            update_box_aabb(obj);
            break;
        case PHYS_SPHERE:
            fprintf(stderr, "Warning: phys_object_set_scale: rescaling a sphere is not supported\n");
            break;
    }
}

void phys_object_set_pos(struct PhysObject* obj, const Vec3 pos) {
    memcpy(obj->pos, pos, sizeof(Vec3));
}

void phys_object_set_orientation(struct PhysObject* obj, const Quaternion orientation) {
    memcpy(obj->orientation, orientation, sizeof(Quaternion));
}

void phys_object_update(struct PhysObject* obj) {
    phys_object_update_transform(obj);
    phys_object_update_aabb(obj);
}

void phys_object_free(struct PhysObject* obj) {
    switch (obj->type) {
        case PHYS_EMPTY:
        case PHYS_BOX:
        case PHYS_SPHERE:
            free(obj);
            break;
    }
}
