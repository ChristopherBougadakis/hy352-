#ifndef TEKKEN_H
#define TEKKEN_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <algorithm>
#include <cmath>

// Forward declarations
class Fighter;
class Ability;

// Global registries
static std::map<std::string, std::shared_ptr<Fighter>> fighterRegistry;
static std::map<std::string, std::shared_ptr<Ability>> abilityRegistry;

// ========== COMMAND SYSTEM ==========

class Command {
public:
    virtual ~Command() = default;
    virtual void execute(Fighter* attacker, Fighter* defender, int round) = 0;
    virtual std::shared_ptr<Command> clone() const = 0;
};

class CompositeCommand : public Command {
public:
    std::vector<std::shared_ptr<Command>> commands;
    
    void add(std::shared_ptr<Command> cmd) {
        commands.push_back(cmd);
    }
    
    void execute(Fighter* attacker, Fighter* defender, int round) override {
        for (auto& cmd : commands) {
            cmd->execute(attacker, defender, round);
        }
    }
    
    std::shared_ptr<Command> clone() const override {
        auto cmd = std::make_shared<CompositeCommand>();
        for (auto& c : commands) {
            cmd->commands.push_back(c->clone());
        }
        return cmd;
    }
};

// ========== FIGHTER CLASS ==========

class Fighter {
public:
    std::string name;
    std::string type;
    double maxHP;
    double currentHP;
    bool inRing;
    std::vector<std::shared_ptr<Ability>> abilities;
    std::vector<std::pair<int, std::shared_ptr<Command>>> delayedCommands;
    std::vector<std::pair<int, std::shared_ptr<Command>>> recurringCommands;
    
    Fighter(const std::string& n, const std::string& t, double hp)
        : name(n), type(t), maxHP(hp), currentHP(hp), inRing(true) {}
    
    void takeDamage(double amount, Fighter* attacker, int round) {
        if (!inRing) return;
        
        double finalDamage = amount;
        
        // Attacker type bonuses
        if (attacker->type == "Rushdown") {
            if (this->type == "Grappler") {
                finalDamage *= 1.20;
            } else {
                finalDamage *= 1.15;
            }
        } else if (attacker->type == "Evasive") {
            finalDamage *= 1.07;
        } else if (attacker->type == "Grappler" && round % 2 == 1) {
            finalDamage *= 1.07;
        }
        
        // Defender type resistances
        if (this->type == "Heavy") {
            if (attacker->type == "Evasive") {
                finalDamage *= 0.70;
            } else {
                finalDamage *= 0.80;
            }
        } else if (this->type == "Evasive") {
            finalDamage *= 0.93;
        }
        
        currentHP -= finalDamage;
        if (currentHP < 0) currentHP = 0;
    }
    
    void heal(double amount) {
        currentHP += amount;
        if (currentHP > maxHP) currentHP = maxHP;
    }
    
    void leaveRing() { inRing = false; }
    void enterRing() { inRing = true; }
    bool isAlive() const { return currentHP > 0; }
    
    void addAbility(std::shared_ptr<Ability> ability) {
        abilities.push_back(ability);
    }
    
    void addDelayedCommand(int rounds, std::shared_ptr<Command> cmd) {
        delayedCommands.push_back({rounds, cmd});
    }
    
    void addRecurringCommand(int rounds, std::shared_ptr<Command> cmd) {
        recurringCommands.push_back({rounds, cmd});
    }
    
    void processDelayedCommands(Fighter* defender, int round) {
        std::vector<std::pair<int, std::shared_ptr<Command>>> remaining;
        for (auto& delayed : delayedCommands) {
            delayed.first--;
            if (delayed.first <= 0) {
                delayed.second->execute(this, defender, round);
            } else {
                remaining.push_back(delayed);
            }
        }
        delayedCommands = remaining;
    }
    
    void processRecurringCommands(Fighter* defender, int round) {
        std::vector<std::pair<int, std::shared_ptr<Command>>> remaining;
        for (auto& recurring : recurringCommands) {
            recurring.second->execute(this, defender, round);
            recurring.first--;
            if (recurring.first > 0) {
                remaining.push_back(recurring);
            }
        }
        recurringCommands = remaining;
    }
    
