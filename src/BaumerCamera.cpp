#include <baumer_camera/BaumerCamera.hpp>

#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>

#include <bgapi2_genicam/bgapi2_genicam.hpp>

#include <map>

#include <iomanip>
#include <iostream>
#include <thread>

namespace {
#define BEGIN_SAFE_CALL() try \
                          {

#define END_SAFE_CALL() } \
                        catch(BGAPI2::Exceptions::IException& ex) \
                        { \
                           std::cout << "ExceptionType:    " << ex.GetType() \
                                     << "\nErrorDescription: " << ex.GetErrorDescription() \
                                     << "\nin function:      " << ex.GetFunctionName() << std::endl; \
                        }
} /// end namespace anonymous

using namespace baumer;

class BaumerCamera::Impl
{
public:


public:
   Impl()
      : _systemList{BGAPI2::SystemList::GetInstance()}
   {
   }

   ~Impl()
   {
      BGAPI2::SystemList::ReleaseInstance();
   }

   auto GetAvailableCameras() -> std::vector<DataStream>&
   {
      _systemList->Refresh();
      for (auto system : *_systemList)
      {
         BEGIN_SAFE_CALL()
            system->second->Open();
            auto interfaceList = system->second->GetInterfaces();
            interfaceList->Refresh(100);
            for (auto interface : *interfaceList)
            {
               BEGIN_SAFE_CALL()
                  interface->second->Open();
                  auto deviceList = interface->second->GetDevices();
                  deviceList->Refresh(100);
                  for (auto device : *deviceList)
                  {
                     BEGIN_SAFE_CALL()
                        device->second->Open();
                        auto dataStreamList = device->second->GetDataStreams();
                        dataStreamList->Refresh();
                        for (auto dataStream : *dataStreamList)
                        {
                           BEGIN_SAFE_CALL()
                              dataStream->second->Open();
                              _dataStreams.emplace_back(std::make_unique<DataStream::Impl>(system->second, interface->second, device->second, dataStream->second));
//                  device->second->Close();
                           END_SAFE_CALL()
                        }
//               device->second->Close();
                     END_SAFE_CALL()
                  }
//            interface->second->Close();
               END_SAFE_CALL()
            }
//         system->second->Close();
         END_SAFE_CALL()
      }
      return _dataStreams;
   }

private:
   BGAPI2::SystemList* _systemList;
   std::vector<DataStream> _dataStreams;
};

class DataStream::Impl
{
public:
   Impl(BGAPI2::System* system,
        BGAPI2::Interface* interface,
        BGAPI2::Device* device,
        BGAPI2::DataStream* dataStream)
      : _system{system}
      , _interface{interface}
      , _device{device}
      , _dataStream{dataStream}
      , _streamAnnounceBufferMinimum{_dataStream->GetNode("StreamAnnounceBufferMinimum")->GetValue()}
   {
   }

   void StartCamera()
   {
      BEGIN_SAFE_CALL()
         auto bufferList = _dataStream->GetBufferList();
         for(auto i = 0; i < 4; ++i)
         {
            bufferList->Add(new BGAPI2::Buffer());
         }
         for (auto buffer : *bufferList)
         {
            buffer->second->QueueBuffer();
         }
         _dataStream->StartAcquisitionContinuous();
         _device->GetRemoteNode("AcquisitionStart")->Execute();
         std::this_thread::sleep_for(std::chrono::seconds(1));
      END_SAFE_CALL()
   }

