#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdio>
int OpenV4l2loop(const char* dev_video, size_t vid_send_size, int width,
                 int height);
void PrintHelp(char* const path);
#endif