    void displayStatus() const {
        std::cout << "Name: " << name << "\n";
        std::cout << "HP: " << (int)currentHP << "\n";
        std::cout << "Type: " << type << "\n";
        std::cout << std::endl;
    }
};

// ========== ABILITY CLASS ==========

class Ability {
public:
    std::string name;
    std::shared_ptr<Command> action;
    
    Ability(const std::string& n) : name(n) {}
    
    void setAction(std::shared_ptr<Command> cmd) {
        action = cmd;
    }
    
    void use(Fighter* attacker, Fighter* defender, int round) {
        if (action) {
            action->execute(attacker, defender, round);
        }
    }
};

// ========== SPECIFIC COMMANDS ==========

class DamageCommand : public Command {
    bool isDefender;
    double amount;
public:
    DamageCommand(bool def, double a) : isDefender(def), amount(a) {}
    
    void execute(Fighter* attacker, Fighter* defender, int round) override {
        Fighter* target = isDefender ? defender : attacker;
        target->takeDamage(amount, attacker, round);
    }
    
    std::shared_ptr<Command> clone() const override {
        return std::make_shared<DamageCommand>(isDefender, amount);
    }
};

class HealCommand : public Command {
    bool isDefender;
    double amount;
public:
    HealCommand(bool def, double a) : isDefender(def), amount(a) {}
    
    void execute(Fighter* attacker, Fighter* defender, int /*round*/) override {
        Fighter* target = isDefender ? defender : attacker;
        target->heal(amount);
    }
    
    std::shared_ptr<Command> clone() const override {
        return std::make_shared<HealCommand>(isDefender, amount);
    }
};

class TagCommand : public Command {
public:
    bool isDefender;
    bool out;
    
    TagCommand(bool def, bool o) : isDefender(def), out(o) {}
    
    void execute(Fighter* attacker, Fighter* defender, int /*round*/) override {
        Fighter* target = isDefender ? defender : attacker;
        if (out) {
            target->leaveRing();
        } else {
            target->enterRing();
        }
    }
    
    std::shared_ptr<Command> clone() const override {
        return std::make_shared<TagCommand>(isDefender, out);
    }
};

class ForRoundsCommand : public Command {
    int rounds;
    std::shared_ptr<Command> cmd;
public:
    ForRoundsCommand(int r, std::shared_ptr<Command> c) : rounds(r), cmd(c) {}
    
    void execute(Fighter* attacker, Fighter* /*defender*/, int /*round*/) override {
        attacker->addRecurringCommand(rounds, cmd);
    }
    
    std::shared_ptr<Command> clone() const override {
        return std::make_shared<ForRoundsCommand>(rounds, cmd->clone());
    }
};

class AfterRoundsCommand : public Command {
    int rounds;
    std::shared_ptr<Command> cmd;
public:
    AfterRoundsCommand(int r, std::shared_ptr<Command> c) : rounds(r), cmd(c) {}
    
    void execute(Fighter* /*attacker*/, Fighter* defender, int /*round*/) override {
        // If the command is TAG_DEFENDER_IN, convert it to TAG_ATTACKER_IN
        // so it brings the defender back in (since the delayed command will execute
        // with the defender as the attacker)
        auto tagCmd = std::dynamic_pointer_cast<TagCommand>(cmd);
        if (tagCmd && tagCmd->isDefender && !tagCmd->out) {
            // Change to TAG_ATTACKER_IN
            auto newCmd = std::make_shared<TagCommand>(false, false);
            defender->addDelayedCommand(rounds, newCmd);
        } else {
            defender->addDelayedCommand(rounds, cmd);
        }
    }
    
    std::shared_ptr<Command> clone() const override {
        return std::make_shared<AfterRoundsCommand>(rounds, cmd->clone());
    }
};

// ========== CONDITION SYSTEM ==========

class ConditionExpr {
public:
    virtual ~ConditionExpr() = default;
    virtual bool evaluate(Fighter* attacker, Fighter* defender) = 0;
    virtual std::shared_ptr<ConditionExpr> clone() const = 0;
};

