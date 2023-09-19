#include "device-finder.h"
#include <QLowEnergyController>
#include <thread>
#include <QtEndian>
#include <cassert>
#include <iostream>

using namespace std::chrono_literals;

DeviceFinder::DeviceFinder(DeviceHandler* handler, QObject* parent) :
    QObject(parent),
    m_deviceHandler(handler),
    m_device(),
    m_heartFound(false)
{
    m_discoveryAgent = std::make_unique<QBluetoothDeviceDiscoveryAgent>(this);
    m_discoveryAgent->setLowEnergyDiscoveryTimeout(15000);
    connect(m_discoveryAgent.get(), &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
        this, &DeviceFinder::deviceDiscovered);
}

DeviceFinder::~DeviceFinder() {}

void DeviceFinder::startSearch()
{
    m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void DeviceFinder::stopSearch()
{
    m_discoveryAgent->stop();
}

void DeviceFinder::process()
{
    if (!m_device)
    {
        std::cout << "No device!\n";
        return;
    }


    m_control = QLowEnergyController::createCentral(*m_device.get());

    connect(m_control, &QLowEnergyController::serviceDiscovered, 
            this,    &DeviceFinder::serviceDiscovered);
    connect(m_control, &QLowEnergyController::connected, this, [this]() {
        std::cout << "Connected! Discovering Services...\n";
        m_control->discoverServices();
    });
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &DeviceFinder::serviceScanDone);

    m_control->connectToDevice();
}

void DeviceFinder::deviceDiscovered(const QBluetoothDeviceInfo &info)
{
    if (info.name().toStdString() == "Polar H10 CA380328")
    {
        std::cout << info.name().toStdString() << " found!\n";
        m_device = std::make_unique<QBluetoothDeviceInfo>(info);
    }
}

void DeviceFinder::serviceDiscovered(const QBluetoothUuid& gatt)
{
    if (gatt == QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::HeartRate))
    {
        std::cout << "Heart rate capability found!\n";
        m_heartFound = true;
    }
}

void DeviceFinder::serviceScanDone()
{
    std::cout << "Service scan done!\n";
    if (m_service)
    {
        delete m_service;
        m_service = nullptr;
    }

    if (!m_heartFound)
    {
        return;
    }

    std::cout << "Creating service...\n";
    auto services = m_control->services();
    for (auto service : services)
    {
        if (service == QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::HeartRate))
        {
            std::cout << "Service found!\n";
            m_service = m_control->createServiceObject(service);
        }
    }

    if (m_service) 
    {
        std::cout << "Discovering details...\n";
        connect(m_service, &QLowEnergyService::characteristicChanged, this, &DeviceFinder::updateHeartRateValue);
        connect(m_service, &QLowEnergyService::stateChanged, this, &DeviceFinder::serviceStateChanged);
        connect(m_service, &QLowEnergyService::descriptorWritten, this, &DeviceFinder::confirmWritten);
        connect(m_service, &QLowEnergyService::errorOccurred, this, &DeviceFinder::errorOccurred);

        m_service->discoverDetails(QLowEnergyService::SkipValueDiscovery);
    }
}

void DeviceFinder::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    std::cout << "State change!\n";

    switch (s) 
    {
    case QLowEnergyService::RemoteServiceDiscovering:
        std::cout << "Discovering details...\n";
        break;
    case QLowEnergyService::RemoteServiceDiscovered:
    {
        std::cout << "Service discovered.\n";

        const QLowEnergyCharacteristic hrChar =
                m_service->characteristic(QBluetoothUuid(QBluetoothUuid::CharacteristicType::HeartRateMeasurement));
        if (!hrChar.isValid()) 
        {
            std::cout << "HR Data not found.\n";
            break;
        }

        m_notificationDesc = hrChar.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
        if (m_notificationDesc.isValid())
        {
            connect(m_service, &QLowEnergyService::descriptorRead, this, 
                [this](const QLowEnergyDescriptor &descriptor, const QByteArray &value) 
                { 
                    std::cout << "Successfully read!\n"; 
                    m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));
                });

            if (m_service->state() == QLowEnergyService::ServiceDiscovered)
            {
                std::cout << "Read Descriptor\n";
                m_service->readDescriptor(m_notificationDesc);
            }
        }
        break;
    }
    default:
        //nothing for now
        break;
    }
}


void DeviceFinder::updateHeartRateValue(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    std::cout << "Updating HR...\n";
    // ignore any other characteristic change -> shouldn't really happen though
    if (c.uuid() != QBluetoothUuid(QBluetoothUuid::CharacteristicType::HeartRateMeasurement))
        return;

    auto data = reinterpret_cast<const quint8 *>(value.constData());
    quint8 flags = *data;

    //Heart Rate
    int hrvalue = 0;
    if (flags & 0x1) // HR 16 bit? otherwise 8 bit
        hrvalue = static_cast<int>(qFromLittleEndian<quint16>(data[1]));
    else
        hrvalue = static_cast<int>(data[1]);

    std::cout << "Heart rate value: " << hrvalue << "\n";
}

void DeviceFinder::errorOccurred(QLowEnergyService::ServiceError newError)
{
    std::cout << "ERROR: " << static_cast<int>(newError) << "\n";
}

void DeviceFinder::confirmWritten(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    std::cout << "Confirm written called!\n";
    if (d.isValid() && d == m_notificationDesc && value == QByteArray::fromHex("0000")) 
    {
        //disabled notifications -> assume disconnect intent
        m_control->disconnectFromDevice();
        delete m_service;
        m_service = nullptr;
    }
}
