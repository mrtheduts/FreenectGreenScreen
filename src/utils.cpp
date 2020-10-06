#include "utils.hpp"

#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

int OpenV4l2loop(const char* dev_video, size_t vid_send_size, int width,
                 int height) {
  int v4l2lo_fd = open(dev_video, O_WRONLY);

  if (v4l2lo_fd < 0) {
    std::cout << "Error opening v4l2l device: " << strerror(errno);
    exit(-2);
  }

  struct v4l2_format v;
  int t;

  v.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

  t = ioctl(v4l2lo_fd, VIDIOC_G_FMT, &v);
  if (t < 0) {
    exit(t);
  }

  v.fmt.pix.width = width;
  v.fmt.pix.height = height;
  v.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
  v.fmt.pix.sizeimage = vid_send_size;
  v.fmt.pix.field = V4L2_FIELD_NONE; 

  t = ioctl(v4l2lo_fd, VIDIOC_S_FMT, &v);
  if (t < 0) {
    exit(t);
  }

  return v4l2lo_fd;
}

void PrintHelp(char* const path) {
  std::cout << "Usage: " << path << " [options] /dev/video<N>"  << std::endl;
  std::cout << "Where <N> is the v4l2loopback device number." << std::endl;
  std::cout << "options:" << std::endl;
  std::cout << "\t-d starting degree (from -30 to 30)" << std::endl;
  std::cout << "\t-i <path/to/image> path to background image" << std::endl;
  std::cout << "\t-w show OpenCV window to show current image" << std::endl;
  std::cout << "\t-v verbose" << std::endl;
  std::cout << "\t-h shows this help" << std::endl;
}
