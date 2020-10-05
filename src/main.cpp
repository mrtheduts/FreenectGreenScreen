#include <cstdio>
#include <opencv2/opencv.hpp>
#include <string>

#include "MyFreenectDevice.hpp"

int main(int argc, const char** args) {
  bool die(false);

  if (argc < 2) {
    std::cerr << "Usage: " << args[0] << " /dev/videoN" << std::endl;
    std::cerr << "Where N is the v4l2loopback device to send video to"
              << std::endl;

    exit(1);
  }

  cv::Mat depthMat(cv::Size(640, 480), CV_16UC1);
  cv::Mat depthf(cv::Size(640, 480), CV_8UC1);
  cv::Mat rgbMat(cv::Size(640, 480), CV_8UC3, cv::Scalar(0));
  cv::Mat ownMat(cv::Size(640, 480), CV_8UC3, cv::Scalar(0));

  // The next two lines must be changed as Freenect::Freenect
  // isn't a template but the method createDevice:
  // Freenect::Freenect<MyFreenectDevice> freenect;
  // MyFreenectDevice& device = freenect.createDevice(0);
  // by these two lines:
  Freenect::Freenect freenect;
  MyFreenectDevice& device = freenect.createDevice<MyFreenectDevice>(0);

  device.setTiltDegrees(0);

  namedWindow("rgb", cv::WINDOW_AUTOSIZE);
  namedWindow("depth", cv::WINDOW_AUTOSIZE);

  device.startVideo();
  device.startDepth();

  while (!die) {
    device.getVideo(rgbMat);
    device.getDepth(depthMat);

    cv::imshow("rgb", rgbMat);
    depthMat.convertTo(depthf, CV_8UC1, 255.0 / 2048.0);
    cv::imshow("depth", depthf);

    char k = cv::waitKey(5);
    if (k == 27) {
      cv::destroyAllWindows();
      break;
    }
  }

  device.stopVideo();
  device.stopDepth();
  return 0;
}