class ComparisonExpr : public ConditionExpr {
    std::function<double(Fighter*, Fighter*)> left;
    std::function<double(Fighter*, Fighter*)> right;
    std::string op;
public:
    ComparisonExpr(std::function<double(Fighter*, Fighter*)> l,
                   std::function<double(Fighter*, Fighter*)> r,
                   const std::string& o)
        : left(l), right(r), op(o) {}
    
    bool evaluate(Fighter* attacker, Fighter* defender) override {
        double lval = left(attacker, defender);
        double rval = right(attacker, defender);
        
        if (op == "==") return std::abs(lval - rval) < 0.001;
        if (op == "!=") return std::abs(lval - rval) >= 0.001;
        if (op == ">") return lval > rval;
        if (op == ">=") return lval >= rval;
        if (op == "<") return lval < rval;
        if (op == "<=") return lval <= rval;
        return false;
    }
    
    std::shared_ptr<ConditionExpr> clone() const override {
        return std::make_shared<ComparisonExpr>(left, right, op);
    }
};

class StringComparisonExpr : public ConditionExpr {
    std::function<std::string(Fighter*, Fighter*)> left;
    std::string right;
    std::string op;
public:
    StringComparisonExpr(std::function<std::string(Fighter*, Fighter*)> l,
                         const std::string& r,
                         const std::string& o)
        : left(l), right(r), op(o) {}
    
    bool evaluate(Fighter* attacker, Fighter* defender) override {
        std::string lval = left(attacker, defender);
        
        if (op == "==") return lval == right;
        if (op == "!=") return lval != right;
        return false;
    }
    
    std::shared_ptr<ConditionExpr> clone() const override {
        return std::make_shared<StringComparisonExpr>(left, right, op);
    }
};

class AndExpr : public ConditionExpr {
public:
    std::vector<std::shared_ptr<ConditionExpr>> conditions;
    
    AndExpr() {}
    AndExpr(std::shared_ptr<ConditionExpr> c1, std::shared_ptr<ConditionExpr> c2) {
        conditions.push_back(c1);
        conditions.push_back(c2);
    }
    
    void add(std::shared_ptr<ConditionExpr> cond) {
        conditions.push_back(cond);
    }
    
    bool evaluate(Fighter* attacker, Fighter* defender) override {
        for (auto& cond : conditions) {
            if (!cond->evaluate(attacker, defender)) return false;
        }
        return true;
    }
    
    std::shared_ptr<ConditionExpr> clone() const override {
        auto expr = std::make_shared<AndExpr>();
        for (auto& c : conditions) {
            expr->add(c->clone());
        }
        return expr;
    }
};

class OrExpr : public ConditionExpr {
public:
    std::vector<std::shared_ptr<ConditionExpr>> conditions;
    
    OrExpr() {}
    OrExpr(std::shared_ptr<ConditionExpr> c1, std::shared_ptr<ConditionExpr> c2) {
        conditions.push_back(c1);
        conditions.push_back(c2);
    }
    
    void add(std::shared_ptr<ConditionExpr> cond) {
        conditions.push_back(cond);
    }
    
    bool evaluate(Fighter* attacker, Fighter* defender) override {
        for (auto& cond : conditions) {
            if (cond->evaluate(attacker, defender)) return true;
        }
        return false;
    }
    
    std::shared_ptr<ConditionExpr> clone() const override {
        auto expr = std::make_shared<OrExpr>();
        for (auto& c : conditions) {
            expr->add(c->clone());
        }
        return expr;
    }
};

class NotExpr : public ConditionExpr {
    std::shared_ptr<ConditionExpr> condition;
public:
    NotExpr(std::shared_ptr<ConditionExpr> cond) : condition(cond) {}
    
    bool evaluate(Fighter* attacker, Fighter* defender) override {
        return !condition->evaluate(attacker, defender);
    }
    
    std::shared_ptr<ConditionExpr> clone() const override {
        return std::make_shared<NotExpr>(condition->clone());
    }
};

class IfCommand : public Command {
    std::shared_ptr<ConditionExpr> condition;
    std::shared_ptr<Command> thenCmd;
    std::shared_ptr<Command> elseCmd;
public:
    IfCommand(std::shared_ptr<ConditionExpr> cond, 
              std::shared_ptr<Command> then, 
              std::shared_ptr<Command> els = nullptr)
        : condition(cond), thenCmd(then), elseCmd(els) {}
    
