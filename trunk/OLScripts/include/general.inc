// General Open Zelda Functions

#if defined _general_included
  #endinput
#endif

#include <float.inc>

//The only function you'll ever need
native DebuggingStuff(randomvalue);

//Group size retreival
native GetGropuWidth();
native GetGroupHeight();
native GetInput(key);
native GetMouseX();
native GetMouseY();
native GetMouseKey();
native float: GetTimeDelta();
native UpdateWorldCo(x, y, groupid[] = "");