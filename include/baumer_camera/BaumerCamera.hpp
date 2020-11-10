#pragma once

#include <memory>

#include <opencv2/core/mat.hpp>

namespace baumer {

class DataStream;

class BaumerCamera
{
public:
    BaumerCamera();
    ~BaumerCamera();
    BaumerCamera(BaumerCamera&&) noexcept;
    BaumerCamera& operator=(BaumerCamera&&) noexcept;

    auto GetAvailableCameras() -> std::vector<DataStream>&;

public:
    class Impl;
private:
    std::unique_ptr<Impl> _pImpl;
};

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
    bool GetFrame(cv::Mat& frame) const;
    void SetExposureTime(double exposureTime);
    auto GetExposureTime() const -> double;
    void SetGain(double gain);
    auto GetGain() const -> double;
    auto GetSerialNumber() const -> std::string;
    auto GetVendor() const -> std::string;

private:
    std::unique_ptr<Impl> _pImpl;
    friend BaumerCamera::Impl;
};

} /// ena namespace baumer