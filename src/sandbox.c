#include <string.h>

#include "sandbox.h"

static char* dirname(char* path) {
    char* s;
    if ((s = strrchr(path, '/'))) {
        *s = 0;
        return path;
    }
    return NULL;
}

static int load_map(struct Sandbox* sandbox, char* map) {
    FILE* mapFile = NULL;
    int ok = 1, i;

    if (!(mapFile = fopen(map, "r"))) {
        fprintf(stderr, "Error: map: could not open file: %s\n", map);
        ok = 0;
    } else if (!ogex_load(&sandbox->scene.root, mapFile, dirname(map), &sandbox->map.sharedData, &sandbox->map.metadata)) {
        fprintf(stderr, "Error: map: ogex load failed\n");
        ok = 0;
    }
    if (mapFile)
        fclose(mapFile);
    for (i = 0; i < sandbox->map.metadata.numLightNodes; i++) {
        struct Node* n = sandbox->map.metadata.lightNodes[i];
        switch (n->type) {
            case NODE_DLIGHT:
                if (sandbox->numDirectionalLights < MAX_DIRECTIONAL_LIGHTS) {
                    lights_buffer_object_update_dlight(&sandbox->scene.lights, n->data.dlight, sandbox->numDirectionalLights);
                    sandbox->dlights[sandbox->numDirectionalLights++] = n;
                } else {
                    fprintf(stderr, "Warning: directional lights limit exceeded\n");
                }
                break;
            case NODE_PLIGHT:
                if (sandbox->numPointLights < MAX_POINT_LIGHTS) {
                    lights_buffer_object_update_plight(&sandbox->scene.lights, n->data.plight, sandbox->numPointLights);
                    sandbox->plights[sandbox->numPointLights++] = n;
                } else {
                    fprintf(stderr, "Warning: point lights limit exceeded\n");
                }
                break;
            default:;
        }
    }
    {
        struct AmbientLight ambient = {0};
        lights_buffer_object_update_ambient(&sandbox->scene.lights, &ambient);
    }

    printf("%d lights - %d cameras\n", sandbox->map.metadata.numLightNodes, sandbox->map.metadata.numCameraNodes);
    return ok;
}

int sandbox_load(struct Sandbox* sandbox, char* character, char* map) {
    int sceneInit = 0, mapLoad = 0;

    if (!game_init(GAME_SHADERS_PATH)) {
        fprintf(stderr, "Error: failed to init game library\n");
        return 0;
    }

    sandbox->numDirectionalLights = 0;
    sandbox->numPointLights = 0;
    if (!(sandbox->viewer = viewer_new(1024, 768, "sandbox"))) {
        fprintf(stderr, "Error: failed to start viewer\n");
    } else if (!(sceneInit = scene_init(&sandbox->scene, NULL))) {
        fprintf(stderr, "Error: failed to init scene\n");
    } else if (!(mapLoad = load_map(sandbox, map))) {
        fprintf(stderr, "Error: failed to load map\n");
    } else {
        sandbox->running = 1;

        sandbox->viewer->callbackData = sandbox;
        sandbox->viewer->resize_callback = resize_callback;
        sandbox->viewer->key_callback = key_callback;
        sandbox->viewer->close_callback = close_callback;

        scene_update_nodes(&sandbox->scene, update_node, sandbox);
        lights_buffer_object_update_ndlight(&sandbox->scene.lights, sandbox->numDirectionalLights);
        lights_buffer_object_update_nplight(&sandbox->scene.lights, sandbox->numPointLights);

        sandbox->camera = sandbox->map.metadata.cameraNodes[0]->data.camera;
        camera_set_ratio(((float)sandbox->viewer->width) / ((float)sandbox->viewer->height), sandbox->camera->projection);
        camera_buffer_object_update_projection(&sandbox->scene.camera, MAT_CONST_CAST(sandbox->camera->projection));
        camera_buffer_object_update_view_and_position(&sandbox->scene.camera, MAT_CONST_CAST(sandbox->camera->view));

        uniform_buffer_send(&sandbox->scene.lights);
        uniform_buffer_send(&sandbox->scene.camera);
        glfwSwapInterval(1);
        return 1;
    }
    if (sandbox->viewer) {
        viewer_free(sandbox->viewer);
    }
    if (sceneInit) {
        scene_free(&sandbox->scene, free_node_callback);
    }
    return 0;
}

int sandbox_run(struct Sandbox* sandbox) {
    double dt;

    viewer_process_events(sandbox->viewer);
    dt = viewer_next_frame(sandbox->viewer);

    scene_update_render_queue(&sandbox->scene, MAT_CONST_CAST(sandbox->camera->view), MAT_CONST_CAST(sandbox->camera->projection));
    scene_render(&sandbox->scene);
    return 1;
}
