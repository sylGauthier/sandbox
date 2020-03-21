#include <stdio.h>
#include <string.h>

#include <game/scene/opengex.h>

#include "map.h"

static char* dirname(char* path) {
    char* s;
    if ((s = strrchr(path, '/'))) {
        *s = 0;
        return path;
    }
    return NULL;
}

int map_load(struct Map* map, char* mapFilename, struct Scene* scene, struct LightManager* lmgr) {
    FILE* mapFile = NULL;
    int ok = 1, i;

    if (!(mapFile = fopen(mapFilename, "r"))) {
        fprintf(stderr, "Error: map: could not open file: %s\n", mapFilename);
        ok = 0;
    } else if (!ogex_load(&scene->root, mapFile, dirname(mapFilename), &map->sharedData, &map->metadata)) {
        fprintf(stderr, "Error: map: ogex load failed\n");
        ok = 0;
    } else {
        for (i = 0; i < map->metadata.numLightNodes; i++) {
            struct Node* n = map->metadata.lightNodes[i];
            switch (n->type) {
                case NODE_DLIGHT:
                    light_mgr_add_dlight_node(lmgr, n);
                    break;
                case NODE_PLIGHT:
                    light_mgr_add_plight_node(lmgr, n);
                    break;
                default:;
            }
        }
        light_mgr_update_all(lmgr);
        printf("%d lights - %d cameras\n", map->metadata.numLightNodes, map->metadata.numCameraNodes);
    }
    if (mapFile)
        fclose(mapFile);
    return ok;
}
