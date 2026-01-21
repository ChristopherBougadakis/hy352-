# Tekken DSL & Battle Engine (C++)

Ένα header-only mini-DSL για μάχες μεταξύ "Fighters" με abilities, commands, συνθήκες και μακροεντολές. Ο κώδικας ζει στο `Tekken.h` και χρησιμοποιείται από απλά προγράμματα όπως τα `example_simple.cpp`, `example_advanced.cpp`, `demo.cpp`, `test_battle.cpp`.

## Γρήγορη Εκκίνηση

- Build:
  ```bash
  make
  ```
- Τρέξιμο απλού παραδείγματος:
  ```bash
  ./example_simple
  ```
- Τρέξιμο προχωρημένου παραδείγματος:
  ```bash
  ./example_advanced
  ```

## Δομή

- `Tekken.h`: Όλη η υλοποίηση του DSL και της engine.
- `example_simple.cpp`, `example_advanced.cpp`, `demo.cpp`, `test_battle.cpp`: Χρήσεις του DSL.
- `Makefile`: Κτίζει τα παραδείγματα.

## Blocks ανά λειτουργικότητα

### Command System
- **`Command`**: Βασική διεπαφή. 
  - Μέθοδοι: `execute(Fighter* attacker, Fighter* defender, int round)`, `clone()`.
- **`CompositeCommand`**: Συνδυάζει πολλές εντολές σε σειρά.
  - Χρήση: `add(std::shared_ptr<Command>)` για να προσθέσεις υπο-commands.
- **`DamageCommand`**: Κάνει ζημιά είτε στον αμυνόμενο είτε στον επιτιθέμενο.
  - Κατασκευαστής: `(bool isDefender, double amount)`.
  - `execute`: Καλεί `takeDamage(...)` στον στόχο.
- **`HealCommand`**: Θεραπεύει είτε τον αμυνόμενο είτε τον επιτιθέμενο.
  - Κατασκευαστής: `(bool isDefender, double amount)`.
- **`TagCommand`**: Βάζει/βγάζει έναν fighter από το ring.
  - Κατασκευαστής: `(bool isDefender, bool out)`.
  - `out=true` → leave; `out=false` → enter.
- **`ForRoundsCommand`**: Προγραμματίζει επαναλαμβανόμενη εκτέλεση ενός command για N γύρους.
  - Κατασκευαστής: `(int rounds, std::shared_ptr<Command> cmd)`.
  - `execute`: `attacker->addRecurringCommand(rounds, cmd)`.
- **`AfterRoundsCommand`**: Καθυστερεί την εκτέλεση ενός command για N γύρους.
  - Κατασκευαστής: `(int rounds, std::shared_ptr<Command> cmd)`.
  - `execute`: Προσθέτει delayed command στον defender (με ειδική μεταχείριση για `TAG_DEFENDER_IN`).
- **`IfCommand`**: Εκτελεί `thenCmd` ή `elseCmd` ανάλογα με συνθήκη.
  - Κατασκευαστής: `(ConditionExpr, then, else)`.
- **`ShowCommand`**: Εκτυπώνει δυναμικά τμήματα κειμένου/τιμών.
  - `addPart(function<string(Fighter*, Fighter*)>)` για τμήματα.

### Fighter
- Πεδία: `name`, `type`, `maxHP`, `currentHP`, `inRing`, `abilities`, `delayedCommands`, `recurringCommands`.
- Μέθοδοι:
  - **`takeDamage(amount, attacker, round)`**: Εφαρμόζει bonus/αντιστάσεις ανά τύπο:
    - Rushdown: +15% (+20% vs Grappler)
    - Evasive: +7%
    - Grappler: +7% σε μονό γύρο
    - Heavy (defender): -20% (ή -30% vs Evasive), Evasive (defender): -7%
    - Κατώτατο όριο HP: 0
  - **`heal(amount)`**: Θεραπεία μέχρι `maxHP`.
  - **`leaveRing()` / `enterRing()`**: Κατάσταση ring.
  - **`isAlive()`**: `currentHP > 0`.
  - **`addAbility(Ability)`**, **`addDelayedCommand(rounds, cmd)`**, **`addRecurringCommand(rounds, cmd)`**.
  - **`processDelayedCommands(defender, round)`**: Εκτελεί commands όταν λήξουν οι γύροι.
  - **`processRecurringCommands(defender, round)`**: Εκτελεί κάθε γύρο μέχρι να μηδενίσει ο μετρητής.
  - **`displayStatus()`**: Εμφάνιση στοιχείων.

### Ability
- Πεδία: `name`, `action` (`Command`).
- Μέθοδοι:
  - **`setAction(cmd)`**: Ορίζει ενέργεια.
  - **`use(attacker, defender, round)`**: Εκτελεί `action->execute` εφόσον υπάρχει.

### Condition System
- **`ConditionExpr`**: Βάση για λογικές εκφράσεις.
- **`ComparisonExpr`** (αριθμητικές): `== != > >= < <=` με `double`.
- **`StringComparisonExpr`** (αλφαριθμητικές): `== !=`.
- **`AndExpr`**, **`OrExpr`**, **`NotExpr`**: Σύνθετες λογικές εκφράσεις.
  - Όλες παρέχουν `evaluate(attacker, defender)` και `clone()`.

### Value Wrappers
- **`NumericValue`**: Τυλιγμένα `double` με operators για συνθήκες.
- **`StringValue`**: Τυλιγμένα `string` με `==`, `!=`.
- **`BoolValue`**: Μετατροπή σε `ConditionExpr` μέσω `toCondition()`.

