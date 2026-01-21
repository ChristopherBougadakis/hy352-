#include "Tekken.h"

BEGIN_GAME

// Simple demo with just 2 fighters and basic abilities

auto punch = []() {
    return DAMAGE_DEFENDER(20);
};

auto heal = []() {
    return HEAL_ATTACKER(15);
};

createAbility("Punch", punch());
createAbility("Heal", heal());

createFighter("Fighter_A", "Rushdown", 100);
createFighter("Fighter_B", "Heavy", 100);

teachAbility("Fighter_A", "Punch");
teachAbility("Fighter_A", "Heal");

teachAbility("Fighter_B", "Punch");
teachAbility("Fighter_B", "Heal");

DUEL

END_GAME
