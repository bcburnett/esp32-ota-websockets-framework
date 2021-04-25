#ifndef STATE
#define STATE

//set initial state
struct State {
  int slider1;
  int slider2;
  String filename;
  bool reload;
  String json;
  bool ota;
  bool alarmSet;
  int alarmHour;
  int alarmMinute;
};
State state = {0, 0, "", false, "", false, false, -1, -1};
#endif
