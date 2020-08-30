#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "sandbox.h"
#include "utils/utils.h"
#include "render_octree.h"

static int octree_add_objects(struct PhysOctree* octree, struct PhysObject** objects, unsigned int numObjects) {
    unsigned int i;
    for (i = 0; i < numObjects; i++) {
        if (!phys_octree_object_add(octree, objects[i])) {
            fprintf(stderr, "Error: failed to add object to octree\n");
            return 0;
        }
    }
    return 1;
}

static int setup_physic(struct Sandbox* sandbox) {
    unsigned int i;
    float size = 0.;
    struct Node* root = &sandbox->scene.root;

    for (i = 0; i < 6; i++) {
        size = fabs(root->position[i%3] - root->boundingBox[i / 3][i % 3]) > size ?
               fabs(root->position[i%3] - root->boundingBox[i / 3][i % 3]) : size;
    }
    if (!phys_octree_init(&sandbox->octree, root->position, size)) return 0;
    if (!octree_add_objects(&sandbox->octree, sandbox->map.objects, sandbox->map.numObjects)) return 0;

    if (!character_setup_physic(&sandbox->character, &sandbox->octree)) return 0;
    /* phys_octree_print(&sandbox->octree); */
    return 1;
}

int sandbox_load(struct Sandbox* sandbox, char* character, char* map) {
    int sceneInit = 0, mapLoad = 0, charLoad = 0;
    Vec3 ambient = {0.03, 0.03, 0.04};

    light_mgr_init(&sandbox->lmgr, &sandbox->scene);
    if (!(sandbox->viewer = viewer_new(1024, 768, "sandbox"))) {
        fprintf(stderr, "Error: failed to start viewer\n");
    } else if (!(sceneInit = scene_init(&sandbox->scene, NULL))) {
        fprintf(stderr, "Error: failed to init scene\n");
    } else if (!(mapLoad = map_load(&sandbox->map, map, &sandbox->scene, &sandbox->lmgr))) {
        fprintf(stderr, "Error: failed to load map\n");
    } else if (!(charLoad = character_load(&sandbox->character, character, &sandbox->scene.root))) {
        fprintf(stderr, "Error: failed to load character\n");
    } else {
        sandbox->running = 1;
        sandbox->renderOctreeOn = 0;

        sandbox->viewer->callbackData = sandbox;
        sandbox->viewer->resize_callback = resize_callback;
        sandbox->viewer->key_callback = key_callback;
        sandbox->viewer->cursor_callback = cursor_callback;
        sandbox->viewer->close_callback = close_callback;
        viewer_set_cursor_mode(sandbox->viewer, VIEWER_CURSOR_DISABLED);

        scene_update_nodes(&sandbox->scene, update_node, sandbox);

        sandbox_set_camera(sandbox, sandbox->map.metadata.cameraNodes[0]);
        light_mgr_set_ambient(&sandbox->lmgr, ambient);

        uniform_buffer_send(&sandbox->scene.lights);
        uniform_buffer_send(&sandbox->scene.camera);
        glfwSwapInterval(1);
        if (setup_physic(sandbox)) {
            render_octree_init();
            return 1;
        }
    }
    if (sandbox->viewer) {
        viewer_free(sandbox->viewer);
    }
    return 0;
}

int sandbox_run(struct Sandbox* sandbox) {
    double dt;

    viewer_process_events(sandbox->viewer);
    dt = viewer_next_frame(sandbox->viewer);
    character_run_action(&sandbox->character, dt);
    character_animate(&sandbox->character, dt);

    scene_update_nodes(&sandbox->scene, update_node, sandbox);
    uniform_buffer_send(&sandbox->scene.lights);
    uniform_buffer_send(&sandbox->scene.camera);

    scene_update_render_queue(&sandbox->scene, MAT_CONST_CAST(sandbox->camera->data.camera->view),
                                               MAT_CONST_CAST(sandbox->camera->data.camera->projection));
    scene_render(&sandbox->scene);
    if (sandbox->renderOctreeOn) {
        render_octree(&sandbox->octree);
    }
    return 1;
}

void sandbox_set_camera(struct Sandbox* sandbox, struct Node* camNode) {
    struct Camera* cam;
    if (camNode->type != NODE_CAMERA) return;
    cam = camNode->data.camera;
    sandbox->camera = camNode;
    camera_set_ratio(((float)sandbox->viewer->width) / ((float)sandbox->viewer->height), cam->projection);
    camera_buffer_object_update_projection(&sandbox->scene.camera, MAT_CONST_CAST(cam->projection));
    camera_buffer_object_update_view_and_position(&sandbox->scene.camera, MAT_CONST_CAST(cam->view));
    uniform_buffer_send(&sandbox->scene.camera);
}

void sandbox_free(struct Sandbox* sandbox) {
    if (sandbox) {
        unsigned int i;
        map_free(&sandbox->map);
        character_free(&sandbox->character);
        for (i = 0; i < sandbox->scene.root.nbChildren; i++) {
            nodes_free(sandbox->scene.root.children[i], imported_node_free);
        }
        sandbox->scene.root.nbChildren = 0;
        render_octree_cleanup();
        scene_free(&sandbox->scene, NULL);
        viewer_free(sandbox->viewer);
    }
}
