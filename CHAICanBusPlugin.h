#ifndef CHAICANBUSPLUGIN_H
#define CHAICANBUSPLUGIN_H

#include <QObject>
#include <QCanBus>
#include <QCanBusDevice>
#include <QCanBusFactory>

#include "CHAI_CAN.h"

class CHAICanBusPlugin : public QObject, public QCanBusFactory
{
/******************************************************************************************************************/
/******************************************************************************************************************/
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QCanBusFactory" FILE "CHAI_CAN.json")
    Q_INTERFACES(QCanBusFactory)
/******************************************************************************************************************/
public:
/******************************************************************************************************************/
    QList<QCanBusDeviceInfo> availableDevices(QString* errorMessage) const override
    {
        return CHAI_CAN::interfaces(errorMessage);
    };
/******************************************************************************************************************/
    QCanBusDevice* createDevice(const QString& interfaceName, QString* errorMessage) const override
    {
        Q_UNUSED(errorMessage);

        auto __temp = new CHAI_CAN(interfaceName);

        return __temp;
    };
/******************************************************************************************************************/
/******************************************************************************************************************/
};





#endif // CHAICANBUSPLUGIN_H
