#ifndef CHARACTER_H
#define CHARACTER_H

#include <game/scene/skin.h>
#include <game/animation/animation.h>

enum CharacterAction {
    ACTION_IDLE = 0,
    ACTION_WALK,
    ACTION_RUN,
    NB_ACTIONS
};

struct Character {
    struct ImportMetadata metadata;
    struct ImportSharedData sharedData;

    struct Node* main;
    struct Node* skeleton;
    struct Node* fppov; /* First person point of view */
    struct Node* tppov; /* Third person point of view */

    enum CharacterAction action;
    struct Clip* actionClips[NB_ACTIONS];
};

int character_load(struct Character* character, char* charFilename, struct Node* root);
int character_animate(struct Character* character, double dt);

#endif
