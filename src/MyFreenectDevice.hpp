#include <libfreenect/libfreenect.hpp>
#include <mutex>
#include <opencv2/opencv.hpp>

class MyFreenectDevice : public Freenect::FreenectDevice {
 public:
  MyFreenectDevice(freenect_context* _ctx, int _index)
      : Freenect::FreenectDevice(_ctx, _index),
        m_buffer_depth(FREENECT_DEPTH_11BIT),
        m_buffer_rgb(FREENECT_VIDEO_RGB),
        m_gamma(2048),
        depthMat(cv::Size(640, 480), CV_16UC1),
        rgbMat(cv::Size(640, 480), CV_8UC3, cv::Scalar(0)),
        ownMat(cv::Size(640, 480), CV_8UC3, cv::Scalar(0)),
        m_new_rgb_frame(false),
        m_new_depth_frame(false) {
    for (unsigned int i = 0; i < 2048; i++) {
      float v = i / 2048.0;
      v = std::pow(v, 3) * 6;
      m_gamma[i] = v * 6 * 256;
    }
  }

  // Do not call them directly even in child
  void VideoCallback(void* _rgb, uint32_t timestamp);
  void DepthCallback(void* _depth, uint32_t timestamp);

  bool getVideo(cv::Mat& output);
  bool getDepth(cv::Mat& output);

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