### ShowBuilder
- Φτιάχνει εύκολα `ShowCommand`:
  - Υποστηρίζει `<< string`, `<< NumericValue`, `<< StringValue`.
  - `build()` ή implicit μετατροπή σε `Command`.

### Battle System
- **`runDuel()`**:
  - Εμφανίζει διαθέσιμους fighters από `fighterRegistry`.
  - Ζητά επιλογές παικτών, δημιουργεί "φρέσκα" αντίγραφα.
  - Κάθε γύρος:
    - Grappler heals 5% σε ζυγούς γύρους εφόσον `inRing`.
    - Εκτελεί delayed/recurring για τον επιτιθέμενο.
    - Αν είναι στο ring και έχει abilities, ο παίκτης διαλέγει και εκτελεί.
    - Εκτυπώνει status.
  - Τέλος: τυπώνει νικητή.

### Helper Functions
- **`createFighter(name, type, hp)`**: Φτιάχνει fighter και γράφει στο `fighterRegistry`.
- **`createAbility(name, action)`**: Φτιάχνει ability, ορίζει `action`, γράφει στο `abilityRegistry`.
- **`teachAbility(fighterName, abilityName)`**: Δίνει ability σε fighter αν υπάρχουν στα registries.
- **Getters**:
  - `GET_HP(isAttacker)`, `GET_TYPE(isAttacker)`, `GET_NAME(isAttacker)`, `IS_OUT_OF_RING(isAttacker)`.

### DSL Macros
- **Ροή προγράμματος**:
  - `BEGIN_GAME` ... `END_GAME`
  - `DUEL` για να τρέξει το `runDuel()`
- **Στόχοι**: `ATTACKER`, `DEFENDER`
- **Damage/Heal/Tag**:
  - `DAMAGE_DEFENDER(amt)`, `DAMAGE_ATTACKER(amt)`
  - `HEAL_DEFENDER(amt)`, `HEAL_ATTACKER(amt)`
  - `TAG_DEFENDER_OUT`, `TAG_DEFENDER_IN`, `TAG_ATTACKER_OUT`, `TAG_ATTACKER_IN`
- **Χρονισμός**:
  - `FOR_ROUNDS(n, cmd)`, `AFTER_ROUNDS(n, cmd)`
- **Λογική**:
  - `IF_THEN(cond, thenCmd)`, `IF_THEN_ELSE(cond, thenCmd, elseCmd)`
  - `AND(c1, c2)`, `OR(c1, c2)`, `NOT(c)`

## Μικρά Παραδείγματα DSL

### 1) Δημιουργία fighters & abilities
```cpp
BEGIN_GAME
  auto jin = createFighter("Jin", "Rushdown", 120);
  auto king = createFighter("King", "Grappler", 140);

  auto punch = createAbility("Punch", DAMAGE_DEFENDER(15));
  auto heal = createAbility("Recover", HEAL_ATTACKER(10));

  teachAbility("Jin", "Punch");
  teachAbility("Jin", "Recover");
  teachAbility("King", "Punch");

  DUEL
END_GAME
```

### 2) Συνθήκη βάσει HP/Type
```cpp
auto finisher = IF_THEN_ELSE(
  GET_HP(DEFENDER) <= NumericValue(30),
  DAMAGE_DEFENDER(40),
  DAMAGE_DEFENDER(20)
);

auto typeCheck = IF_THEN(
  GET_TYPE(DEFENDER) == std::string("Heavy"),
  DAMAGE_DEFENDER(25)
);
```

### 3) Χρονισμός & Tagging
```cpp
auto vanish = TAG_ATTACKER_OUT;
auto comeback = AFTER_ROUNDS(2, TAG_DEFENDER_IN); // θα μετατραπεί ώστε να μπει ο επιτιθέμενος

auto dot = FOR_ROUNDS(3, DAMAGE_DEFENDER(5)); // 3 γύροι συνεχόμενα
```

### 4) ShowBuilder
```cpp
ShowBuilder sb;
sb << "Attacker: " << GET_NAME(ATTACKER) << ", HP=" << GET_HP(ATTACKER);
auto showStatus = sb.build();
```

## Σημειώσεις Ισορροπίας
- Τα bonuses/αντιστάσεις στον `takeDamage` επηρεάζονται από `type`.
- Οι `Grappler` θεραπεύονται 5% του `maxHP` στην αρχή ζυγών γύρων αν είναι `inRing`.
- Τα delayed/recurring commands εκτελούνται με βάση τους μετρητές γύρων.

## Δοκιμές/Τρέξιμο
- Μετά το `make`, τρέξτε τα binaries:
  ```bash
  ./example_simple
  ./example_advanced
  ./demo
  ./test_battle
  ```

## Συμβουλές Χρήσης
- Κρατήστε τα abilities μικρά και συνθέστε τα με `CompositeCommand` όταν χρειάζεται.
- Χρησιμοποιήστε `IF_THEN/ELSE` με `GET_HP`, `GET_TYPE`, `IS_OUT_OF_RING` για έξυπνες αποφάσεις.
- Προτιμήστε `FOR_ROUNDS` για effects τύπου "damage-over-time" και `AFTER_ROUNDS` για καθυστερήσεις/επιστροφές.

---
Για επέκταση: προσθέστε νέους τύπους `Command`, νέους getters, ή κανόνες ισορροπίας στο `takeDamage` σύμφωνα με τις ανάγκες σας.
