#include "Tekken.h"

BEGIN_GAME

// Simple example demonstrating all core features

// 1. Simple damage ability
auto punch = []() {
    return DAMAGE_DEFENDER(15);
};

// 2. Simple heal ability
auto meditate = []() {
    return HEAL_ATTACKER(20);
};

// 3. Combo ability (multiple actions)
auto powerCombo = []() {
    auto cmd = std::make_shared<CompositeCommand>();
    cmd->add(DAMAGE_DEFENDER(10));
    cmd->add(DAMAGE_DEFENDER(10));
    cmd->add(HEAL_ATTACKER(5));
    return cmd;
};

// 4. Conditional ability
auto smartAttack = []() {
    auto cmd = std::make_shared<CompositeCommand>();
    
    // If defender HP > 50, do heavy damage, else do light damage
    auto condition = GET_HP(DEFENDER) > NumericValue(50);
    auto heavyDamage = DAMAGE_DEFENDER(30);
    auto lightDamage = DAMAGE_DEFENDER(15);
    
    cmd->add(IF_THEN_ELSE(condition, heavyDamage, lightDamage));
    return cmd;
};

// 5. Recurring damage (damage over time)
auto poison = []() {
    // 10 damage for 3 rounds
    return FOR_ROUNDS(3, DAMAGE_DEFENDER(10));
};

// 6. Delayed action
auto timeBomb = []() {
    auto cmd = std::make_shared<CompositeCommand>();
    
    // Immediate small damage
    cmd->add(DAMAGE_DEFENDER(5));
    
    // Big damage after 2 rounds
    cmd->add(AFTER_ROUNDS(2, DAMAGE_DEFENDER(25)));
    
    return cmd;
};

// 7. Ring control
auto ringOut = []() {
    auto cmd = std::make_shared<CompositeCommand>();
    
    // Push opponent out of ring
    cmd->add(TAG_DEFENDER_OUT);
    
    // Bring them back after 1 round
    cmd->add(AFTER_ROUNDS(1, TAG_DEFENDER_IN));
    
    return cmd;
};

// Register all abilities
createAbility("Punch", punch());
createAbility("Meditate", meditate());
createAbility("Power_Combo", powerCombo());
createAbility("Smart_Attack", smartAttack());
createAbility("Poison", poison());
createAbility("Time_Bomb", timeBomb());
createAbility("Ring_Out", ringOut());

// Create diverse fighters
createFighter("Striker", "Rushdown", 100);   // Fast attacker
createFighter("Tank", "Heavy", 150);         // High HP, good defense
createFighter("Ninja", "Evasive", 80);       // Low HP, high evasion
createFighter("Wrestler", "Grappler", 120);  // Balanced, heals on even rounds

// Teach abilities to fighters
// Striker: Aggressive fighter
teachAbility("Striker", "Punch");
teachAbility("Striker", "Power_Combo");
teachAbility("Striker", "Poison");

// Tank: Defensive fighter
teachAbility("Tank", "Punch");
teachAbility("Tank", "Meditate");
teachAbility("Tank", "Smart_Attack");

// Ninja: Tactical fighter
teachAbility("Ninja", "Ring_Out");
teachAbility("Ninja", "Time_Bomb");
teachAbility("Ninja", "Poison");

// Wrestler: Balanced fighter
teachAbility("Wrestler", "Punch");
teachAbility("Wrestler", "Power_Combo");
teachAbility("Wrestler", "Meditate");
teachAbility("Wrestler", "Smart_Attack");

// Start battle
std::cout << "=== TEKKEN BATTLE SIMULATOR ===" << std::endl;
std::cout << "\nFighters demonstrate different types and abilities:" << std::endl;
std::cout << "- Striker (Rushdown): +20% damage to Grappler, +15% to others" << std::endl;
std::cout << "- Tank (Heavy): -30% damage from Evasive, -20% from others" << std::endl;
std::cout << "- Ninja (Evasive): +7% damage dealt, -7% damage taken" << std::endl;
std::cout << "- Wrestler (Grappler): +7% damage on odd rounds, +5% heal on even rounds" << std::endl;
std::cout << std::endl;

DUEL

END_GAME
