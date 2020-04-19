#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <game/scene/opengex.h>

#include "character.h"
#include "phys_solver.h"
#include "utils.h"

#define CHARACTER_NODE_NAME             "character"
#define CHARACTER_SKELETON_NAME         "skeleton"
#define CHARACTER_FPPOV_NAME            "fppov"
#define CHARACTER_TPPOV_NAME            "tppov"

#define CHARACTER_TRANSITION_TIME       0.150
#define CHARACTER_RUN_SPEED             4
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
    character->fppov = NULL;
    character->tppov = NULL;
    character->action = ACTION_IDLE;
    memset(character->actionClips, 0, sizeof(character->actionClips));

    character->bodySphere = NULL;
    memcpy(character->bodySphereOffset, characterBodySphereOffset, sizeof(Vec3));
    character->octree = NULL;
    return 1;
}

static int character_setup_nodes(struct Node* cur, struct Character* character) {
    int i = 0;
    if (!cur || !cur->name) return 1;
    if (!strcmp(cur->name, CHARACTER_SKELETON_NAME)) {
        character->skeleton = cur;
        printf("Found character skeleton\n");
    } else if (!strcmp(cur->name, CHARACTER_FPPOV_NAME)) {
        character->fppov = cur;
        printf("Found character fppov\n");
    } else if (!strcmp(cur->name, CHARACTER_TPPOV_NAME)) {
        character->tppov = cur;
        printf("Found character tppov\n");
    }
    for (i = 0; i < cur->nbChildren; i++) {
        character_setup_nodes(cur->children[i], character);
    }
    return 1;
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
    } else if (!ogex_load(character->main, charFile, dirname(charFilename),
        &character->sharedData, &character->metadata)) {
        fprintf(stderr, "Error: character import failed\n");
        ok = 0;
    } else if (!node_add_child(root, character->main)) {
        nodes_free(character->main, imported_node_free);
        free(character->main);
        ok = 0;
    }
    if (charFile) fclose(charFile);
    character_setup_nodes(character->main, character);
    character_setup_clip(character);
    character->animStack = NULL;
    character_set_action(character, ACTION_IDLE);
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
    quaternion_compose(globalDir, character->main->orientation, dir);
    phys_octree_object_translate(character->octree, character->bodySphere, globalDir);
    if (phys_solve_object_move(character->octree, character->bodySphere, correction)) {
        correction[1] = 0;
        phys_octree_object_translate(character->octree, character->bodySphere, correction);
    }
    add3v(finalDir, globalDir, correction);
    node_translate(character->main, finalDir);
    return 1;
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
