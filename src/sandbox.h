#ifndef SANDBOX_H
#define SANDBOX_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <game/init.h>
#include <game/render/camera_buffer_object.h>
#include <game/render/lights_buffer_object.h>
#include <game/scene/opengex.h>

#include "map.h"
#include "character.h"

struct Sandbox {
    struct Map map;
    struct Character character;

    struct Scene scene;
    struct LightManager lmgr;
    struct Viewer* viewer;
    struct Node* camera;
    int running;
};

int sandbox_load(struct Sandbox* sandbox, char* character, char* map);
void sandbox_free(struct Sandbox* sandbox);

int sandbox_run(struct Sandbox* sandbox);
void sandbox_set_camera(struct Sandbox* sandbox, struct Node* camNode);


void free_node_callback(struct Node* node);
void resize_callback(struct Viewer* viewer, void* data);
void update_cam(struct Viewer* viewer, struct Sandbox* prog);
void key_callback(struct Viewer* viewer, int key, int scancode, int action, int mods, void* data);
void cursor_callback(struct Viewer* viewer, double, double, double, double, int, int, int, void*);
void close_callback(struct Viewer* viewer, void* data);
void update_node(struct Scene* scene, struct Node* n, void* data);


#endif