   void StopCamera()
   {
      BEGIN_SAFE_CALL()
         if(_device->GetRemoteNodeList()->GetNodePresent("AcquisitionAbort"))
         {
            _device->GetRemoteNode("AcquisitionAbort")->Execute();
            std::cout << "5.1.12   " << _device->GetModel() << " aborted " << std::endl;
         }
         _device->GetRemoteNode("AcquisitionStop")->Execute();
         if( _dataStream->GetTLType() == "GEV" )
         {
            std::cout << "         DataStream Statistics " << std::endl;
            std::cout << "           DataBlockComplete:              " << _dataStream->GetNodeList()->GetNode("DataBlockComplete")->GetInt() << std::endl;
            std::cout << "           DataBlockInComplete:            " << _dataStream->GetNodeList()->GetNode("DataBlockInComplete")->GetInt() << std::endl;
            std::cout << "           DataBlockMissing:               " << _dataStream->GetNodeList()->GetNode("DataBlockMissing")->GetInt() << std::endl;
            std::cout << "           PacketResendRequestSingle:      " << _dataStream->GetNodeList()->GetNode("PacketResendRequestSingle")->GetInt() << std::endl;
            std::cout << "           PacketResendRequestRange:       " << _dataStream->GetNodeList()->GetNode("PacketResendRequestRange")->GetInt() << std::endl;
            std::cout << "           PacketResendReceive:            " << _dataStream->GetNodeList()->GetNode("PacketResendReceive")->GetInt() << std::endl;
            std::cout << "           DataBlockDroppedBufferUnderrun: " << _dataStream->GetNodeList()->GetNode("DataBlockDroppedBufferUnderrun")->GetInt() << std::endl;
            std::cout << "           Bitrate:                        " << _dataStream->GetNodeList()->GetNode("Bitrate")->GetDouble() << std::endl;
            std::cout << "           Throughput:                     " << _dataStream->GetNodeList()->GetNode("Throughput")->GetDouble() << std::endl;
            std::cout << std::endl;
         }
         if( _dataStream->GetTLType() == "U3V" )
         {
            std::cout << "         DataStream Statistics " << std::endl;
            std::cout << "           GoodFrames:            " << _dataStream->GetNodeList()->GetNode("GoodFrames")->GetInt() << std::endl;
            std::cout << "           CorruptedFrames:       " << _dataStream->GetNodeList()->GetNode("CorruptedFrames")->GetInt() << std::endl;
            std::cout << "           LostFrames:            " << _dataStream->GetNodeList()->GetNode("LostFrames")->GetInt() << std::endl;
            std::cout << std::endl;
         }

         auto bufferList = _dataStream->GetBufferList();
         std::cout << "         BufferList Information " << std::endl;
         std::cout << "           DeliveredCount:        " << bufferList->GetDeliveredCount() << std::endl;
         std::cout << "           UnderrunCount:         " << bufferList->GetUnderrunCount() << std::endl;
         std::cout << std::endl;

         _dataStream->StopAcquisition();
         std::cout << "5.1.12   DataStream stopped " << std::endl;
         bufferList->DiscardAllBuffers();
      END_SAFE_CALL()
   }

   bool GetFrame(cv::Mat& frame)
   {
      BEGIN_SAFE_CALL()
         auto pBufferFilled = _dataStream->GetFilledBuffer(1000); //timeout 1000 msec
         if(!pBufferFilled)
         {
            std::cout << "Error: Buffer Timeout after 1000 msec" << std::endl;
            return false;
         }
         else if(pBufferFilled->GetIsIncomplete())
         {
            std::cout << "Error: Image is incomplete" << std::endl;
            pBufferFilled->QueueBuffer();
            return false;
         }
         else
         {
            std::cout << " Image " << std::setw(5) << pBufferFilled->GetFrameID() << " received in memory address " << std::hex << pBufferFilled->GetMemPtr() << std::dec << std::endl;
            frame = cv::Mat((int)pBufferFilled->GetHeight(), (int)pBufferFilled->GetWidth(), CV_8UC1, (void*)pBufferFilled->GetMemPtr()).clone();
            pBufferFilled->QueueBuffer();
            return true;
         }
      END_SAFE_CALL()
   }

   void SetExposureTime(double exposureTime)
   {
      BEGIN_SAFE_CALL()
         BGAPI2::String sExposureNodeName = "";
         if (_device->GetRemoteNodeList()->GetNodePresent("ExposureTime"))
         {
            sExposureNodeName = "ExposureTime";
         }
         else if (_device->GetRemoteNodeList()->GetNodePresent("ExposureTimeAbs"))
         {
            sExposureNodeName = "ExposureTimeAbs";
         }
         _device->GetRemoteNode(sExposureNodeName)->SetDouble(exposureTime);
      END_SAFE_CALL()
   }

