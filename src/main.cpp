#include <unistd.h>

#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "CmdLineOpts.hpp"
#include "MyFreenectDevice.hpp"
#include "utils.hpp"

int main(int argc, char* const* args) {
  // It should receive at least one argument, /dev/videoN loopback device;
  if (argc < 2) {
    PrintHelp(args[0]);
    exit(-1);
  }

  CmdLineOpts opts;

  // Options parser block
  // d - Starting angle (in degrees)
  // t - Distance threshold to consider as background
  // i - Image to use instead of real background
  // b - Blur background - real or image
  // w - Show final image with OpenCV window
  // v - Prints log messages
  // h - Show help
  {
    int opt = 0;
    opterr = 0;

    while ((opt = getopt(argc, args, ":d:t:i:b:wvh")) != -1) {
      switch (opt) {
        case 'd':
          std::sscanf(optarg, "%d", &(opts.starting_angle));

          if (opts.starting_angle < -30 || opts.starting_angle > 30) {
            std::cerr << "Tilt degree allowed only between -30 and 30. Using ";
            opts.starting_angle = (opts.starting_angle < 0 ? -30 : 30);
            std::cerr << opts.starting_angle << "." << std::endl;
          }
          break;

        case 't':
          std::sscanf(optarg, "%d", &(opts.bg_threshold));

          if (opts.bg_threshold < 0 ||
              opts.bg_threshold > FREENECT_DEPTH_MM_MAX_VALUE) {
            std::cerr << "Threshold distance must be between 0 and "
                      << FREENECT_DEPTH_MM_MAX_VALUE << ". Using ";

            if (opts.bg_threshold < 0) opts.bg_threshold = 0;
            if (opts.bg_threshold > FREENECT_DEPTH_MM_MAX_VALUE)
              opts.bg_threshold = FREENECT_DEPTH_MM_MAX_VALUE;

            std::cerr << opts.bg_threshold << "." << std::endl;
          }
          break;

        case 'i':
          opts.use_bg_img = true;
          opts.bg_path = optarg;
          break;

        case 'b':
          opts.blur_bg = true;
          std::sscanf(optarg, "%d", &(opts.blur_int));
          if (opts.blur_int % 2 == 0) {
            std::cerr << "Blur intensity must be an odd number." << std::endl;
            exit(-2);
          }
          break;

        case 'w':
          opts.show_cv_window = true;
          break;

        case 'v':
          opts.verbose = true;
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
    std::cerr << "Wrong number of arguments." << std::endl;
    PrintHelp(args[0]);
    exit(-1);
  }

  const char* dev_video = args[optind];
  // TODO: understand why this number
  size_t vid_send_size = 640 * 720;

  if (opts.verbose) std::cout << "Opening v4l2loop device..." << std::endl;
  int v4l2lo_fd = OpenV4l2loop(dev_video, vid_send_size, WIDTH, HEIGHT);

  if (opts.verbose) std::cout << "Creating matrices..." << std::endl;
  cv::Mat depth_img(cv::Size(WIDTH, HEIGHT), CV_16UC1);
  cv::Mat rgb_img(cv::Size(WIDTH, HEIGHT), CV_8UC3);

  if (opts.verbose) std::cout << "Opening device..." << std::endl;
  Freenect::Freenect freenect;
  MyFreenectDevice& device = freenect.createDevice<MyFreenectDevice>(0);

  InitialSetup(opts, device);

  if (opts.verbose) std::cout << "Opening OpenCV window..." << std::endl;
  // Opens "feedback image" window
  if (opts.show_cv_window) {
    if (opts.verbose) std::cout << "Starting OpenCV window..." << std::endl;
    namedWindow("Freenect Green Screen", cv::WINDOW_AUTOSIZE);
  }

  if (opts.verbose) std::cout << "Creating locks..." << std::endl;
  // Locks required to signal main thread when a new frame is available
  std::unique_lock<std::mutex> rgb_lck(device.new_rgb_frame_mutex);
  std::unique_lock<std::mutex> dep_lck(device.new_dep_frame_mutex);

  if (opts.verbose) std::cout << "Creating frame matrix..." << std::endl;
  // TODO: I need to understand the need of this frame better. But currently is
  // a quick-fix of the displacement of the depth image to fit the rgb position
  cv::Mat frame(HEIGHT, WIDTH, CV_8UC1, 255);
  frame(cv::Rect(30, 40, 565, 440)) = 0;

  cv::Mat bg_img;
  cv::Mat mask(cv::Size(WIDTH, HEIGHT), CV_8UC1);
  cv::Mat mask_fg;
  cv::Mat mask_bg;
  cv::Mat final_fg;
  cv::Mat final_bg;
  cv::Mat final_img(cv::Size(WIDTH, HEIGHT), CV_8UC3);

  if (opts.use_bg_img) {
    if (opts.verbose)
      std::cout << "Setting background image matrix..." << std::endl;
    bg_img = cv::imread(opts.bg_path);
    // TODO: Make a better image fitting algorithm
    cv::resize(bg_img, bg_img, cv::Size(WIDTH, HEIGHT));
  }

  int erosion_size;
  cv::Mat element;
  if (opts.use_bg_img || opts.blur_bg) {
    if (opts.verbose) std::cout << "Setting erosion variables..." << std::endl;
    erosion_size = 10;
    element = cv::getStructuringElement(
        0, cv::Size(3 * erosion_size + 1, erosion_size + 1),
        cv::Point(erosion_size, erosion_size));
  }

  if (opts.verbose) std::cout << "Entering main loop..." << std::endl;

  while (true) {
    device.cond_var.wait(rgb_lck);
    device.getVideo(rgb_img);

    device.cond_var.wait(dep_lck);
    device.getDepth(depth_img);

    if (opts.use_bg_img || opts.blur_bg) {
      cv::GaussianBlur(depth_img, depth_img, cv::Size(21, 21), 0);

      // Set depth image to binary values: foreground and background
      cv::threshold(depth_img, depth_img, opts.bg_threshold,
                    FREENECT_DEPTH_MM_MAX_VALUE, cv::THRESH_BINARY);

      // Set to binary greyscale
      depth_img.convertTo(mask, CV_8UC1, 255.0 / FREENECT_DEPTH_MM_MAX_VALUE);

      // Fix curved edges
      mask = mask | frame;

      // Dilate to eliminate gaps in background and decrease the border around
      // the foreground object/person
      cv::dilate(mask, mask, element);

      cv::cvtColor(mask, mask, cv::COLOR_GRAY2BGR);

      // Generate foreground image
      mask_fg = mask.clone();
      mask_fg = ~mask_fg;
      final_fg = mask_fg & rgb_img;

      mask_bg = mask;

      if (opts.blur_bg && opts.use_bg_img) {
        cv::GaussianBlur(bg_img, bg_img, cv::Size(opts.blur_int, opts.blur_int),
                         0);
      } else if (opts.blur_bg && !opts.use_bg_img) {
        cv::GaussianBlur(rgb_img, rgb_img,
                         cv::Size(opts.blur_int, opts.blur_int), 0);
      }

      final_bg = (opts.use_bg_img ? bg_img & mask_bg : rgb_img & mask_bg);
      final_img = final_fg | final_bg;

    } else {
      final_img = rgb_img;
    }

    if (opts.show_cv_window) {
      cv::imshow("Freenect Green Screen", final_img);

      char k = cv::waitKey(5);
      if (k == 27) {
        cv::destroyAllWindows();
        break;
      }
    }

    cv::cvtColor(final_img, final_img, cv::COLOR_BGR2YUV_I420);
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
