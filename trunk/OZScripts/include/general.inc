// General Open Zelda Functions

#if defined _general_included
  #endinput
#endif
#define _general_included


// Define 8 Directions
enum _Directions{
  north,
  east,
  south,
  west,
  northeast,
  southeast,
  southwest,
  northwest,
  }


native PutSprite(ident[], valx, valy, vald, layer = 0, r = 255, g = 255, b = 255, a = 255, rot = 0, scale = 100);
native DrawImage(code[], x, y, r = 255, g = 255, b = 255, a = 255, rot = 0, scale = 100);
native GetWidth(ident[]);
native GetHeight(ident[]);
native QKey();
native AKey();
native SKey();
native WKey();
native EnterKey();
native UpKey();
native DownKey();
native LeftKey();
native RightKey();
native SetQKey(val);
native SetAKey(val);
native SetSKey(val);
native SetWKey(val);
native SetEnterKey(val);
native SetUpKey(val);
native SetDownKey(val);
native SetLeftKey(val);
native SetRightKey(val);
native SetWorldX(val);
native SetWorldY(val);
native GetWorldX();
native GetWorldY();
native GetGroupMinX();
native GetGroupMinY();
native GetGroupMaxX();
native GetGroupMaxY();
native NearPoint(x1, y1, x2, y2, dist);
native SetPauseLevel(val);
native GetPauseLevel();
native FirstRun();
native UpdateWorldCo(x, y);
native SetFilter(filter, ...);
native InitialPlayerX();
native InitialPlayerY();
native CheckMask(x, y, checkForObjects = 0);
native CheckForHoleXY(x,y);
native FadeTo(red, green, blue, speed);
native Wipe(direction, speed);
native CalculateAngle(x1,y1,x2,y2);

// Saving loading functions
native SetRestartTable(index, x, y, desc[] );
native SetRestartPoint(x, y);
native ClearRestartTable();

// Text Drawing Functions
native SetTextSize( size );
native DrawText(string[], x, y, red = 255, green = 255, blue = 255, alpha = 255);
native DrawBitmapText(string[], x, y, style = 1, red = 255, green = 255, blue = 255, alpha = 255, scale = 200);

// Functions for drawing shapes
native DrawRectangle(x1, y1, x2, y2, red, green, blue, alpha = 255, angle = 0);
native DrawLine(x1, y1, x2, y2, width, red, green, blue, aplha = 255);

// Enables or Disables user input
native EnableControls( enable );

// Copies a numeric value in val to the string
native ToString(val, string[]);

// Sets and Gets the screen something falls to when it falls down a hole
native SetLowerLevel(x, y);
native GetLowerLevelX();
native GetLowerLevelY();

// TextBox Functions
native InitTextBox(string[], wid, priority);
native SetTextBoxColor(red, green, blue, alpha);
native FinishedReading();
native GetLastAnswer();

// Time Functions for Day/Night
native GetMinuteCount();
native GetHourCount();
native GetDayCount();
native SetMinuteCount(val);
native SetHourCount(val);
native SetDayCount(val);
native SetDayLength(val);

// Global\Local Functions
native SetGlobal(index, val);
native GetGlobal(index);
native SetLocal(index, val);
native GetLocal(index);

// Sound functions
native PlaySound(code[], volume = 128);
native SetMusicVolume(volume);
native PlayMusic(code[], fade);
native SetMusicFadeSpeed(speed = 110);

// Starts a gameover screen
native GameOver();

// Used for changing the alpha value of a background sprite
native SetSpriteAlpha( spriteCode[], newAlpha);

// Debugging functions
native ToFile(value);
native TextFile(string[]);