    void execute(Fighter* attacker, Fighter* defender, int round) override {
        if (condition->evaluate(attacker, defender)) {
            if (thenCmd) thenCmd->execute(attacker, defender, round);
        } else {
            if (elseCmd) elseCmd->execute(attacker, defender, round);
        }
    }
    
    std::shared_ptr<Command> clone() const override {
        return std::make_shared<IfCommand>(
            condition->clone(),
            thenCmd ? thenCmd->clone() : nullptr,
            elseCmd ? elseCmd->clone() : nullptr
        );
    }
};

class ShowCommand : public Command {
    std::vector<std::function<std::string(Fighter*, Fighter*)>> parts;
public:
    ShowCommand() {}
    
    void addPart(std::function<std::string(Fighter*, Fighter*)> part) {
        parts.push_back(part);
    }
    
    void execute(Fighter* attacker, Fighter* defender, int /*round*/) override {
        for (auto& part : parts) {
            std::cout << part(attacker, defender);
        }
        std::cout << std::endl;
    }
    
    std::shared_ptr<Command> clone() const override {
        auto cmd = std::make_shared<ShowCommand>();
        cmd->parts = parts;
        return cmd;
    }
};

// ========== VALUE WRAPPERS ==========

class NumericValue {
    std::function<double(Fighter*, Fighter*)> value;
public:
    NumericValue(double val) : value([val](Fighter*, Fighter*) { return val; }) {}
    NumericValue(std::function<double(Fighter*, Fighter*)> val) : value(val) {}
    
    std::function<double(Fighter*, Fighter*)> getValue() const { return value; }
    
    std::shared_ptr<ConditionExpr> operator==(const NumericValue& other) const {
        return std::make_shared<ComparisonExpr>(value, other.value, "==");
    }
    std::shared_ptr<ConditionExpr> operator!=(const NumericValue& other) const {
        return std::make_shared<ComparisonExpr>(value, other.value, "!=");
    }
    std::shared_ptr<ConditionExpr> operator>(const NumericValue& other) const {
        return std::make_shared<ComparisonExpr>(value, other.value, ">");
    }
    std::shared_ptr<ConditionExpr> operator>=(const NumericValue& other) const {
        return std::make_shared<ComparisonExpr>(value, other.value, ">=");
    }
    std::shared_ptr<ConditionExpr> operator<(const NumericValue& other) const {
        return std::make_shared<ComparisonExpr>(value, other.value, "<");
    }
    std::shared_ptr<ConditionExpr> operator<=(const NumericValue& other) const {
        return std::make_shared<ComparisonExpr>(value, other.value, "<=");
    }
};

class StringValue {
    std::function<std::string(Fighter*, Fighter*)> value;
public:
    StringValue(const std::string& val) 
        : value([val](Fighter*, Fighter*) { return val; }) {}
    StringValue(std::function<std::string(Fighter*, Fighter*)> val) : value(val) {}
    
    std::function<std::string(Fighter*, Fighter*)> getValue() const { return value; }
    
    std::shared_ptr<ConditionExpr> operator==(const std::string& other) const {
        return std::make_shared<StringComparisonExpr>(value, other, "==");
    }
    std::shared_ptr<ConditionExpr> operator!=(const std::string& other) const {
        return std::make_shared<StringComparisonExpr>(value, other, "!=");
    }
};

class BoolValue {
    std::function<bool(Fighter*, Fighter*)> value;
public:
    BoolValue(bool val) : value([val](Fighter*, Fighter*) { return val; }) {}
    BoolValue(std::function<bool(Fighter*, Fighter*)> val) : value(val) {}
    
    std::shared_ptr<ConditionExpr> toCondition() const {
        auto func = value;
        return std::make_shared<ComparisonExpr>(
            [func](Fighter* a, Fighter* d) { return func(a, d) ? 1.0 : 0.0; },
            [](Fighter*, Fighter*) { return 1.0; },
            "=="
        );
    }
};

// ========== SHOW BUILDER ==========

class ShowBuilder {
    std::shared_ptr<ShowCommand> cmd;
public:
    ShowBuilder() : cmd(std::make_shared<ShowCommand>()) {}
    
