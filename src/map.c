#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <3dmr/scene/gltf.h>

#include "map.h"
#include "utils/utils.h"

static void map_init(struct Map* map) {
    import_init_metadata(&map->metadata);
    import_init_shared_data(&map->sharedData);
    map->objects = NULL;
    map->numObjects = 0;
}

static int map_add_object(struct Map* map, struct PhysObject* obj) {
    void* tmp;
    if (!(tmp = realloc(map->objects, (map->numObjects + 1) * sizeof(*map->objects))))
        return 0;
    map->objects = tmp;
    map->objects[map->numObjects++] = obj;
    return 1;
}

static int load_phys_objects(struct Map* map, struct Node* root) {
    unsigned int i;
    struct PhysObject* obj = NULL;
    if (root->name && !strncmp(root->name, "phys_box", 8)) {
        Vec3 dims;
        for (i = 0; i < 3; i++) {
            dims[i] = fabs(root->boundingBox[0][i]) > fabs(root->boundingBox[1][i]) ?
                      fabs(root->boundingBox[0][i]) : fabs(root->boundingBox[1][i]);
        }
        if (!(obj = phys_object_new_box(dims))) return 0;
    } else if (root->name && !strncmp(root->name, "phys_sphere", 11)) {
        float radius = 0.;
        for (i = 0; i < 6; i++) {
            radius = fabs(root->boundingBox[i / 3][i % 3]) > radius ?
                     fabs(root->boundingBox[i / 3][i % 3]) : radius;
        }
        if (!(obj = phys_object_new_sphere(radius))) return 0;
    }
    if (obj) {
        phys_object_set_scale(obj, root->scale);
        phys_object_set_orientation(obj, root->orientation);
        phys_object_set_pos(obj, root->position);
        phys_object_update(obj);
    }
    if (obj && !map_add_object(map, obj)) {
        phys_object_free(obj);
        return 0;
    }
    for (i = 0; i < root->nbChildren; i++) {
        if (!load_phys_objects(map, root->children[i])) return 0;
    }
    return 1;
}

int map_load(struct Map* map, char* mapFilename, struct Scene* scene, struct LightManager* lmgr) {
    FILE* mapFile = NULL;
    int ok = 1, i;

    map_init(map);

    if (!(mapFile = fopen(mapFilename, "r"))) {
        fprintf(stderr, "Error: map: could not open file: %s\n", mapFilename);
        ok = 0;
    } else if (!gltf_load(&scene->root, mapFile, dirname(mapFilename), &map->sharedData, &map->metadata, 1)) {
        fprintf(stderr, "Error: map: ogex load failed\n");
        ok = 0;
    } else if (!load_phys_objects(map, &scene->root)) {
        fprintf(stderr, "Error: map: physic load failed\n");
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
                case NODE_SLIGHT:
                    light_mgr_add_slight_node(lmgr, n);
                    break;
                default:;
            }
        }
        printf("%d lights - %d cameras\n", map->metadata.numLightNodes, map->metadata.numCameraNodes);
        printf("%d physic objects\n", map->numObjects);
    }
    if (mapFile)
        fclose(mapFile);
    return ok;
}

void map_free(struct Map* map) {
    if (map) {
        unsigned int i;
        import_free_metadata(&map->metadata);
        import_free_shared_data(&map->sharedData);
        for (i = 0; i < map->numObjects; i++) {
            phys_object_free(map->objects[i]);
        }
        free(map->objects);
    }
}
