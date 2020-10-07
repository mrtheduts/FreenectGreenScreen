#include "utils.hpp"

#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

void PrintHelp(char* const path) {
  std::cout << "Usage: " << path << " [options] /dev/video<N>" << std::endl;
  std::cout << "Where <N> is the v4l2loopback device number." << std::endl;
  std::cout << "options:" << std::endl;
  std::cout << "\t-d <angle>\t\tset the starting angle from -30 to 30 degrees "
               "(default: 0)"
            << std::endl;
  std::cout << "\t-t <threshold distance>\tset distance in mm to consider img "
               "as background (default: 1000)"
            << std::endl;
  std::cout << "\t-i <path/to/image>\tset a background image" << std::endl;
  std::cout << "\t-b <blur intensity>\tblur background with provided intensity "
               "(must be an odd natural number)"
            << std::endl;
  std::cout
      << "\t-w \t\t\topens a window to show video provided to virtual webcam"
      << std::endl;
  std::cout << "\t-v \t\t\tverbose" << std::endl;
  std::cout << "\t-h \t\t\tshows this help" << std::endl;
}

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

void InitialSetup(CmdLineOpts& opts, MyFreenectDevice& device) {
  if (opts.verbose)
    std::cout << "Tilting " << opts.starting_angle << " degrees..."
              << std::endl;
  device.setTiltDegrees(opts.starting_angle);

  if (opts.verbose)
    std::cout << "Setting depth detection to MM..." << std::endl;
  device.setDepthFormat(FREENECT_DEPTH_REGISTERED);

  if (opts.verbose) std::cout << "Starting RGB video capture..." << std::endl;
  device.startVideo();

  if (opts.verbose) std::cout << "Starting depth video capture..." << std::endl;
  device.startDepth();
}
