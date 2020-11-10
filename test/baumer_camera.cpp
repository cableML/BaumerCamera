#include <baumer_camera/BaumerCamera.hpp>

#include "src/TimeMeasuring.hpp"

#include <opencv2/opencv.hpp>

#ifdef _MSC_VER
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#include <sstream>

auto main(int argc, char** argv) -> int32_t
{
  auto exposition = std::stod(argv[1]);
  auto gain = std::stod(argv[2]);
  auto delay = std::stoi(argv[3]);
  baumer::BaumerCamera baumerCamera{};
  auto& availableCameras = baumerCamera.GetAvailableCameras();

  cv::Mat frame;
  auto i = 0;
  for (auto& availableCamera : availableCameras)
  {
      availableCamera.StartCamera();
      availableCamera.SetExposureTime(exposition);
      availableCamera.SetGain(gain);
      fs::create_directories("/media/user/Data/baumer_out/" + std::to_string(i++));
  }
  auto frameNumber = 0;
  while(true)
  {
    TAKEN_TIME();
    auto index = 0;
    i = 0;
    for (auto& availableCamera : availableCameras)
    {
        availableCamera.GetFrame(frame);
        cv::imshow(std::string("Camera #") + std::to_string(index++), frame);
        auto frameNumberStr = std::to_string(frameNumber);
        cv::imwrite("/media/user/Data/baumer_out/" + std::to_string(i++) + "/" + std::string(8 - frameNumberStr.length(), '0') + frameNumberStr + ".png", frame);
    }
    frameNumber++;
    if (cv::waitKey(delay) == 27)
    {
        break;
    }
  }
  for (auto& availableCamera : availableCameras)
  {
      availableCamera.StopCamera();
  }
  return 0;
}