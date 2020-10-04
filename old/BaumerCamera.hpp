#pragma once

#include <memory>

class BaumerCamera
{
public:
  BaumerCamera();
  ~BaumerCamera();
  BaumerCamera(BaumerCamera&&) noexcept;
  BaumerCamera& operator=(BaumerCamera&&) noexcept;

private:
  class Impl;
  std::unique_ptr<Impl> _pImpl;
};
