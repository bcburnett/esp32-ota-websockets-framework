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
};
State state = {0,0,"",false,"",false};
#endif 
