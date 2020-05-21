#ifndef LIGHT_MAnAGER_H
#define LIGHT_MAnAGER_H

#include <3dmr/scene/scene.h>
#include <3dmr/render/lights_buffer_object.h>

struct LightManager {
    unsigned int numDirectionalLights, numPointLights, numSpotLights;
    struct Node *dlights[MAX_DIRECTIONAL_LIGHTS], *plights[MAX_POINT_LIGHTS], *slights[MAX_SPOT_LIGHTS];
    struct AmbientLight ambient;

    struct Scene* scene;
};

int light_mgr_add_plight_node(struct LightManager* lmgr, struct Node* plight);
int light_mgr_add_dlight_node(struct LightManager* lmgr, struct Node* dlight);
int light_mgr_add_slight_node(struct LightManager* lmgr, struct Node* slight);
void light_mgr_set_ambient(struct LightManager* lmgr, Vec3 color);
void light_mgr_init(struct LightManager* lmgr, struct Scene* scene);

#endif
