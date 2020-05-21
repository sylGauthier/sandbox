#include <3dmr/math/linear_algebra.h>
#include <3dmr/math/quaternion.h>

#ifndef PHYSIC_H
#define PHYSIC_H

struct PhysObject {
    Vec3 aabbDimensions;
    Vec3 pos;
    Quaternion orientation;
    Mat4 transform;
    char dynamic;

    enum PhysObjectType {
        PHYS_EMPTY,
        PHYS_BOX,
        PHYS_SPHERE
    } type;
    union PhysObjectData {
        Vec3 dimensions;
        float radius;
    } data;
};

struct PhysObject* phys_object_new();
struct PhysObject* phys_object_new_box(Vec3 dimensions);
struct PhysObject* phys_object_new_sphere(float radius);

void phys_object_set_scale(struct PhysObject* obj, const Vec3 scale);
void phys_object_set_pos(struct PhysObject* obj, const Vec3 pos);
void phys_object_set_orientation(struct PhysObject* obj, const Quaternion orientation);

void phys_object_update_transform(struct PhysObject* obj);
void phys_object_update_aabb(struct PhysObject* obj);
void phys_object_update(struct PhysObject* obj);

void phys_object_free(struct PhysObject* object);

int phys_object_collide(struct PhysObject* obj1, struct PhysObject* obj2, Vec3 penetration);

#endif
