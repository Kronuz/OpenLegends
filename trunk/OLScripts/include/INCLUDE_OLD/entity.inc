// Open Legends Entity Functions

#if defined _entity_included
  #endinput
#endif
#define _entity_included


// Define General Entity Types
enum _Types{
     otherType,     // Other
     playerType,    // Main Player	
     enemyType,     // Enemy
     npcType,       // NPC
     weaponType,    // Weapon
     itemType,      // Item
     doorType,       // Door
     switchType       // Switch
     }


// Define a set of states which an entity can be in
enum _States{
     standing,
     walking,
     hit,
     knocked,
     jumping,
     swimming,
     floating,
     chasing,
     using,
     lifting,
     standLift,
     walkLift,
     dying,
     dead,
     falling,
     falling2,
     stunned,
     frozen,
     shocked,
     burning,
     sleeping,
     waiting,
     leaping, 
     pushing,
     pulling
     }


// Entity Flag functions
native isOpen(ident[]);
native SetOpenFlag(ident[], val);
native isTaken(ident[]);
native SetTakenFlag(ident[], val);
native isDead(ident[]);
native SetDeadFlag(ident[], val);
native isVisible(ident[]);
native SetVisibleFlag(ident[], val);
native isActive(ident[]);
native SetActiveFlag(ident[], val);
native isCuttable(ident[]);
native SetCuttableFlag(ident[], val);
native isLarge(ident[]);
native SetLargeFlag(ident[], val);
native isPickable(ident[])
native SetPickableFlag(ident[], val);
native isPushed(ident[])
native SetPushedFlag(ident[], val);
native isInteracting(ident[])
native SetInteractingFlag(ident[], val);
native isOwned(ident[]);
native SetOwnedFlag(ident[], val);

// Other entity functions
native SetDamage(ident[], val);
native GetDamage(ident[]);
native SetLiftLevel(ident[], val);
native GetLiftLevel(ident[]);
native DeleteEntity(ident[]);
native GetX(ident[]);
native GetY(ident[]);
native SetX(ident[], val);
native SetY(ident[], val);
native SetPosition(ident[], x, y);
native GetMoveAngle(ident[]);
native SetMoveAngle(ident[], val);
native GetDirection(ident[]);
native SetDirection(ident[], val);
native GetSpeed(ident[]);
native SetSpeed(ident[], val);
native GetSpeedMod(ident[]);
native SetSpeedMod(ident[], val);
native AngleMove(ident[]);
native AngleCollide(ident[], val1, val2, val3, val4, val5, val6);
native SetAngleFromDir(ident[]);
native SetDirFromAngle(ident[]);
native ChangeDirection(ident[]);
native SetCollisionRect(ident[], index, solid, left, top, right, bottom);
native ClearCollisionRect(ident[], index);
native SetItem(ident[], code[]);
native GetItem(ident[], code[]);
native SetImage(ident[], code[]);
native GetImage(ident[], code[]);
native SetType(ident[], val);
native GetType(ident[]);
native Collide(ident[], ident2[]);
native CollideAll(ident[], ident2[]);
native AllocateStrings(ident[], numstr, length);
native GetString(ident[], index, string[]);
native SetString(ident[], index, string[]);
native SetActiveDist(ident[], val);
native SetState(ident[], val);
native GetState(ident[]);
native GetInitialX(ident[]);
native GetInitialY(ident[]);
native GetEntityCount();
native GetAnglePointX(ident[], dist, xoffset);
native GetAnglePointY(ident[], dist, yoffset);
native CollidePoint(ident[], x, y);
native GetWeight(ident[]);
native SetWeight(ident[], val);
native GetBounceValue(ident[]);
native SetBounceValue(ident[], val);
native CheckForHole(ident[]);
native SetValue(ident[], index, val);
native GetValue(ident[], index);
native SetFloat(ident[], index, floatval);
native GetFloat(ident[], index);
native SetHealth(ident[], val);
native SetMaxHealth(ident[], val);
native GetHealth(ident[]);
native GetMaxHealth(ident[]);
native Respawn(ident[], seconds);
native CallFunction(ident[], isLibrary, funcname[], typestr[], ...);

// Set this to true to allow this entity to move to and be active in groups other
// than the one it was created in, the default is for this to be false.
native SetActiveInGroups(ident[], val);

// Functions for moving through the list of Entities
native StartEntity(dist = 40, x=-999, y=-999);
native GetCurrentEntity();
native NextEntity();

// Message Map Functions
native MessageMap(ident[], index, Message[], gotoNext, ...);
native GetNextMessage(ident[], index, index2);
native GetGotoNextMessage(ident[], index);
native GetNumNextMessages(ident[], index);

// Function for getting the script's parameter character
native GetParam( ident[] );

// Gets the identifier of the entity which created this one
native GetParent( ident[], buffer[] );

// Functions for creating entities on the fly, both with and without preset identifiers
native CreateEntity(code[], x, y, ident[]);
native CreateEntityWithID(code[], x, y, ident[]);



