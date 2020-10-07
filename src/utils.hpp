#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdio>

#include "CmdLineOpts.hpp"
#include "MyFreenectDevice.hpp"

void PrintHelp(char* const path);

// Opens and returns the v4l2loop file descriptor (/dev/videoN) and configures
// it to use YUV420 pixel format
int OpenV4l2loop(const char* dev_video, size_t vid_send_size, int width,
                 int height);

void InitialSetup(CmdLineOpts& opts, MyFreenectDevice& device);
#endif