    ShowBuilder& operator<<(const std::string& text) {
        cmd->addPart([text](Fighter*, Fighter*) { return text; });
        return *this;
    }
    
    ShowBuilder& operator<<(NumericValue val) {
        auto func = val.getValue();
        cmd->addPart([func](Fighter* a, Fighter* d) { 
            return std::to_string((int)func(a, d)); 
        });
        return *this;
    }
    
    ShowBuilder& operator<<(StringValue val) {
        auto func = val.getValue();
        cmd->addPart([func](Fighter* a, Fighter* d) { return func(a, d); });
        return *this;
    }
    
    operator std::shared_ptr<Command>() const {
        return std::static_pointer_cast<Command>(cmd);
    }
    
    std::shared_ptr<Command> build() const {
        return std::static_pointer_cast<Command>(cmd);
    }
};

// ========== BATTLE SYSTEM ==========

inline void runDuel() {
    std::cout << "=== Available Fighters ===" << std::endl;
    int idx = 1;
    std::vector<std::string> fighterNames;
    for (const auto& pair : fighterRegistry) {
        std::cout << idx++ << ". " << pair.first << " (" << pair.second->type 
                  << ", HP: " << pair.second->maxHP << ")" << std::endl;
        fighterNames.push_back(pair.first);
    }
    
    std::cout << "\nPlayer 1, select your fighter (1-" << fighterNames.size() << "): ";
    int choice1;
    std::cin >> choice1;
    
    std::cout << "Player 2, select your fighter (1-" << fighterNames.size() << "): ";
    int choice2;
    std::cin >> choice2;
    
    // Create fresh copies of fighters for battle
    auto origFighter1 = fighterRegistry[fighterNames[choice1-1]];
    auto origFighter2 = fighterRegistry[fighterNames[choice2-1]];
    
    auto fighter1 = std::make_shared<Fighter>(origFighter1->name, origFighter1->type, origFighter1->maxHP);
    auto fighter2 = std::make_shared<Fighter>(origFighter2->name, origFighter2->type, origFighter2->maxHP);
    
    // Copy abilities
    for (auto& ability : origFighter1->abilities) {
        fighter1->addAbility(ability);
    }
    for (auto& ability : origFighter2->abilities) {
        fighter2->addAbility(ability);
    }
    
    std::cout << "\n=== BATTLE START ===" << std::endl;
    std::cout << fighter1->name << " VS " << fighter2->name << std::endl << std::endl;
    
    int round = 1;
    bool player1Turn = true;
    
    while (fighter1->isAlive() && fighter2->isAlive()) {
        std::cout << "=== Round " << round << " ===" << std::endl;
        
        // Grappler healing on even rounds
        if (round % 2 == 0) {
            if (fighter1->type == "Grappler" && fighter1->inRing) {
                double healAmount = fighter1->maxHP * 0.05;
                fighter1->heal(healAmount);
                std::cout << fighter1->name << " (Grappler) heals " << (int)healAmount 
                         << " HP at start of round!" << std::endl;
            }
            if (fighter2->type == "Grappler" && fighter2->inRing) {
                double healAmount = fighter2->maxHP * 0.05;
                fighter2->heal(healAmount);
                std::cout << fighter2->name << " (Grappler) heals " << (int)healAmount 
                         << " HP at start of round!" << std::endl;
            }
        }
        
        Fighter* attacker = player1Turn ? fighter1.get() : fighter2.get();
        Fighter* defender = player1Turn ? fighter2.get() : fighter1.get();
        
        // Process delayed and recurring commands
        attacker->processDelayedCommands(defender, round);
        attacker->processRecurringCommands(defender, round);
        
        if (!attacker->inRing) {
            std::cout << attacker->name << " is out of the ring and cannot attack!" << std::endl;
        } else if (attacker->abilities.empty()) {
            std::cout << attacker->name << " has no abilities!" << std::endl;
        } else {
            std::cout << (player1Turn ? "Player 1" : "Player 2") << " (" << attacker->name << "), select ability:" << std::endl;
            for (size_t i = 0; i < attacker->abilities.size(); i++) {
                std::cout << (i+1) << ". " << attacker->abilities[i]->name << std::endl;
            }
            
            int abilityChoice;
            std::cin >> abilityChoice;
            
            if (abilityChoice >= 1 && abilityChoice <= (int)attacker->abilities.size()) {
                attacker->abilities[abilityChoice-1]->use(attacker, defender, round);
            }
        }
        
        std::cout << std::endl;
        fighter1->displayStatus();
        fighter2->displayStatus();
        
        player1Turn = !player1Turn;
        if (player1Turn) round++;
    }
    
    std::cout << "=== BATTLE END ===" << std::endl;
    if (fighter1->isAlive()) {
        std::cout << fighter1->name << " WINS!" << std::endl;
    } else {
        std::cout << fighter2->name << " WINS!" << std::endl;
    }
}

