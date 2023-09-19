#ifndef _TINKER_DEVICEFINDER_H_
#define _TINKER_DEVICEFINDER_H_

#include <QBluetoothDeviceDiscoveryAgent>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <memory>

#include "device-handler.h"
#include <QObject>

class DeviceFinder : public QObject
{
    public:
        DeviceFinder(DeviceHandler* handler, QObject* parent = nullptr);
        ~DeviceFinder();

        void startSearch();
        void stopSearch();

        void process();

    public slots:
        void deviceDiscovered(const QBluetoothDeviceInfo &info);
        void serviceDiscovered(const QBluetoothUuid& newService);
        void serviceScanDone();
        void updateHeartRateValue(const QLowEnergyCharacteristic &c, const QByteArray &value);
        void serviceStateChanged(QLowEnergyService::ServiceState s);
        void errorOccurred(QLowEnergyService::ServiceError newError);
        void confirmWritten(const QLowEnergyDescriptor &d, const QByteArray &value);

    private:
        std::unique_ptr<QBluetoothDeviceDiscoveryAgent> m_discoveryAgent;
        DeviceHandler* m_deviceHandler;
        std::unique_ptr<QBluetoothDeviceInfo> m_device;
        QLowEnergyController* m_control;
        QLowEnergyService* m_service;
        bool m_heartFound;
        QLowEnergyDescriptor m_notificationDesc;

};

#endif  // _TINKER_DEVICEFINDER_H_