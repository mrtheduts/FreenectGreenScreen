#include "MyFreenectDevice.hpp"

#include <iostream>

void MyFreenectDevice::VideoCallback(void* _rgb, uint32_t /*timestamp*/) {
  std::cout << "RGB callback" << std::endl;
  m_rgb_mutex.lock();

  uint8_t* rgb = static_cast<uint8_t*>(_rgb);
  rgbMat.data = rgb;
  m_new_rgb_frame = true;

  m_rgb_mutex.unlock();
};

void MyFreenectDevice::DepthCallback(void* _depth, uint32_t /*timestamp*/) {
  std::cout << "Depth callback" << std::endl;
  m_depth_mutex.lock();

  uint16_t* depth = static_cast<uint16_t*>(_depth);
  depthMat.data = (unsigned char*)depth;
  m_new_depth_frame = true;

  m_depth_mutex.unlock();
}

bool MyFreenectDevice::getVideo(cv::Mat& output) {
  m_rgb_mutex.lock();

  if (m_new_rgb_frame) {
    cv::cvtColor(rgbMat, output, cv::COLOR_RGB2BGR);
    m_new_rgb_frame = false;
    m_rgb_mutex.unlock();
    return true;

  } else {
    m_rgb_mutex.unlock();
    return false;
  }
}

bool MyFreenectDevice::getDepth(cv::Mat& output) {
  m_depth_mutex.lock();

  if (m_new_depth_frame) {
    depthMat.copyTo(output);
    m_new_depth_frame = false;
    m_depth_mutex.unlock();
    return true;

  } else {
    m_depth_mutex.unlock();
    return false;
  }
}
