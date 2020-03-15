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
    struct Node* node;
    struct Skin* skin;

    enum CharacterAction curAction;
    struct Clip actionClips[NB_ACTIONS];
};

#endif
