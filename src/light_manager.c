#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "light_manager.h"

int light_mgr_add_plight_node(struct LightManager* lmgr, struct Node* plight) {
    if (lmgr->numPointLights < MAX_POINT_LIGHTS) {
        lights_buffer_object_update_plight(&lmgr->scene->lights, plight->data.plight, lmgr->numPointLights);
        lmgr->plights[lmgr->numPointLights++] = plight;
        lights_buffer_object_update_nplight(&lmgr->scene->lights, lmgr->numPointLights);
        return 1;
    }
    fprintf(stderr, "Warning: point lights limit exceeded\n");
    return 0;
}

int light_mgr_add_dlight_node(struct LightManager* lmgr, struct Node* dlight) {
    if (lmgr->numDirectionalLights < MAX_DIRECTIONAL_LIGHTS) {
        lights_buffer_object_update_dlight(&lmgr->scene->lights, dlight->data.dlight, lmgr->numDirectionalLights);
        lmgr->dlights[lmgr->numDirectionalLights++] = dlight;
        lights_buffer_object_update_ndlight(&lmgr->scene->lights, lmgr->numDirectionalLights);
        return 1;
    }
    fprintf(stderr, "Warning: directional lights limit exceeded\n");
    return 0;
}

int light_mgr_add_slight_node(struct LightManager* lmgr, struct Node* slight) {
    if (lmgr->numSpotLights < MAX_SPOT_LIGHTS) {
        lights_buffer_object_update_slight(&lmgr->scene->lights, slight->data.slight, lmgr->numSpotLights);
        lmgr->slights[lmgr->numSpotLights++] = slight;
        lights_buffer_object_update_nslight(&lmgr->scene->lights, lmgr->numSpotLights);
        return 1;
    }
    fprintf(stderr, "Warning: directional lights limit exceeded\n");
    return 0;
}

void light_mgr_set_ambient(struct LightManager* lmgr, Vec3 color) {
    struct AmbientLight ambient;
    ambient.color[0] = color[0];
    ambient.color[1] = color[1];
    ambient.color[2] = color[2];
    lights_buffer_object_update_ambient(&lmgr->scene->lights, &ambient);
}

void light_mgr_init(struct LightManager* lmgr, struct Scene* scene) {
    lmgr->numPointLights = 0;
    lmgr->numDirectionalLights = 0;
    lmgr->numSpotLights = 0;
    memset(lmgr->ambient.color, 0, sizeof(lmgr->ambient.color));
    lmgr->scene = scene;
}
