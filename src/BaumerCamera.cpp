#include <baumer_camera/BaumerCamera.hpp>

#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>

#include <bgapi2_genicam/bgapi2_genicam.hpp>

#include <iomanip>
#include <iostream>

namespace {

std::ostream& operator<<(std::ostream& out, BGAPI2::System* system)
{
  out << "ID: " << system->GetID() << '\n'
      << "\tVendor: " << system->GetVendor() << '\n'
      << "\tModel: " << system->GetModel() << '\n'
      << "\tVersion: " << system->GetVersion() << '\n'
      << "\tTLType: " << system->GetTLType() << '\n'
      << "\tFileName: " << system->GetFileName() << '\n'
      << "\tPathNeme: " << system->GetPathName() << '\n'
      << "\tDisplayName: " << system->GetDisplayName() << std::endl;
  return out;
}

std::ostream& operator<<(std::ostream& out, BGAPI2::SystemList* systemList)
{
  for (auto systemListIt = systemList->begin(); systemListIt != systemList->end(); ++systemListIt)
  {
    out << "SystemId: " << systemListIt->first << ":\n" << systemListIt->second;
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, BGAPI2::Interface* interface)
{
  out << "ID: " << interface->GetID() << '\n'
      << "\tTLType: " << interface->GetTLType() << '\n'
      << "\tDisplayName: " << interface->GetDisplayName() << std::endl;
  return out;
}

std::ostream& operator<<(std::ostream& out, BGAPI2::InterfaceList* interfaceList)
{
  for (auto interfaceListIt = interfaceList->begin(); interfaceListIt != interfaceList->end(); ++interfaceListIt)
  {
    out << "SystemId: " << interfaceListIt->first << ":\n" << interfaceListIt->second;
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, BGAPI2::Device* device)
{
  out << "ID: " << device->GetID() << '\n'
      << "\tVendor: " << device->GetVendor() << '\n'
      << "\tModel: " << device->GetModel() << '\n'
      << "\tSerialNumber: " << device->GetSerialNumber() << '\n'
      << "\tTLType: " << device->GetTLType() << '\n'
      << "\tDisplayName: " << device->GetDisplayName() << '\n'
      << "\tAccessStatus: " << device->GetAccessStatus() << std::endl;
  return out;
}

std::ostream& operator<<(std::ostream& out, BGAPI2::DeviceList* deviceList)
{
  for (auto deviceListIt = deviceList->begin(); deviceListIt != deviceList->end(); ++deviceListIt)
  {
    out << "SystemId: " << deviceListIt->first << ":\n" << deviceListIt->second;
  }
  return out;
}

class System {
public:

  class Interface {
  public:
    class Device {
    public:
      Device(BGAPI2::Device* device,
             std::function<bool(Device const& device)> leaveDeviceOpened = [](Device const&){ return true; })
        : id{device->GetID()}
        , vendor{device->GetVendor()}
        , model{device->GetModel()}
        , serialNumber{device->GetSerialNumber()}
        , tlType{device->GetTLType()}
        , displayName{device->GetDisplayName()}
        , accessStatus{device->GetAccessStatus()}
        , device{device}
      {
        try
        {
          device->Open();
          isOpened = true;
          std::cout << ">>> Device Opened <<<\n" << device << std::endl;
          if (!leaveDeviceOpened(*this))
          {
            device->Close();
            isOpened = false;
          }
        }
        catch (BGAPI2::Exceptions::ResourceInUseException& ex)
        {
          std::cout << " Device  " << device->GetID() << " already opened " << std::endl;
          std::cout << " ResourceInUseException: " << ex.GetErrorDescription() << std::endl;
        }
        catch (BGAPI2::Exceptions::AccessDeniedException& ex)
        {
          std::cout << " Device  " << device->GetID() << " already opened " << std::endl;
          std::cout << " AccessDeniedException " << ex.GetErrorDescription() << std::endl;
        }
      }
      std::string id;
      std::string vendor;
      std::string model;
      std::string serialNumber;
      std::string tlType;
      std::string displayName;
      std::string accessStatus;
      BGAPI2::Device* device;
      bool isOpened{};
    };

  public:
    Interface(BGAPI2::Interface* interface,
              std::function<bool(Interface const& interface)> leaveInterfaceOpened = [](Interface const&){ return true; },
              std::function<bool(Device const& device)> leaveDeviceOpened = [](Device const&){ return true; })
      : id{interface->GetID()}
      , displayName{interface->GetDisplayName()}
      , tlType{interface->GetTLType()}
      , devices{getDevices(interface, leaveInterfaceOpened, leaveDeviceOpened)}
      , interface{interface}
    {
    }

    auto getDevices(BGAPI2::Interface* interface,
                    std::function<bool(Interface const& interface)> leaveInterfaceOpened = [](Interface const&){ return true; },
                    std::function<bool(Device const& device)> leaveDeviceOpened = [](Device const&){ return true; }) -> std::vector<Device>
    {
      std::vector<Device> devices;
      try
      {
        interface->Open();
        std::cout << ">>> Interface Opened <<<\n" << interface << std::endl;
        auto deviceList = interface->GetDevices();
        deviceList->Refresh(100);
        for (auto deviceIt = deviceList->begin(); deviceIt != deviceList->end(); ++deviceIt)
        {
          devices.emplace_back(Device{deviceIt->second, leaveDeviceOpened});
        }
        if (!leaveInterfaceOpened(*this))
        {
          interface->Close();
        }
      }
      catch (BGAPI2::Exceptions::ResourceInUseException& ex)
      {
        std::cout << " Interface " << interface->GetID() << " already opened " << std::endl;
        std::cout << " ResourceInUseException: " << ex.GetErrorDescription() << std::endl;
      }
      return devices;
    }

    std::string id;
    std::string displayName;
    std::string tlType;
    std::vector<Device> devices;
    BGAPI2::Interface* interface;
  };

public:
  System(BGAPI2::System* system,
         std::function<bool(System const& system)> leaveSystemOpened = [](System const&){ return true; },
         std::function<bool(Interface const& interface)> leaveInterfaceOpened = [](Interface const&){ return true; },
         std::function<bool(Interface::Device const& device)> leaveDeviceOpened = [](Interface::Device const&){ return true; })
    : id{system->GetID()}
    , vendor{system->GetVendor()}
    , model{system->GetModel()}
    , version{system->GetVersion()}
    , tlType{system->GetTLType()}
    , fileName{system->GetFileName()}
    , pathName{system->GetPathName()}
    , displayName{system->GetDisplayName()}
    , interfaces{getInterfaces(system, leaveSystemOpened, leaveInterfaceOpened, leaveDeviceOpened)}
    , system{system}
  {
  }

  auto getInterfaces(BGAPI2::System* system,
                     std::function<bool(System const& system)> leaveSystemOpened = [](System const&){ return true; },
                     std::function<bool(Interface const& interface)> leaveInterfaceOpened = [](Interface const&){ return true; },
                     std::function<bool(Interface::Device const& device)> leaveDeviceOpened = [](Interface::Device const&){ return true; }) -> std::vector<Interface>
  {
    std::vector<Interface> interfaces;
    try
    {
      system->Open();
      std::cout << ">>> System Opened <<<\n" << system << std::endl;
      auto interfaceList = system->GetInterfaces();
      interfaceList->Refresh(100);
      for (auto interfaceIt = interfaceList->begin(); interfaceIt != interfaceList->end(); ++interfaceIt)
      {
        interfaces.emplace_back(Interface{interfaceIt->second});
      }
      if (!leaveSystemOpened(*this))
      {
        system->Close();
      }
    }
    catch (BGAPI2::Exceptions::ResourceInUseException& ex)
    {
      std::cout << " System " << system->GetID() << " already opened " << std::endl;
      std::cout << " ResourceInUseException: " << ex.GetErrorDescription() << std::endl;
    }
    return interfaces;
  }

  std::string id;
  std::string vendor;
  std::string model;
  std::string version;
  std::string tlType;
  std::string fileName;
  std::string pathName;
  std::string displayName;
  std::vector<Interface> interfaces;
  BGAPI2::System* system;
};

class SystemList
{
public:
  SystemList(std::function<bool(System const& system)> leaveSystemOpened = [](System const&){ return true; },
             std::function<bool(System::Interface const& interface)> leaveInterfaceOpened = [](System::Interface const&){ return true; },
             std::function<bool(System::Interface::Device const& device)> leaveDeviceOpened = [](System::Interface::Device const&){ return true; })
    try : systemList{BGAPI2::SystemList::GetInstance()}
    {
      systemList->Refresh();
      for (auto sytemIt = systemList->begin(); sytemIt != systemList->end(); ++sytemIt)
      {
          systems.emplace_back(System{sytemIt->second, leaveSystemOpened, leaveInterfaceOpened, leaveDeviceOpened});
      }
    }
    catch (BGAPI2::Exceptions::IException& ex)
    {
      std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
      std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
      std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
    }
    catch (BGAPI2::Exceptions::ResourceInUseException& ex)
    {
      std::cout << " ResourceInUseException: " << ex.GetErrorDescription() << std::endl;
    }

  auto GetAllAvailableDevices() -> std::vector<System::Interface::Device>
  {
    std::vector<System::Interface::Device> devices;
    for (auto const& system : systems)
    {
      for(auto const& interface : system.interfaces)
      {
        for(auto const& device : interface.devices)
        {
          devices.push_back(device);
        }
      }
    }
    return devices;
  }

  ~SystemList()
  {
    BGAPI2::SystemList::ReleaseInstance();
  }

  std::vector<System> systems;

private:
  BGAPI2::SystemList* systemList;
};

} /// end namespace anonymous

class BaumerCamera::Impl {
public:
  Impl()
  {
    auto systemList = SystemList();
    auto devices = systemList.GetAllAvailableDevices();
    std::cout << "All opened devices" << std::endl;
    for (auto& device : devices)
    {
      if (device.isOpened)
      {
        std::cout << device.device << std::endl;
      }
    }
#if 0
    BGAPI2::System * pSystem = NULL;
    BGAPI2::String sSystemID;

    BGAPI2::InterfaceList *interfaceList = NULL;
    BGAPI2::Interface * pInterface = NULL;
    BGAPI2::String sInterfaceID;

    BGAPI2::DeviceList *deviceList = NULL;
    BGAPI2::Device * pDevice = NULL;
    BGAPI2::String sDeviceID;

    BGAPI2::DataStreamList *datastreamList = NULL;
    //BGAPI2::DataStream * pDataStream = NULL;
    std::unique_ptr<BGAPI2::DataStream, void(*)(BGAPI2::DataStream*)> pDataStream{nullptr, [](BGAPI2::DataStream* _pDataStream) { _pDataStream->Close(); }};
    BGAPI2::String sDataStreamID;

    BGAPI2::BufferList *bufferList = NULL;
    BGAPI2::Buffer * pBuffer = NULL;
    BGAPI2::String sBufferID;
    int returncode = 0;

    try
    {
      std::cout << "SYSTEM LIST" << std::endl;
      std::cout << "###########" << std::endl << std::endl;

      std::unique_ptr<BGAPI2::SystemList, void(*)(BGAPI2::SystemList*)> systemList{SystemList::GetInstance(), [](BGAPI2::SystemList*) { BGAPI2::SystemList::ReleaseInstance(); }};
      systemList->Refresh();

      std::cout << systemList << std::endl;

      BGAPI2::String sSystemID;
      for (auto sysIterator = systemList->begin(); sysIterator != systemList->end(); sysIterator++)
      {
        try
        {
          sysIterator->second->Open();
          std::cout << ">>> Opened <<<\n" << sysIterator->second << std::endl;
          sSystemID = sysIterator->first;

          std::cout << "        Opened system - NodeList Information " << std::endl;
          std::cout << "          GenTL Version:   " << sysIterator->second->GetNode("GenTLVersionMajor")->GetValue() << "." << sysIterator->second->GetNode("GenTLVersionMinor")->GetValue() << std::endl << std::endl;

          std::cout << "INTERFACE LIST" << std::endl;
          std::cout << "##############" << std::endl << std::endl;

          auto interfaceList = sysIterator->second->GetInterfaces();
          interfaceList->Refresh(100); // timeout of 100 msec
          std::cout << "5.1.4   Detected interfaces: " << interfaceList->size() << std::endl;

          for (InterfaceList::iterator ifIterator = interfaceList->begin(); ifIterator != interfaceList->end(); ifIterator++)
          {
            std::cout << "  5.2.2   Interface ID:      " << ifIterator->first << std::endl;
            std::cout << "          Interface Type:    " << ifIterator->second->GetTLType() << std::endl;
            std::cout << "          Interface Name:    " << ifIterator->second->GetDisplayName() << std::endl << std::endl;
          }

          std::cout << "INTERFACE" << std::endl;
          std::cout << "#########" << std::endl << std::endl;

            for (InterfaceList::iterator ifIterator = interfaceList->begin(); ifIterator != interfaceList->end(); ifIterator++)
            {
              try
              {
                std::cout << "5.1.5   Open interface " << std::endl;
                std::cout << "  5.2.2   Interface ID:      " << ifIterator->first << std::endl;
                std::cout << "          Interface Type:    " << ifIterator->second->GetTLType() << std::endl;
                std::cout << "          Interface Name:    " << ifIterator->second->GetDisplayName() << std::endl;
                ifIterator->second->Open();
                //search for any camera is connetced to this interface
                deviceList = ifIterator->second->GetDevices();
                deviceList->Refresh(100);
                if(deviceList->size() == 0)
                {
                  std::cout << "5.1.13   Close interface (" << deviceList->size() << " cameras found) " << std::endl << std::endl;
                  ifIterator->second->Close();
                }
                else
                {
                  sInterfaceID = ifIterator->first;
                  std::cout << "   " << std::endl;
                  std::cout << "        Opened interface - NodeList Information " << std::endl;
                  if( ifIterator->second->GetTLType() == "GEV" )
                  {
                    std::cout << "          GevInterfaceSubnetIPAddress: " << ifIterator->second->GetNode("GevInterfaceSubnetIPAddress")->GetValue() << std::endl;
                    std::cout << "          GevInterfaceSubnetMask:      " << ifIterator->second->GetNode("GevInterfaceSubnetMask")->GetValue() << std::endl;
                  }
                  if( ifIterator->second->GetTLType() == "U3V" )
                  {
                    //std::cout << "          NodeListCount:     " << ifIterator->second->GetNodeList()->GetNodeCount() << std::endl;
                  }
                  std::cout << "  " << std::endl;
                  break;
                }
              }
              catch (BGAPI2::Exceptions::ResourceInUseException& ex)
              {
                returncode = (returncode == 0) ? 1 : returncode;
                std::cout << " Interface " << ifIterator->first << " already opened " << std::endl;
                std::cout << " ResourceInUseException: " << ex.GetErrorDescription() << std::endl;
              }
            }



          //if a camera is connected to the system interface then leave the system loop
          if(sInterfaceID != "")
          {
            break;
          }
        }
        catch (BGAPI2::Exceptions::ResourceInUseException& ex)
        {
          returncode = (returncode == 0) ? 1 : returncode;
          std::cout << " System " << sysIterator->first << " already opened " << std::endl;
          std::cout << " ResourceInUseException: " << ex.GetErrorDescription() << std::endl;
        }
      }

      if ( sSystemID == "" )
      {
        //BGAPI2::SystemList::ReleaseInstance();
        return;
      }
      else
      {
        pSystem = (*systemList)[sSystemID];
      }

      if (sInterfaceID == "")
      {
        std::cout << " No camera found " << sInterfaceID << std::endl;
        pSystem->Close();
        //BGAPI2::SystemList::ReleaseInstance();
        return;
      }
      else
      {
        pInterface = (*interfaceList)[sInterfaceID];
      }

      std::cout << "DEVICE LIST" << std::endl;
      std::cout << "###########" << std::endl << std::endl;

      //COUNTING AVAILABLE CAMERAS
      deviceList = pInterface->GetDevices();
      deviceList->Refresh(100);
      std::cout << "5.1.6   Detected devices:         " << deviceList->size() << std::endl;

      //DEVICE INFORMATION BEFORE OPENING
      for (DeviceList::iterator devIterator = deviceList->begin(); devIterator != deviceList->end(); devIterator++)
      {
        std::cout << "  5.2.3   Device DeviceID:        " << devIterator->first << std::endl;
        std::cout << "          Device Model:           " << devIterator->second->GetModel() << std::endl;
        std::cout << "          Device SerialNumber:    " << devIterator->second->GetSerialNumber() << std::endl;
        std::cout << "          Device Vendor:          " << devIterator->second->GetVendor() << std::endl;
        std::cout << "          Device TLType:          " << devIterator->second->GetTLType() << std::endl;
        std::cout << "          Device AccessStatus:    " << devIterator->second->GetAccessStatus() << std::endl;
        std::cout << "          Device UserID:          " << devIterator->second->GetDisplayName() << std::endl << std::endl;
      }

      std::cout << "DEVICE" << std::endl;
      std::cout << "######" << std::endl << std::endl;

      //OPEN THE FIRST CAMERA IN THE LIST
      for (DeviceList::iterator devIterator = deviceList->begin(); devIterator != deviceList->end(); devIterator++)
      {
        try
        {
          std::cout << "5.1.7   Open first device " << std::endl;
          std::cout << "          Device DeviceID:        " << devIterator->first << std::endl;
          std::cout << "          Device Model:           " << devIterator->second->GetModel() << std::endl;
          std::cout << "          Device SerialNumber:    " << devIterator->second->GetSerialNumber() << std::endl;
          std::cout << "          Device Vendor:          " << devIterator->second->GetVendor() << std::endl;
          std::cout << "          Device TLType:          " << devIterator->second->GetTLType() << std::endl;
          std::cout << "          Device AccessStatus:    " << devIterator->second->GetAccessStatus() << std::endl;
          std::cout << "          Device UserID:          " << devIterator->second->GetDisplayName() << std::endl << std::endl;
          devIterator->second->Open();
          sDeviceID = devIterator->first;
          std::cout << "        Opened device - RemoteNodeList Information " << std::endl;
          std::cout << "          Device AccessStatus:    " << devIterator->second->GetAccessStatus() << std::endl;

          //SERIAL NUMBER
          if(devIterator->second->GetRemoteNodeList()->GetNodePresent("DeviceSerialNumber"))
            std::cout << "          DeviceSerialNumber:     " << devIterator->second->GetRemoteNode("DeviceSerialNumber")->GetValue() << std::endl;
          else if(devIterator->second->GetRemoteNodeList()->GetNodePresent("DeviceID"))
            std::cout << "          DeviceID (SN):          " << devIterator->second->GetRemoteNode("DeviceID")->GetValue() << std::endl;
          else
            std::cout << "          SerialNumber:           Not Available "  << std::endl;

          //DISPLAY DEVICEMANUFACTURERINFO
          if(devIterator->second->GetRemoteNodeList()->GetNodePresent("DeviceManufacturerInfo"))
            std::cout << "          DeviceManufacturerInfo: " << devIterator->second->GetRemoteNode("DeviceManufacturerInfo")->GetValue() << std::endl;

          //DISPLAY DEVICEFIRMWAREVERSION OR DEVICEVERSION
          if(devIterator->second->GetRemoteNodeList()->GetNodePresent("DeviceFirmwareVersion"))
            std::cout << "          DeviceFirmwareVersion:  " << devIterator->second->GetRemoteNode("DeviceFirmwareVersion")->GetValue() << std::endl;
          else if(devIterator->second->GetRemoteNodeList()->GetNodePresent("DeviceVersion"))
            std::cout << "          DeviceVersion:          " << devIterator->second->GetRemoteNode("DeviceVersion")->GetValue() << std::endl;
          else
            std::cout << "          DeviceVersion:          Not Available "  << std::endl;

          if(devIterator->second->GetTLType() == "GEV")
          {
            std::cout << "          GevCCP:                 " << devIterator->second->GetRemoteNode("GevCCP")->GetValue() << std::endl;
            std::cout << "          GevCurrentIPAddress:    " << devIterator->second->GetRemoteNode("GevCurrentIPAddress")->GetValue() << std::endl;
            std::cout << "          GevCurrentSubnetMask:   " << devIterator->second->GetRemoteNode("GevCurrentSubnetMask")->GetValue() << std::endl;
          }
          std::cout << "  " << std::endl;
          break;
        }
        catch (BGAPI2::Exceptions::ResourceInUseException& ex)
        {
          returncode = (returncode == 0) ? 1 : returncode;
          std::cout << " Device  " << devIterator->first << " already opened " << std::endl;
          std::cout << " ResourceInUseException: " << ex.GetErrorDescription() << std::endl;
        }
        catch (BGAPI2::Exceptions::AccessDeniedException& ex)
        {
          returncode = (returncode == 0) ? 1 : returncode;
          std::cout << " Device  " << devIterator->first << " already opened " << std::endl;
          std::cout << " AccessDeniedException " << ex.GetErrorDescription() << std::endl;
        }
      }

      if (sDeviceID == "")
      {
        std::cout << " No Device found " << sDeviceID << std::endl;
        std::cout << std::endl << "End" << std::endl << "Input any number to close the program:";
        int endKey = 0;
        std::cin >> endKey;
        pInterface->Close();
        pSystem->Close();
        //BGAPI2::SystemList::ReleaseInstance();
        return;
      }
      else
      {
        pDevice = (*deviceList)[sDeviceID];
      }
#endif
#if 0
      std::cout << "DEVICE PARAMETER SETUP" << std::endl;
      std::cout << "######################" << std::endl << std::endl;
      //SET TRIGGER MODE OFF (FreeRun)
      pDevice->GetRemoteNode("TriggerMode")->SetString("Off");
      std::cout << "         TriggerMode:             " << pDevice->GetRemoteNode("TriggerMode")->GetValue() << std::endl;
      std::cout << std::endl;

      std::cout << "DATA STREAM LIST" << std::endl;
      std::cout << "################" << std::endl << std::endl;
      //COUNTING AVAILABLE DATASTREAMS
      datastreamList = pDevice->GetDataStreams();
      datastreamList->Refresh();
      std::cout << "5.1.8   Detected datastreams:     " << datastreamList->size() << std::endl;

      //DATASTREAM INFORMATION BEFORE OPENING
      for (DataStreamList::iterator dstIterator = datastreamList->begin(); dstIterator != datastreamList->end(); dstIterator++)
      {
        std::cout << "  5.2.4   DataStream ID:          " << dstIterator->first << std::endl << std::endl;
      }

      std::cout << "DATA STREAM" << std::endl;
      std::cout << "###########" << std::endl << std::endl;

      //OPEN THE FIRST DATASTREAM IN THE LIST
      for (DataStreamList::iterator dstIterator = datastreamList->begin(); dstIterator != datastreamList->end(); dstIterator++)
      {
        std::cout << "5.1.9   Open first datastream " << std::endl;
        std::cout << "          DataStream ID:          " << dstIterator->first << std::endl << std::endl;
        dstIterator->second->Open();
        sDataStreamID = dstIterator->first;
        std::cout << "        Opened datastream - NodeList Information " << std::endl;
        std::cout << "          StreamAnnounceBufferMinimum:  " << dstIterator->second->GetNode("StreamAnnounceBufferMinimum")->GetValue() << std::endl;
        if( dstIterator->second->GetTLType() == "GEV" )
        {
          std::cout << "          StreamDriverModel:            " << dstIterator->second->GetNode("StreamDriverModel")->GetValue() << std::endl;
        }
        std::cout << "  " << std::endl;
        break;
      }

      if (sDataStreamID == "")
      {
        std::cout << " No DataStream found " << sDataStreamID << std::endl;
        pDevice->Close();
        pInterface->Close();
        pSystem->Close();
        //BGAPI2::SystemList::ReleaseInstance();
        return;
      }
      else
      {
        pDataStream.reset((*datastreamList)[sDataStreamID]);
      }

      std::cout << "BUFFER LIST" << std::endl;
      std::cout << "###########" << std::endl << std::endl;

      //BufferList
      bufferList = pDataStream->GetBufferList();

      // 4 buffers using internal buffer mode
      for(int i=0; i<4; i++)
      {
        pBuffer = new BGAPI2::Buffer();
        bufferList->Add(pBuffer);
      }
      std::cout << "5.1.10   Announced buffers:       " << bufferList->GetAnnouncedCount() << " using " << pBuffer->GetMemSize() * bufferList->GetAnnouncedCount() << " [bytes]" << std::endl;

      for (BufferList::iterator bufIterator = bufferList->begin(); bufIterator != bufferList->end(); bufIterator++)
      {
        bufIterator->second->QueueBuffer();
      }
      std::cout << "5.1.11   Queued buffers:          " << bufferList->GetQueuedCount() << std::endl;

      std::cout << " " << std::endl;

      std::cout << "CAMERA START" << std::endl;
      std::cout << "############" << std::endl << std::endl;
      //START DataStream acquisition
      pDataStream->StartAcquisitionContinuous();
      std::cout << "5.1.12   DataStream started " << std::endl;

      //START CAMERA
      std::cout << "5.1.12   " << pDevice->GetModel() << " started " << std::endl;
      pDevice->GetRemoteNode("AcquisitionStart")->Execute();

      //CAPTURE 12 IMAGES
      std::cout << " " << std::endl;
      std::cout << "CAPTURE 12 IMAGES BY IMAGE POLLING" << std::endl;
      std::cout << "##################################" << std::endl << std::endl;
      BGAPI2::Buffer * pBufferFilled = NULL;
      for(int i = 0; i < 10000; i++)
      {
        pBufferFilled = pDataStream->GetFilledBuffer(1000); //timeout 1000 msec
        if(pBufferFilled == NULL)
        {
          std::cout << "Error: Buffer Timeout after 1000 msec" << std::endl;
        }
        else if(pBufferFilled->GetIsIncomplete() == true)
        {
          std::cout << "Error: Image is incomplete" << std::endl;
          // queue buffer again
          pBufferFilled->QueueBuffer();
        }
        else
        {
          std::cout << " Image " << std::setw(5) << pBufferFilled->GetFrameID() << " received in memory address " << std::hex << pBufferFilled->GetMemPtr() << std::dec << std::endl;
          cv::Mat const frame = cv::Mat((int)pBufferFilled->GetHeight(), (int)pBufferFilled->GetWidth(), CV_8UC1, (void*)pBufferFilled->GetMemPtr());
          cv::imshow("", frame);
          cv::waitKey(1);
          // queue buffer again
          pBufferFilled->QueueBuffer();
        }
      }

      std::cout << " " << std::endl;

      std::cout << "CAMERA STOP" << std::endl;
      std::cout << "###########" << std::endl << std::endl;
      //STOP CAMERA
      //SEARCH FOR 'AcquisitionAbort'
      if(pDevice->GetRemoteNodeList()->GetNodePresent("AcquisitionAbort"))
      {
        pDevice->GetRemoteNode("AcquisitionAbort")->Execute();
        std::cout << "5.1.12   " << pDevice->GetModel() << " aborted " << std::endl;
      }

      pDevice->GetRemoteNode("AcquisitionStop")->Execute();
      std::cout << "5.1.12   " << pDevice->GetModel() << " stopped " << std::endl;
      std::cout << std::endl;

      BGAPI2::String sExposureNodeName = "";
      if (pDevice->GetRemoteNodeList()->GetNodePresent("ExposureTime")) {
        sExposureNodeName = "ExposureTime";
      }
      else if (pDevice->GetRemoteNodeList()->GetNodePresent("ExposureTimeAbs")) {
        sExposureNodeName = "ExposureTimeAbs";
      }
      std::cout << "         ExposureTime:                   " << std::fixed << std::setprecision(0) << pDevice->GetRemoteNode(sExposureNodeName)->GetDouble() << " [" << pDevice->GetRemoteNode(sExposureNodeName)->GetUnit() << "]" << std::endl;
      if( pDevice->GetTLType() == "GEV" )
      {
        if(pDevice->GetRemoteNodeList()->GetNodePresent("DeviceStreamChannelPacketSize"))
          std::cout << "         DeviceStreamChannelPacketSize:  " << pDevice->GetRemoteNode("DeviceStreamChannelPacketSize")->GetInt() << " [bytes]" << std::endl;
        else
          std::cout << "         GevSCPSPacketSize:              " << pDevice->GetRemoteNode("GevSCPSPacketSize")->GetInt() << " [bytes]" << std::endl;
        std::cout << "         GevSCPD (PacketDelay):          " << pDevice->GetRemoteNode("GevSCPD")->GetInt() << " [tics]" << std::endl;
      }
      std::cout << std::endl;

      //STOP DataStream acquisition
      if( pDataStream->GetTLType() == "GEV" )
      {
        //DataStream Statistic
        std::cout << "         DataStream Statistics " << std::endl;
        std::cout << "           DataBlockComplete:              " << pDataStream->GetNodeList()->GetNode("DataBlockComplete")->GetInt() << std::endl;
        std::cout << "           DataBlockInComplete:            " << pDataStream->GetNodeList()->GetNode("DataBlockInComplete")->GetInt() << std::endl;
        std::cout << "           DataBlockMissing:               " << pDataStream->GetNodeList()->GetNode("DataBlockMissing")->GetInt() << std::endl;
        std::cout << "           PacketResendRequestSingle:      " << pDataStream->GetNodeList()->GetNode("PacketResendRequestSingle")->GetInt() << std::endl;
        std::cout << "           PacketResendRequestRange:       " << pDataStream->GetNodeList()->GetNode("PacketResendRequestRange")->GetInt() << std::endl;
        std::cout << "           PacketResendReceive:            " << pDataStream->GetNodeList()->GetNode("PacketResendReceive")->GetInt() << std::endl;
        std::cout << "           DataBlockDroppedBufferUnderrun: " << pDataStream->GetNodeList()->GetNode("DataBlockDroppedBufferUnderrun")->GetInt() << std::endl;
        std::cout << "           Bitrate:                        " << pDataStream->GetNodeList()->GetNode("Bitrate")->GetDouble() << std::endl;
        std::cout << "           Throughput:                     " << pDataStream->GetNodeList()->GetNode("Throughput")->GetDouble() << std::endl;
        std::cout << std::endl;
      }
      if( pDataStream->GetTLType() == "U3V" )
      {
        //DataStream Statistic
        std::cout << "         DataStream Statistics " << std::endl;
        std::cout << "           GoodFrames:            " << pDataStream->GetNodeList()->GetNode("GoodFrames")->GetInt() << std::endl;
        std::cout << "           CorruptedFrames:       " << pDataStream->GetNodeList()->GetNode("CorruptedFrames")->GetInt() << std::endl;
        std::cout << "           LostFrames:            " << pDataStream->GetNodeList()->GetNode("LostFrames")->GetInt() << std::endl;
        std::cout << std::endl;
      }

      //BufferList Information
      std::cout << "         BufferList Information " << std::endl;
      std::cout << "           DeliveredCount:        " << bufferList->GetDeliveredCount() << std::endl;
      std::cout << "           UnderrunCount:         " << bufferList->GetUnderrunCount() << std::endl;
      std::cout << std::endl;

      pDataStream->StopAcquisition();
      std::cout << "5.1.12   DataStream stopped " << std::endl;
      bufferList->DiscardAllBuffers();

      std::cout << std::endl;

      std::cout << "RELEASE" << std::endl;
      std::cout << "#######" << std::endl << std::endl;

      //Release buffers
      std::cout << "5.1.13   Releasing the resources " << std::endl;

      while( bufferList->size() > 0)
      {
        pBuffer = bufferList->begin()->second;
        bufferList->RevokeBuffer(pBuffer);
        delete pBuffer;
      }
      std::cout << "         buffers after revoke:    " << bufferList->size() << std::endl;

      pDataStream->Close();
      pDevice->Close();
      pInterface->Close();
      pSystem->Close();
      //BGAPI2::SystemList::ReleaseInstance();
    }
    catch (BGAPI2::Exceptions::IException& ex)
    {
      std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
      std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
      std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
    }
#endif
  }

  ~Impl()
  {
  }

private:
};

BaumerCamera::~BaumerCamera() = default;
BaumerCamera::BaumerCamera(BaumerCamera&&) noexcept = default;
BaumerCamera& BaumerCamera::operator=(BaumerCamera&&) noexcept = default;

BaumerCamera::BaumerCamera()
  : _pImpl{std::make_unique<Impl>()}
{
}
