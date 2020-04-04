#include <stdlib.h>

#include "sandbox.h"

void resize_callback(struct Viewer* viewer, void* data) {
    struct Sandbox* sandbox = data;
    struct Camera* activeCam = sandbox->camera->data.camera;
    glViewport(0, 0, viewer->width, viewer->height);
    camera_set_ratio(((float)viewer->width) / ((float)viewer->height), activeCam->projection);
    camera_buffer_object_update_projection(&sandbox->scene.camera, MAT_CONST_CAST(activeCam->projection));
    uniform_buffer_send(&sandbox->scene.camera);
}

void key_callback(struct Viewer* viewer, int key, int scancode, int action, int mods, void* data) {
    struct Sandbox* sandbox = data;
    static int camIdx = 0;
    switch (key) {
        case GLFW_KEY_ESCAPE:
            if (action != GLFW_PRESS) return;
            sandbox->running = 0;
            break;
        case GLFW_KEY_F1:
            if (action != GLFW_PRESS) return;
            if (sandbox->camera == sandbox->character.fppov) {
            sandbox_set_camera(sandbox, sandbox->character.tppov);
            } else {
                sandbox_set_camera(sandbox, sandbox->character.fppov);
            }
            break;
        case GLFW_KEY_F2:
            if (action != GLFW_PRESS) return;
            if (sandbox->map.metadata.numCameraNodes) {
                sandbox_set_camera(sandbox, sandbox->map.metadata.cameraNodes[(camIdx++) % sandbox->map.metadata.numCameraNodes]);
            }
            break;
        case GLFW_KEY_W:
            if (action == GLFW_PRESS) {
                character_set_action(&sandbox->character, ACTION_RUN);
            } else if (action == GLFW_RELEASE) {
                character_set_action(&sandbox->character, ACTION_IDLE);
            }
            break;
            /*
        case GLFW_KEY_LEFT: case GLFW_KEY_UP:
            prog->activeCam = (prog->activeCam + prog->metadata.nbCameraNodes - 1) % prog->metadata.nbCameraNodes;
            update_cam(viewer, prog);
            break;
        case GLFW_KEY_RIGHT: case GLFW_KEY_DOWN:
            prog->activeCam = (prog->activeCam + 1) % prog->metadata.nbCameraNodes;
            update_cam(viewer, prog);
            break;
            */
    }
}

void cursor_callback(struct Viewer* viewer, double xpos, double ypos, double dx, double dy,
                     int bl, int bm, int br, void* data) {
    Vec3 axisY = {0, 1, 0};
    struct Sandbox* sandbox = data;

    node_rotate(sandbox->character.main, axisY, -dx / viewer->width);
}

void close_callback(struct Viewer* viewer, void* data) {
    struct Sandbox* sandbox = data;
    sandbox->running = 0;
}

void update_node(struct Scene* scene, struct Node* n, void* data) {
    struct Sandbox* sandbox = data;
    unsigned int i;

    switch (n->type) {
        case NODE_DLIGHT:
            for (i = 0; i < sandbox->lmgr.numDirectionalLights; i++) {
                if (n == sandbox->lmgr.dlights[i]) {
                    lights_buffer_object_update_dlight(&scene->lights, n->data.dlight, i);
                }
            }
            break;
        case NODE_PLIGHT:
            for (i = 0; i < sandbox->lmgr.numPointLights; i++) {
                if (n == sandbox->lmgr.plights[i]) {
                    lights_buffer_object_update_plight(&scene->lights, n->data.plight, i);
                }
            }
            break;
        case NODE_CAMERA:
            if (n == sandbox->camera) {
                camera_buffer_object_update_view(&scene->camera, MAT_CONST_CAST(n->data.camera->view));
                camera_buffer_object_update_position(&scene->camera, n->position);
            }
            break;
        default:;
    }
}
