#ifndef COMPONENTS_H
#define COMPONENTS_H


struct PlayerStateC final{
static constexpr float MAX_HEALTH = 10.0F;
static constexpr float MAX_MANA= 5.0F;

float health = MAX_HEALTH;
float mana;

};

#endif //COMPONENTS_H