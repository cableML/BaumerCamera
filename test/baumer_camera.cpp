#include <baumer_camera/BaumerCamera.hpp>

#include <opencv2/opencv.hpp>

auto main(int argc, char** argv) -> int32_t
{
  BaumerCamera baumerCamera{};
  auto& availableCameras = baumerCamera.GetAvailableCameras();

  cv::Mat frame;
  for (auto& availableCamera : availableCameras) {
      availableCamera.StartCamera();
      availableCamera.SetExposureTime(50000);
      availableCamera.SetGain(5);
  }
  while(true) {
    auto index = 0;
    for (auto& availableCamera : availableCameras) {
    //auto& availableCamera = availableCameras.front();
        availableCamera.GetFrame(frame);
        cv::imshow(std::string("Camera #") + std::to_string(index++), frame);
    }
    if (cv::waitKey(1) == 27) {
        break;
    }
  }
  for (auto& availableCamera : availableCameras) {
      availableCamera.StopCamera();
  }
  return 0;
}