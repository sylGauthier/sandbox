#include <stdlib.h>

#include "sandbox.h"

void free_node_callback(struct Node* node) {
    if (node->type == NODE_GEOMETRY) {
        free(node->data.geometry);
    } else if (node->type == NODE_PLIGHT) {
        free(node->data.plight);
    } else if (node->type == NODE_DLIGHT) {
        free(node->data.dlight);
    }
    if (node->father) {
        free(node);
    }
}

void resize_callback(struct Viewer* viewer, void* data) {
    struct Sandbox* sandbox = data;
    struct Camera* activeCam = sandbox->camera;
    glViewport(0, 0, viewer->width, viewer->height);
    camera_set_ratio(((float)viewer->width) / ((float)viewer->height), activeCam->projection);
    camera_buffer_object_update_projection(&sandbox->scene.camera, MAT_CONST_CAST(activeCam->projection));
    uniform_buffer_send(&sandbox->scene.camera);
}

void update_cam(struct Viewer* viewer, struct Sandbox* sandbox) {
    camera_set_ratio(((float)viewer->width) / ((float)viewer->height), sandbox->camera->projection);
    camera_buffer_object_update_projection(&sandbox->scene.camera, MAT_CONST_CAST(sandbox->camera->projection));
    camera_buffer_object_update_view_and_position(&sandbox->scene.camera, MAT_CONST_CAST(sandbox->camera->view));
    uniform_buffer_send(&sandbox->scene.camera);
}

void key_callback(struct Viewer* viewer, int key, int scancode, int action, int mods, void* data) {
    struct Sandbox* sandbox = data;
    if (action != GLFW_PRESS) return;
    switch (key) {
        case GLFW_KEY_ESCAPE:
            sandbox->running = 0;
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
            /*if (n == sandbox->metadata.cameraNodes[prog->activeCam]) {
                camera_buffer_object_update_view(&scene->camera, MAT_CONST_CAST(n->data.camera->view));
                camera_buffer_object_update_position(&scene->camera, n->position);
            }
            */
            break;
        default:;
    }
}
