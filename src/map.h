#ifndef MAP_H
#define MAP_H

#include <game/scene/scene.h>

#include "light_manager.h"
#include "phys/phys_object.h"

struct Map {
    struct ImportMetadata metadata;
    struct ImportSharedData sharedData;

    struct PhysObject** objects;
    unsigned int numObjects;
};

int map_load(struct Map* map, char* mapFilename, struct Scene* scene, struct LightManager* lmgr);
void map_free(struct Map* map);

#endif
