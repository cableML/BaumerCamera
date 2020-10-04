#pragma once

#include <memory>

#include <opencv2/core/mat.hpp>

class BaumerCamera
{
private:
   class Impl;

public:
  class DataStream
  {
  private:
     class Impl;

  public:
    DataStream(std::unique_ptr<Impl>&& pImpl);
    ~DataStream();
    DataStream(DataStream&&) noexcept;
    DataStream& operator=(DataStream&&) noexcept;

    void StartCamera();
    void StopCamera();
    bool GetFrame(cv::Mat& frame);
    void SetExposureTime();

  private:
    std::unique_ptr<Impl> _pImpl;
    friend BaumerCamera::Impl;
  };

public:
  BaumerCamera();
  ~BaumerCamera();
  BaumerCamera(BaumerCamera&&) noexcept;
  BaumerCamera& operator=(BaumerCamera&&) noexcept;

  auto GetAvailableCameras() -> std::vector<DataStream>&;

private:
   std::unique_ptr<Impl> _pImpl;
};
