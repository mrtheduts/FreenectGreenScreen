#include "CmdLineOpts.hpp"

#include <cstddef>

CmdLineOpts::CmdLineOpts()
    : starting_angle(0),
      bg_threshold(1000),
      use_bg_img(false),
      bg_path(NULL),
      blur_bg(false),
      blur_int(3),
      show_cv_window(false),
      verbose(false) {}