// ========== HELPER FUNCTIONS ==========

inline std::shared_ptr<Fighter> createFighter(const std::string& name, const std::string& type, double hp) {
    auto fighter = std::make_shared<Fighter>(name, type, hp);
    fighterRegistry[name] = fighter;
    return fighter;
}

inline std::shared_ptr<Ability> createAbility(const std::string& name, std::shared_ptr<Command> action) {
    auto ability = std::make_shared<Ability>(name);
    ability->setAction(action);
    abilityRegistry[name] = ability;
    return ability;
}

inline void teachAbility(const std::string& fighterName, const std::string& abilityName) {
    if (fighterRegistry.find(fighterName) != fighterRegistry.end() &&
        abilityRegistry.find(abilityName) != abilityRegistry.end()) {
        fighterRegistry[fighterName]->addAbility(abilityRegistry[abilityName]);
    }
}

inline NumericValue GET_HP(bool isAttacker) {
    return NumericValue([isAttacker](Fighter* a, Fighter* d) {
        return isAttacker ? a->currentHP : d->currentHP;
    });
}

inline StringValue GET_TYPE(bool isAttacker) {
    return StringValue([isAttacker](Fighter* a, Fighter* d) {
        return isAttacker ? a->type : d->type;
    });
}

inline StringValue GET_NAME(bool isAttacker) {
    return StringValue([isAttacker](Fighter* a, Fighter* d) {
        return isAttacker ? a->name : d->name;
    });
}

inline BoolValue IS_OUT_OF_RING(bool isAttacker) {
    return BoolValue([isAttacker](Fighter* a, Fighter* d) {
        return isAttacker ? !a->inRing : !d->inRing;
    });
}

// ========== DSL MACROS ==========

#define BEGIN_GAME int main() { try {
#define END_GAME } catch(const std::exception& e) { std::cerr << "Error: " << e.what() << std::endl; } return 0; }

#define ATTACKER true
#define DEFENDER false

#define DAMAGE_DEFENDER(amt) std::make_shared<DamageCommand>(true, amt)
#define DAMAGE_ATTACKER(amt) std::make_shared<DamageCommand>(false, amt)
#define HEAL_DEFENDER(amt) std::make_shared<HealCommand>(true, amt)
#define HEAL_ATTACKER(amt) std::make_shared<HealCommand>(false, amt)
#define TAG_DEFENDER_OUT std::make_shared<TagCommand>(true, true)
#define TAG_DEFENDER_IN std::make_shared<TagCommand>(true, false)
#define TAG_ATTACKER_OUT std::make_shared<TagCommand>(false, true)
#define TAG_ATTACKER_IN std::make_shared<TagCommand>(false, false)

#define FOR_ROUNDS(num, cmd) std::make_shared<ForRoundsCommand>(num, cmd)
#define AFTER_ROUNDS(num, cmd) std::make_shared<AfterRoundsCommand>(num, cmd)

#define IF_THEN(cond, thenCmd) std::make_shared<IfCommand>(cond, thenCmd)
#define IF_THEN_ELSE(cond, thenCmd, elseCmd) std::make_shared<IfCommand>(cond, thenCmd, elseCmd)

#define AND(c1, c2) std::make_shared<AndExpr>(c1, c2)
#define OR(c1, c2) std::make_shared<OrExpr>(c1, c2)
#define NOT(c) std::make_shared<NotExpr>(c)

#define DUEL runDuel();

#endif // TEKKEN_H
