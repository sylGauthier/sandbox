#ifndef CHARACTER_H
#define CHARACTER_H

#include <3dmr/scene/skin.h>
#include <3dmr/animation/transition.h>
#include <3dmr/animation/play.h>

#include "phys/phys_octree.h"

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
    struct AnimStack* animStack;

    struct PhysObject* bodySphere;
    Vec3 bodySphereOffset;
    struct PhysOctree* octree;
};

int character_load(struct Character* character, char* charFilename, struct Node* root);
int character_setup_physic(struct Character* character, struct PhysOctree* octree);

void character_set_action(struct Character* character, enum CharacterAction action);
void character_run_action(struct Character* character, double dt);
int character_animate(struct Character* character, double dt);
void character_free(struct Character* character);

#endif
