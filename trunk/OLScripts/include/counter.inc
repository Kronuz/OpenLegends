// Open Legends Counter Object Functions

#if defined _counter_included
  #endinput
#endif
#define _counter_included


native CreateCounter(min, max, ident[]);
native CreateCounterWithID(min, max, ident[]);
native SetCounterMin(ident[], val);
native SetCounterMax(ident[], val);
native SetCounterValue(ident[], val);
native SetCounterTarget(ident[], val);
native IncCounterTarget(ident[], val);
native IncCounterValue(ident[], val);
native GetCounterValue(ident[]);
native SetCounterSpeed(ident[], val);
native DeleteCounter(ident[]);




