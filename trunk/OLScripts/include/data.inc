#if defined _data_included
  #endinput
#endif

//Strings (returns false(0) on entity not found)
native sGetString(Entity[], ID[], Output[]);
native vGetString(Entity[], ID,   Output[]);
native sSetString(Entity[], ID[], Input[]);
native vSetString(Entity[], ID,   Input[]);

//Integer values (returns -1 on entity not found)
native sGetValue( Entity[], ID[]);
native vGetValue( Entity[], ID);

//Returns false on entity not found
native sSetValue( Entity[], ID[], Input);
native vSetValue( Entity[], ID,   Input);

//Boolean flags (returns -1 on entity not found)
native sGetFlag(  Entity[], ID[]);
native vGetFlag(  Entity[], ID);

//Returns false on entity not found
native sSetFlag(  Entity[], ID[], Input);
native vSetFlag(  Entity[], ID,   Input);