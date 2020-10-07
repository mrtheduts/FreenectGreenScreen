#ifndef CMD_LINE_OPTS_HPP
#define CMD_LINE_OPTS_HPP

class CmdLineOpts {
 public:
  CmdLineOpts();

  //                       Default values
  int starting_angle;   // 0
  int bg_threshold;     // 1000
  bool use_bg_img;      // false
  char const* bg_path;  // NULL
  bool blur_bg;         // false
  int blur_int;         // 3
  bool show_cv_window;  // false
  bool verbose;         // false
};

#endif
