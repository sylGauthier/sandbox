#include <string.h>

#include "sandbox.h"

static int load_character(struct Sandbox* sandbox, char* character) {
/*
    FILE* charFile = NULL;
    int ok = 1;
    if (!(charFile = fopen(character, "r"))) {
        fprintf(stderr, "Error: could not open character file: %s\n", character);
        ok = 0;
    } else if (!ogex_load(&sandbox->scene.root, charFile, dirname(character), 
    */
    return 1;
}

int sandbox_load(struct Sandbox* sandbox, char* character, char* map) {
    int sceneInit = 0, mapLoad = 0;

    if (!game_init(GAME_SHADERS_PATH)) {
        fprintf(stderr, "Error: failed to init game library\n");
        return 0;
    }
    light_mgr_init(&sandbox->lmgr, &sandbox->scene);
    if (!(sandbox->viewer = viewer_new(1024, 768, "sandbox"))) {
        fprintf(stderr, "Error: failed to start viewer\n");
    } else if (!(sceneInit = scene_init(&sandbox->scene, NULL))) {
        fprintf(stderr, "Error: failed to init scene\n");
    } else if (!(mapLoad = map_load(&sandbox->map, map, &sandbox->scene, &sandbox->lmgr))) {
        fprintf(stderr, "Error: failed to load map\n");
    } else {
        sandbox->running = 1;

        sandbox->viewer->callbackData = sandbox;
        sandbox->viewer->resize_callback = resize_callback;
        sandbox->viewer->key_callback = key_callback;
        sandbox->viewer->close_callback = close_callback;

        scene_update_nodes(&sandbox->scene, update_node, sandbox);

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
