#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <3dmr/scene/gltf.h>

#include "character.h"
#include "phys/phys_solver.h"
#include "utils/utils.h"

#define CHARACTER_NODE_NAME             "character"
#define CHARACTER_SKELETON_NAME         "skeleton"
#define CHARACTER_MESH_NAME             "mesh"
#define CHARACTER_FOV                   80.

#define CHARACTER_TRANSITION_TIME       0.150
#define CHARACTER_RUN_SPEED             4
#define CHARACTER_ANGULAR_SPEED         4 * M_PI
#define CHARACTER_FORWARD               {0, 0, 1}
Vec3 characterBodySphereOffset =        {0, 1, 0};

struct CharacterClipEntry {
    const enum CharacterAction action;
    const char* name;
    const enum ClipPlayMode mode;
    const char loop;
};

static const struct CharacterClipEntry CHARACTER_CLIP[NB_ACTIONS] = {
    { ACTION_IDLE,  "idle",     CLIP_BACK_FORTH,   1 },
    { ACTION_WALK,  "walking",  CLIP_FORWARD,   1 },
    { ACTION_RUN,   "running",  CLIP_FORWARD,   1 },
};

static int character_init(struct Character* character) {
    import_init_metadata(&character->metadata);
    import_init_shared_data(&character->sharedData);

    if (!(character->main = malloc(sizeof(struct Node)))) {
        fprintf(stderr, "Error: cannot allocate character node\n");
        return 0;
    }
    node_init(character->main);
    character->main->name = strcopy(CHARACTER_NODE_NAME);

    character->skeleton = NULL;
    character->tppov = NULL;
    character->tppovOrientation = NULL;
    character->action = ACTION_IDLE;
    memset(character->actionClips, 0, sizeof(character->actionClips));

    character->bodySphere = NULL;
    memcpy(character->bodySphereOffset, characterBodySphereOffset, sizeof(Vec3));
    character->octree = NULL;
    return 1;
}

static int character_setup_cam(struct Character* character) {
    struct Camera* cam;
    struct Node *orientation, *camNode;
    int a = 0, b = 0;
    Vec3 pos = CHARACTER_FORWARD;

    if (       !(cam = malloc(sizeof(*cam)))
            || !(orientation = malloc(sizeof(*orientation)))
            || !(camNode = malloc(sizeof(*camNode)))) {
        fprintf(stderr, "Error: can't allocate camera\n");
        return 0;
    }
    camera_projection(1., CHARACTER_FOV / 360. * 2. * M_PI, 0.0001, 1000., cam->projection);
    node_init(orientation);
    node_init(camNode);
    node_set_camera(camNode, cam);

    scale3v(pos, 2);
    node_set_pos(camNode, pos);
    node_set_pos(orientation, characterBodySphereOffset);

    if (!(a = node_add_child(character->main, orientation)) || !(b = node_add_child(orientation, camNode))) {
        if (!a) free(orientation);
        if (!b) {
            free(cam);
            free(camNode);
        }
        fprintf(stderr, "Error: can't setup camera\n");
        return 0;
    }
    character->tppov = camNode;
    character->tppovOrientation = orientation;
    return 1;
}

static int find_skel(struct Character* character) {
    struct Node* cur = character->main;

    while (cur) {
        if (cur->name && !strcmp(cur->name, CHARACTER_SKELETON_NAME)) {
            character->skeleton = cur;
            return 1;
        }
        if (cur->nbChildren) {
            cur = cur->children[0];
        } else if (cur->father) {
            int stop = 0;
            while (!stop) {
                int i = 0;
                for (i = 0; cur != cur->father->children[i]; i++);
                if (i < cur->father->nbChildren - 1) {
                    cur = cur->father->children[i + 1];
                    stop = 1;
                }
            }
        } else {
            cur = NULL;
        }
    }
    return 0;
}

static int character_setup_nodes(struct Character* character) {
    return find_skel(character) && character_setup_cam(character);
}

static int character_setup_clip(struct Character* character) {
    unsigned int i = 0;

    for (i = 0; i < character->metadata.numClips; i++) {
        unsigned int j = 0;
        if (!character->metadata.clips[i]->name) continue;
        for (j = 0; j < NB_ACTIONS; j++) {
            if (!strcmp(character->metadata.clips[i]->name, CHARACTER_CLIP[j].name)) {
                struct Clip* clip = character->metadata.clips[i];
                character->actionClips[CHARACTER_CLIP[j].action] = clip;
                clip->mode = CHARACTER_CLIP[j].mode;
                clip->loop = CHARACTER_CLIP[j].loop;
            }
        }
    }
    return 1;
}

