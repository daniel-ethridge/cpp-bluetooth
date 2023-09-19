#ifndef _TINKER_DEVICEHANDLER_H_
#define _TINKER_DEVICEHANDLER_H_

#include <QObject>

class DeviceHandler : public QObject
{
public:
    DeviceHandler(QObject* parent = nullptr);
    ~DeviceHandler();
};

#endif  //_TINKER_DEVICEHANDLER_H_