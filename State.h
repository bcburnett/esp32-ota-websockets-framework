#ifndef STATE
#define STATE
//set initial state
struct State { 
  int slider1;
  int slider2;
  String filename;
  bool reload;
  String json;
};
State state = {0,0};
#endif 
