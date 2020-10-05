#include <unistd.h>

#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <opencv2/opencv.hpp>

#include "MyFreenectDevice.hpp"
#include "utils.hpp"

int main(int argc, char* const* args) {
  if (argc < 2) {
    PrintHelp(args[0]);
    exit(-1);
  }

  int starting_degree = 0;
  bool showCV(false);
  bool verbose(false);

  {
    int opt = 0;
    opterr = 0;

    while ((opt = getopt(argc, args, ":d:iwvh")) != -1) {
      switch (opt) {
        case 'd':
          std::sscanf(optarg, "%d", &starting_degree);
          if(starting_degree < -30 || starting_degree > 30) {
            std::cerr << "Tilt degree allowed only between -30 and 30. Using ";
            starting_degree = (starting_degree < 0 ? -30 : 30);
            std::cerr << starting_degree << "." << std::endl;
          }
          break;
        case 'i':
          std::cerr << "Background image not yet implemented.";
          break;
        case 'w':
          showCV = true;
          break;
        case 'v':
          verbose = true;
          break;
        case '?':
          std::cerr << "Unkown option: " << (char)opt << std::endl;
          PrintHelp(args[0]);
          break;
        case 'h':
          PrintHelp(args[0]);
          exit(0);
      }
    }
  }

  if(optind == argc) {
    std::cerr << "Number of arguments wrong." << std::endl;
    PrintHelp(args[0]);
    exit(-1);
  }
  const char* dev_video = args[optind];
  size_t vid_send_size = WIDTH * HEIGHT * 3;

  if (verbose) std::cout << "Opening v4l2loop device..." << std::endl;

  int v4l2lo_fd = OpenV4l2loop(dev_video, vid_send_size, WIDTH, HEIGHT);

  cv::Mat depthMat(cv::Size(WIDTH, HEIGHT), CV_16UC1);
  cv::Mat depthf(cv::Size(WIDTH, HEIGHT), CV_8UC1);
  cv::Mat ownMat(cv::Size(WIDTH, HEIGHT), CV_8UC3, cv::Scalar(0));
  cv::Mat rgbMat;

  Freenect::Freenect freenect;
  MyFreenectDevice& device = freenect.createDevice<MyFreenectDevice>(0);

  device.setTiltDegrees(starting_degree);

  if (showCV) {
    namedWindow("rgb", cv::WINDOW_AUTOSIZE);
    // namedWindow("depth", cv::WINDOW_AUTOSIZE);
  }

  device.startVideo();
  device.startDepth();

  bool die(false);
  std::unique_lock<std::mutex> rgb_lck(device.new_rgb_frame_mutex);

  while (!die) {
    device.cond_var.wait(rgb_lck);
    device.getVideo(rgbMat);
    // device.getDepth(depthMat);

    if (showCV) {
      cv::imshow("rgb", rgbMat);
      // depthMat.convertTo(depthf, CV_8UC1, 255.0 / 2048.0);
      // cv::imshow("depth", depthf);

      char k = cv::waitKey(5);
      if (k == 27) {
        cv::destroyAllWindows();
        break;
      }
    }

    int written = write(v4l2lo_fd, rgbMat.data, vid_send_size);
    if (written < 0) {
      std::cout << "Error writing v4l2l device";
      close(v4l2lo_fd);
      return 1;
    }
  }

  close(v4l2lo_fd);
  device.stopVideo();
  device.stopDepth();
  return 0;
}
