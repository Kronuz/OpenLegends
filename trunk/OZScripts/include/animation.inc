// Open Zelda Animation Object Functions

#if defined _animation_included
  #endinput
#endif
#define _animation_included


native CreateAnim(speed, ident[]);
native CreateAnimWithID(speed, ident[]);
native AddAnimframe(ident[], x, y, code[]);
native SetAnimSpeed(ident[], speed);
native GetAnimImage(ident[], code[]);
native GetAnimCount(ident[]);
native SetAnimCount(ident[], val);
native DeleteAnim(ident[]);
native IncrementAnim(ident[]);
native GetMaxAnim(ident[]);
native GetAnimWidth(ident[]);
native GetAnimHeight(ident[]);
native SetAnimDirection(ident[], val);
native SetAnimLoop(ident[], doesloop);
native FinishedAnim(ident[]);
native DrawAnim(ident[], x, y, depth, layer = 0, r = 255, g = 255, b = 255, a = 255, rot = 0, scale = 100);
native DrawAnimNoInc(ident[], x, y, depth, layer = 0, r = 255, g = 255, b = 255, a = 255, rot = 0, scale = 100);



