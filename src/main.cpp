#include <unistd.h>

#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "MyFreenectDevice.hpp"
#include "utils.hpp"

int main(int argc, char* const* args) {
  if (argc < 2) {
    PrintHelp(args[0]);
    exit(-1);
  }

  int starting_degree = 0;
  int threshold = 1000;
  bool useBG(false);
  char const* bg_path = NULL;
  bool showCV(false);
  bool verbose(false);

  {
    int opt = 0;
    opterr = 0;

    while ((opt = getopt(argc, args, ":d:t:i:wvh")) != -1) {
      switch (opt) {
        case 'd':
          std::sscanf(optarg, "%d", &starting_degree);
          if (starting_degree < -30 || starting_degree > 30) {
            std::cerr << "Tilt degree allowed only between -30 and 30. Using ";
            starting_degree = (starting_degree < 0 ? -30 : 30);
            std::cerr << starting_degree << "." << std::endl;
          }
          break;
        case 't':
          std::sscanf(optarg, "%d", &threshold);
          break;
        case 'i':
          useBG = true;
          bg_path = optarg;
          std::cout << bg_path << std::endl;
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

  if (optind == argc) {
    std::cerr << "Number of arguments wrong." << std::endl;
    PrintHelp(args[0]);
    exit(-1);
  }
  const char* dev_video = args[optind];
  // size_t vid_send_size = WIDTH * HEIGHT * 3;
  size_t vid_send_size = 640 * 720;

  if (verbose) std::cout << "Opening v4l2loop device..." << std::endl;
  int v4l2lo_fd = OpenV4l2loop(dev_video, vid_send_size, WIDTH, HEIGHT);

  if (verbose) std::cout << "Creating matrices..." << std::endl;
  cv::Mat depthMat(cv::Size(WIDTH, HEIGHT), CV_16UC1);
  cv::Mat depthf(cv::Size(WIDTH, HEIGHT), CV_8UC1);
  cv::Mat ownMat(cv::Size(WIDTH, HEIGHT), CV_8UC3, cv::Scalar(0));
  cv::Mat rgbMat;

  if (verbose) std::cout << "Opening device..." << std::endl;
  Freenect::Freenect freenect;
  MyFreenectDevice& device = freenect.createDevice<MyFreenectDevice>(0);

  if (verbose)
    std::cout << "Tilting " << starting_degree << " degrees..." << std::endl;
  device.setTiltDegrees(starting_degree);

  if (verbose) std::cout << "Setting depth detection to MM..." << std::endl;
  device.setDepthFormat(FREENECT_DEPTH_REGISTERED);

  if (verbose) std::cout << "Starting RGB video..." << std::endl;
  device.startVideo();

  if (verbose) std::cout << "Starting depth video..." << std::endl;
  device.startDepth();

  if (showCV) {
    if (verbose) std::cout << "Starting OpenCV window..." << std::endl;
    namedWindow("rgb", cv::WINDOW_AUTOSIZE);
    // namedWindow("depth", cv::WINDOW_AUTOSIZE);
  }

  bool die(false);
  std::unique_lock<std::mutex> rgb_lck(device.new_rgb_frame_mutex);
  std::unique_lock<std::mutex> dep_lck(device.new_dep_frame_mutex);

  cv::Mat frame = cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC1);
  frame(cv::Rect(30, 40, 565, 440)) = 255;

  cv::Mat bg;
  if(useBG){
    bg = cv::imread(bg_path);
    cv::cvtColor(bg, bg, cv::COLOR_BGR2RGB);
    cv::resize(bg, bg, cv::Size(WIDTH, HEIGHT));
  }
  cv::Mat mask_bg;

  // cv::Mat d_blur(cv::Size(WIDTH, HEIGHT), CV_16UC1);
  cv::Mat mask(cv::Size(WIDTH, HEIGHT), CV_8UC1);
  cv::Mat final_img(cv::Size(WIDTH, HEIGHT), CV_8UC3);
  int erosion_size = 10;
  cv::Mat element = cv::getStructuringElement(
      0, cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
      cv::Point(erosion_size, erosion_size));

  if (verbose) std::cout << "Entering main loop..." << std::endl;
  while (!die) {
    device.cond_var.wait(rgb_lck);
    device.getVideo(rgbMat);

    device.cond_var.wait(dep_lck);
    device.getDepth(depthMat);

    // cv::threshold(depthMat, depthMat, 0, FREENECT_DEPTH_MM_MAX_VALUE,
    // cv::THRESH_BINARY + cv::THRESH_OTSU);
    cv::GaussianBlur(depthMat, depthMat, cv::Size(3, 3), 0);
    depthMat.setTo(FREENECT_DEPTH_MM_MAX_VALUE, depthMat >= threshold);
    depthMat.setTo(0, depthMat < threshold);
    cv::dilate(depthMat, depthMat, element);
    depthMat.convertTo(mask, CV_8UC1, 255.0 / FREENECT_DEPTH_MM_MAX_VALUE);
    // cv::resize(mask, mask_resized, cv::Size(565, 440), 0.5,1,
    // cv::INTER_AREA); mask(cv::Rect(30,40,565,440)) = mask_resized;
    if(useBG){
      mask_bg = mask.clone();
      cv::cvtColor(mask_bg, mask_bg, cv::COLOR_GRAY2RGB);
      mask_bg = bg & mask_bg;
    }
    mask = ~mask;
    mask = mask & frame;
    cv::cvtColor(mask, mask, cv::COLOR_GRAY2RGB);
    final_img = rgbMat & mask;

    if(useBG)
      final_img = final_img | mask_bg;

    // std::cout << "d_blur size: " << d_blur.size << d_blur.channels() <<
    // std::endl; std::cout << "d_blur size: " << d_blur.size <<
    // d_blur.channels() << std::endl; std::cout << "d_blur size: " <<
    // d_blur.size << std::endl; d_not= ~d_not; cv::bitwise_not(d_not, d_not);
    // d_not.convertTo(mask, CV_8UC3);
    // mask = ~mask;
    // cv::cvtColor(d_not, mask, cv::COLOR_GRAY2RGB);
    // std::cout << "d_not size: " << d_not.size << std::endl;
    // std::cout << "rgbMat size: " << rgbMat.size << std::endl;

    // cv::bitwise_and(rgbMat, mask, final_img);

    if (showCV) {
      cv::imshow("rgb", final_img);
      // cv::imshow("rgb", mask);
      // depthMat.convertTo(depthf, CV_8UC1, 255.0 / 2048.0);
      // cv::imshow("depth", mask);

      char k = cv::waitKey(5);
      if (k == 27) {
        cv::destroyAllWindows();
        break;
      }
    }

    cv::cvtColor(final_img, final_img, cv::COLOR_RGB2YUV_I420);
    int written = write(v4l2lo_fd, final_img.data, vid_send_size);
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
