#include "Tekken.h"

int main() {
    // Create abilities
    {
        auto cmd = std::make_shared<CompositeCommand>();
        cmd->add(TAG_DEFENDER_OUT);
        cmd->add(AFTER_ROUNDS(2, TAG_DEFENDER_IN));
        createAbility("Give_Autographs", cmd);
    }
    
    createAbility("Bleeding_Bite", FOR_ROUNDS(5, DAMAGE_DEFENDER(8)));
    createAbility("Head_Smash", DAMAGE_DEFENDER(22));
    createAbility("Catch_A_Break", HEAL_ATTACKER(30));

    // Create fighters
    createFighter("Lee", "Rushdown", 100);
    createFighter("Jack-6", "Heavy", 90);

    // Teach abilities to fighters
    teachAbility("Lee", "Give_Autographs");
    teachAbility("Lee", "Head_Smash");
    teachAbility("Lee", "Catch_A_Break");
    teachAbility("Lee", "Bleeding_Bite");

    teachAbility("Jack-6", "Head_Smash");
    teachAbility("Jack-6", "Catch_A_Break");
    teachAbility("Jack-6", "Bleeding_Bite");

    // Start the battle
    runDuel();
    
    return 0;
}
