#include <stdlib.h>
#include <stdio.h>

#include "phys_object.h"

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

static int collide_sphere_box(struct PhysObject* sphere, struct PhysObject* box, Vec3 penetration) {
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