int character_load(struct Character* character, char* charFilename, struct Node* root) {
    FILE* charFile = NULL;
    int ok = 1;

    if (!character_init(character)) {
        ok = 0;
    } else if (!(charFile = fopen(charFilename, "r"))) {
        fprintf(stderr, "Error: could not open character file: %s\n", charFilename);
        ok = 0;
    } else if (!gltf_load(character->main, charFile, dirname(charFilename),
                          &character->sharedData, &character->metadata, 1)) {
        fprintf(stderr, "Error: character import failed\n");
        ok = 0;
    } else if (!character_setup_nodes(character) || !node_add_child(root, character->main)) {
        nodes_free(character->main, imported_node_free);
        free(character->main);
        ok = 0;
    } else {
        character_setup_clip(character);
        character->animStack = NULL;
        character_set_action(character, ACTION_IDLE);
    }
    if (charFile) fclose(charFile);
    return ok;
}

int character_setup_physic(struct Character* character, struct PhysOctree* octree) {
    Vec3 spherePos;
    character->octree = octree;
    if (!(character->bodySphere = phys_object_new_sphere(0.25))) return 0;
    add3v(spherePos, character->main->position, character->bodySphereOffset);
    memcpy(character->bodySphere->pos, spherePos, sizeof(Vec3));
    if (!(phys_octree_object_add(octree, character->bodySphere))) return 0;
    return 1;
}

void character_rotate(struct Character* character, float angle) {
    Vec3 axisY = {0, 1, 0};

    node_rotate(character->skeleton, axisY, angle);
}

void character_set_action(struct Character* character, enum CharacterAction action) {
    if (character->actionClips[action]) {
        struct Clip* transition;
        anim_stack_flush(&character->animStack);
        character->actionClips[character->action]->curPos = 0;
        character->action = action;
        if ((transition = anim_make_clip_transition(character->actionClips[action], CHARACTER_TRANSITION_TIME))) {
            anim_stack_push(&character->animStack, character->actionClips[action], 0);
            anim_stack_push(&character->animStack, transition, 0);
        }
    }
}

static int character_move(struct Character* character, Vec3 dir) {
    Vec3 globalDir, correction, finalDir;
    quaternion_compose(globalDir, character->skeleton->orientation, dir);
    phys_octree_object_translate(character->octree, character->bodySphere, globalDir);
    if (phys_solve_object_move(character->octree, character->bodySphere, correction)) {
        correction[1] = 0;
        phys_octree_object_translate(character->octree, character->bodySphere, correction);
    }
    add3v(finalDir, globalDir, correction);
    node_translate(character->main, finalDir);
    return 1;
}

static float angle_btw_nodes(struct Node* n1, struct Node* n2) {
    Vec4 axisX = {1, 0, 0, 0}, v1, v2;
    Vec2 sv1, sv2;

    node_update_matrices(n1);
    node_update_matrices(n2);
    mul4mv(v1, MAT_CONST_CAST(n1->model), axisX);
    mul4mv(v2, MAT_CONST_CAST(n2->model), axisX);
    sv1[0] = v1[0]; sv1[1] = v1[2];
    sv2[0] = v2[0]; sv2[1] = v2[2];
    return atan2(sv1[0] * sv2[1] - sv1[1] * sv2[0], sv1[0] * sv2[0] + sv1[1] * sv2[1]);
}

static void character_update_dir(struct Character* character, float dt) {
    float angle;

    angle = M_PI - angle_btw_nodes(character->skeleton, character->tppovOrientation);
    angle = angle > M_PI ? angle - 2 * M_PI : angle;
    if (fabs(angle) >= dt * CHARACTER_ANGULAR_SPEED) angle = angle / fabs(angle) * dt * CHARACTER_ANGULAR_SPEED;
    character_rotate(character, angle);
    return;
}

void character_run_action(struct Character* character, double dt) {
    Vec3 axis = CHARACTER_FORWARD;
    switch (character->action) {
        case ACTION_IDLE:
            break;
        case ACTION_WALK:
            break;
        case ACTION_RUN:
            scale3v(axis, CHARACTER_RUN_SPEED * dt);
            character_update_dir(character, dt);
            character_move(character, axis);
            break;
        default:
            break;
    }
}

int character_animate(struct Character* character, double dt) {
    anim_run_stack(&character->animStack, dt);
    return 1;
}

void character_free(struct Character* character) {
    if (character) {
        anim_stack_flush(&character->animStack);
        import_free_metadata(&character->metadata);
        import_free_shared_data(&character->sharedData);
        phys_object_free(character->bodySphere);
    }
}
