#ifndef MY_FREENECT_DEVICE_HPP
#define MY_FREENECT_DEVICE_HPP
#include <condition_variable>
#include <libfreenect/libfreenect.hpp>
#include <mutex>
#include <opencv2/opencv.hpp>

#define WIDTH 640
#define HEIGHT 480

class MyFreenectDevice : public Freenect::FreenectDevice {
 public:
  MyFreenectDevice(freenect_context* _ctx, int _index);

  // Do not call them directly even in child
  void VideoCallback(void* _rgb, uint32_t timestamp);
  void DepthCallback(void* _depth, uint32_t timestamp);

  bool getVideo(cv::Mat& output);
  bool getDepth(cv::Mat& output);

  std::condition_variable cond_var;
  std::mutex new_rgb_frame_mutex;
  std::mutex new_dep_frame_mutex;

 private:
  std::vector<uint8_t> m_buffer_depth;
  std::vector<uint8_t> m_buffer_rgb;
  std::vector<uint16_t> m_gamma;

  cv::Mat depthMat;
  cv::Mat rgbMat;
  cv::Mat ownMat;

  std::mutex m_rgb_mutex;
  std::mutex m_depth_mutex;

  bool m_new_rgb_frame;
  bool m_new_depth_frame;
};

#endif
