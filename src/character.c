#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <game/scene/opengex.h>

#include "character.h"
#include "utils.h"

#define CHARACTER_NODE_NAME "character"
#define CHARACTER_SKELETON_NAME "skeleton"
#define CHARACTER_FPPOV_NAME "fppov"
#define CHARACTER_TPPOV_NAME "tppov"

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
    return ok;
}

int character_animate(struct Character* character, double dt) {
    if (character->actionClips[character->action]) {
        anim_play_clip(character->actionClips[character->action], dt * 1000);
    }
    return 1;
}
