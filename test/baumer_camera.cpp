#include <baumer_camera/BaumerCamera.hpp>

#include <opencv2/opencv.hpp>

auto main(int argc, char** argv) -> int32_t
{
  BaumerCamera baumerCamera{};
  auto& availableCameras = baumerCamera.GetAvailableCameras();

  cv::Mat frame;
  auto index = 0;
  for (auto& availableCamera : availableCameras)
  {
    availableCamera.StartCamera();
    availableCamera.GetFrame(frame);
    cv::imshow(std::string("Camera #") + std::to_string(index), frame);
    cv::waitKey(1);
    availableCamera.StopCamera();
    ++index;
  }
  return 0;
}