#include "Tekken.h"

BEGIN_GAME

// Example with conditional abilities

auto yoshimitsuHeal = []() {
    auto healCmd = std::make_shared<CompositeCommand>();
    
    // If HP < 30, heal 25, else heal 15
    auto lowHPHeal = HEAL_ATTACKER(25);
    auto normalHeal = HEAL_ATTACKER(15);
    auto condition = GET_HP(ATTACKER) < NumericValue(30);
    
    healCmd->add(IF_THEN_ELSE(condition, lowHPHeal, normalHeal));
    return healCmd;
};

auto spinningKick = []() {
    return DAMAGE_DEFENDER(18);
};

auto highPunch = []() {
    return DAMAGE_DEFENDER(15);
};

auto lowKick = []() {
    return DAMAGE_DEFENDER(12);
};

auto rollingKick = []() {
    // Damage depends on defender type
    auto cmd = std::make_shared<CompositeCommand>();
    auto isGrappler = GET_TYPE(DEFENDER) == "Grappler";
    auto strongDamage = DAMAGE_DEFENDER(25);
    auto normalDamage = DAMAGE_DEFENDER(18);
    
    cmd->add(IF_THEN_ELSE(isGrappler, strongDamage, normalDamage));
    return cmd;
};

// Create abilities
createAbility("Yoshimitsu_Heal", yoshimitsuHeal());
createAbility("Spinning_Kick", spinningKick());
createAbility("High_Punch", highPunch());
createAbility("Low_Kick", lowKick());
createAbility("Rolling_Kick", rollingKick());

// Create fighters
createFighter("Yoshimitsu", "Evasive", 85);
createFighter("Ashuka", "Evasive", 90);
createFighter("Paul", "Heavy", 125);
createFighter("King", "Grappler", 150);

// Teach abilities
teachAbility("Yoshimitsu", "Yoshimitsu_Heal");
teachAbility("Yoshimitsu", "Spinning_Kick");

teachAbility("Ashuka", "Spinning_Kick");
teachAbility("Ashuka", "High_Punch");
teachAbility("Ashuka", "Low_Kick");
teachAbility("Ashuka", "Rolling_Kick");

teachAbility("Paul", "High_Punch");
teachAbility("Paul", "Low_Kick");

teachAbility("King", "Rolling_Kick");
teachAbility("King", "High_Punch");

// Start battle
DUEL

END_GAME
