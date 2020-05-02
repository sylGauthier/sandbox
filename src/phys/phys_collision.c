#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "phys_object.h"

static float copysign(float amp, float sign) {
    return sign >= 0. ? fabs(amp) : -fabs(amp);
}

static int collide_sphere_sphere(struct PhysObject* sphere1, struct PhysObject* sphere2, Vec3 penetration) {
    float sqDist, sqRad, dist;
    sub3v(penetration, sphere1->pos, sphere2->pos);
    sqDist = norm3sq(penetration);
    sqRad = sphere1->data.radius + sphere2->data.radius;
    sqRad *= sqRad;
    if (sqDist > sqRad) return 0;
    dist = norm3(penetration);
    if (dist == 0.) return 0;
    scale3v(penetration, (sphere1->data.radius + sphere2->data.radius) / dist - 1.);
    return 1;
}

static int sb_test_faces(Vec3 dims, Vec3 pos, float radius, Vec3 penetration) {
    if (fabs(pos[1]) < dims[1] && fabs(pos[2]) < dims[2] && fabs(pos[0]) < dims[0] + radius) {
        penetration[0] = copysign(dims[0] + radius - fabs(pos[0]), pos[0]);
        penetration[1] = 0;
        penetration[2] = 0;
        return 1;
    }
    if (fabs(pos[0]) < dims[0] && fabs(pos[2]) < dims[2] && fabs(pos[1]) < dims[1] + radius) {
        penetration[1] = copysign(dims[1] + radius - fabs(pos[1]), pos[1]);
        penetration[0] = 0;
        penetration[2] = 0;
        return 1;
    }
    if (fabs(pos[0]) < dims[0] && fabs(pos[1]) < dims[1] && fabs(pos[2]) < dims[2] + radius) {
        penetration[2] = copysign(dims[2] + radius - fabs(pos[2]), pos[2]);
        penetration[0] = 0;
        penetration[1] = 0;
        return 1;
    }
    return 0;
}

static int sb_test_edges(Vec3 dims, Vec3 pos, float radius, Vec3 penetration) {
    Vec3 proj;
    float dist;
    if (fabs(pos[0]) > dims[0] && fabs(pos[1]) > dims[1] && fabs(pos[2]) <= dims[2]) {
        proj[0] = copysign(dims[0], pos[0]);
        proj[1] = copysign(dims[1], pos[1]);
        proj[2] = pos[2];
    }
    if (fabs(pos[0]) > dims[0] && fabs(pos[2]) > dims[2] && fabs(pos[1]) <= dims[1]) {
        proj[0] = copysign(dims[0], pos[0]);
        proj[2] = copysign(dims[2], pos[2]);
        proj[1] = pos[1];
    }
    if (fabs(pos[2]) > dims[2] && fabs(pos[1]) > dims[1] && fabs(pos[0]) <= dims[0]) {
        proj[2] = copysign(dims[2], pos[2]);
        proj[1] = copysign(dims[1], pos[1]);
        proj[0] = pos[0];
    }
    sub3v(penetration, pos, proj);
    if (norm3sq(penetration) >= radius * radius) return 0;
    dist = norm3(penetration);
    scale3v(penetration, radius / dist - 1.);
    return 1;
}

static int sb_test_vertices(Vec3 dims, Vec3 pos, float radius, Vec3 penetration) {
    Vec3 vert;
    float dist;
    vert[0] = copysign(dims[0], pos[0]);
    vert[1] = copysign(dims[1], pos[1]);
    vert[2] = copysign(dims[2], pos[2]);
    sub3v(penetration, pos, vert);
    if (norm3sq(penetration) >= radius * radius) return 0;
    dist = norm3(penetration);
    scale3v(penetration, radius / dist - 1.);
    return 1;
}

static int collide_sphere_box(struct PhysObject* sphere, struct PhysObject* box, Vec3 penetration) {
    Mat4 invModel;
    Mat3 model3;
    Vec3 relCoord, relPenetration;

    invert4m(invModel, MAT_CONST_CAST(box->transform));
    mul4m3v(relCoord, MAT_CONST_CAST(invModel), sphere->pos);
    if (fabs(relCoord[0]) < box->data.dimensions[0]
            && fabs(relCoord[1]) < box->data.dimensions[1]
            && fabs(relCoord[2]) < box->data.dimensions[2]) {
        return 1;
    }
    mat4to3(model3, MAT_CONST_CAST(box->transform));
    normalize3(model3[0]);
    normalize3(model3[1]);
    normalize3(model3[2]);
    if (       sb_test_faces(box->data.dimensions, relCoord, sphere->data.radius, relPenetration)
            || sb_test_edges(box->data.dimensions, relCoord, sphere->data.radius, relPenetration)
            || sb_test_vertices(box->data.dimensions, relCoord, sphere->data.radius, relPenetration)) {
        mul3mv(penetration, MAT_CONST_CAST(model3), relPenetration);
        return 1;
    }
    return 0;
}

static int collide_box_box(struct PhysObject* box1, struct PhysObject* box2, Vec3 penetration) {
    return 0;
}

int phys_object_collide(struct PhysObject* obj1, struct PhysObject* obj2, Vec3 penetration) {
    if (obj1->type == PHYS_EMPTY || obj2->type == PHYS_EMPTY) return 0;
    if (obj1->type == PHYS_SPHERE && obj2->type == PHYS_SPHERE) {
        return collide_sphere_sphere(obj1, obj2, penetration);
    }
    if (obj1->type == PHYS_SPHERE && obj2->type == PHYS_BOX) {
        return collide_sphere_box(obj1, obj2, penetration);
    }
    if (obj1->type == PHYS_BOX && obj2->type == PHYS_SPHERE) {
        if (!collide_sphere_box(obj2, obj1, penetration)) return 0;
        scale3v(penetration, -1);
        return 1;
    }
    if (obj1->type == PHYS_BOX && obj2->type == PHYS_BOX) {
        return collide_box_box(obj1, obj2, penetration);
    }
    return 0;
}