   auto GetExposureTime() -> double
   {
       double exposureTimeValue{-1};
       BEGIN_SAFE_CALL()
           BGAPI2::String sExposureNodeName = "";
           if (_device->GetRemoteNodeList()->GetNodePresent("ExposureTime"))
           {
               sExposureNodeName = "ExposureTime";
           }
           else if (_device->GetRemoteNodeList()->GetNodePresent("ExposureTimeAbs"))
           {
               sExposureNodeName = "ExposureTimeAbs";
           }
           exposureTimeValue = _device->GetRemoteNode(sExposureNodeName)->GetDouble();
       END_SAFE_CALL()
       return exposureTimeValue;
   }

   void SetGain(double gain)
   {
       BEGIN_SAFE_CALL()
           if (_device->GetRemoteNodeList()->GetNodePresent("Gain"))
           {
               //auto gainValue = _device->GetRemoteNodeList()->GetNode("Gain")->GetDouble();
               //std::cout << "Gain: " << gainValue
               //          << " [" << _device->GetRemoteNode("Gain")->GetUnit() << "]" << std::endl;
               _device->GetRemoteNodeList()->GetNode("Gain")->SetDouble(gain);
           }
       END_SAFE_CALL()
   }

   auto GetGain() -> double
   {
       double gainValue{-1};
       BEGIN_SAFE_CALL()
           if (_device->GetRemoteNodeList()->GetNodePresent("Gain"))
           {
               gainValue = _device->GetRemoteNodeList()->GetNode("Gain")->GetDouble();
           }
       END_SAFE_CALL()
       return gainValue;
   }

   auto GetSerialNumber() const -> std::string
   {
       return std::string(_device->GetSerialNumber());
   }

   auto GetVendor() const -> std::string
   {
       return std::string(_device->GetVendor());
   }

   ~Impl()
   {
      BEGIN_SAFE_CALL()
         auto bufferList = _dataStream->GetBufferList();
         while (bufferList->size() > 0)
         {
            auto pBuffer = bufferList->begin()->second;
            bufferList->RevokeBuffer(pBuffer);
            delete pBuffer;
         }
         std::cout << "buffers after revoke:" << bufferList->size() << std::endl;
         _dataStream->Close();
         _device->Close();
         _interface->Close();
         _system->Close();
      END_SAFE_CALL()
   }

private:
   BGAPI2::System* _system;
   BGAPI2::Interface* _interface;
   BGAPI2::Device* _device;
   BGAPI2::DataStream* _dataStream;
   std::string _streamAnnounceBufferMinimum;
};

BaumerCamera::~BaumerCamera() = default;
BaumerCamera::BaumerCamera(BaumerCamera&&) noexcept = default;
BaumerCamera& BaumerCamera::operator=(BaumerCamera&&) noexcept = default;

BaumerCamera::BaumerCamera()
  : _pImpl{std::make_unique<Impl>()}
{
}

auto BaumerCamera::GetAvailableCameras() -> std::vector<DataStream>&
{
   return _pImpl->GetAvailableCameras();
}

DataStream::~DataStream() = default;
DataStream::DataStream(DataStream&&) noexcept = default;
DataStream& DataStream::operator=(DataStream&&) noexcept = default;

DataStream::DataStream(std::unique_ptr<Impl>&& pImpl)
   : _pImpl{std::move(pImpl)}
{
}

void DataStream::StartCamera()
{
   _pImpl->StartCamera();
}

void DataStream::StopCamera()
{
   _pImpl->StopCamera();
}

bool DataStream::GetFrame(cv::Mat& frame) const
{
   return _pImpl->GetFrame(frame);
}

void DataStream::SetExposureTime(double exposureTime)
{
   _pImpl->SetExposureTime(exposureTime);
}

auto DataStream::GetExposureTime() const -> double
{
    return _pImpl->GetExposureTime();
}

void DataStream::SetGain(double gain)
{
    _pImpl->SetGain(gain);
}

auto DataStream::GetGain() const -> double
{
   return _pImpl->GetGain();
}

auto DataStream::GetSerialNumber() const -> std::string
{
    return _pImpl->GetSerialNumber();
}

auto DataStream::GetVendor() const -> std::string
{
    return _pImpl->GetVendor();
}