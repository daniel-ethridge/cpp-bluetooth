#include <QBluetoothLocalDevice>
#include <QString>

#include <thread>
#include <chrono>
#include <iostream>

#include "device-finder.h"
#include "device-handler.h"

using namespace std::chrono_literals;

// #include <QBluetoothDiscoveryAgent>
int main()
{
   QBluetoothLocalDevice localDevice;
   QString localDeviceName;

   // Check if Bluetooth is available on this device. Abort if not.
   if (!localDevice.isValid()) 
   {
      std::cout << "No bluetooth device found. Aborting...\n";
      return 0;
   }
      
   // TODO: Power on (currently doesn't work. Low priority)
   // localDevice.powerOn();

   // Check if Bluetooth is powered on. Abort if not.
   int poweredOn = static_cast<int>(localDevice.hostMode());
   if (poweredOn == 0)  // Not powered on
   {
      std::cout << "Bluetooth is off. Please turn it on and try again. Aborting...\n";
      return 0;
   }

   // Read and print local device name
   localDeviceName = localDevice.name();
   std::cout << localDeviceName.toStdString() << std::endl;

   DeviceHandler deviceHandler;
   DeviceFinder deviceFinder(&deviceHandler);

   deviceFinder.startSearch();
   std::this_thread::sleep_for(500ms);
   deviceFinder.stopSearch();
   deviceFinder.process();

   while (true)
   {
      
   }

   return 0;
}