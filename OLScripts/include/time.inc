/* Date/time functions
 *
 * (c) Copyright 2001, ITB CompuPhase
 * This file is provided as is (no warranties).
 */
#if defined _time_included
  #endinput
#endif
#define _time_included

native time(&hour=0,&minute=0,&second=0);
native date(&year=0,&month=0,&day=0);

native tickcount(&granularity=0);

