#include "CHAI_CAN.h"

#include "ChaiLibWraps.h"

#include <QVariant>
#include <QEvent>

QList<QCanBusDeviceInfo> CHAI_CAN::interfaces(QString *error)
{
    if (auto __devices = Chai_GetDeviceList(error); __devices.size())
    {
        QList<QCanBusDeviceInfo> __list;

        for (const auto& __device : qAsConst(__devices))
        {
            __list.append(createDeviceInfo("CHAI_CAN",
                                           QString{ "CHAI_CAN_%1_%2::%3" }
                                               .arg(__device.name)
                                               .arg(__device.boardNumber)
                                               .arg(__device.chip),

                                           QString::number(__device.boardNumber),

                                           QString { __device.manufactory }, "", __device.chip, false, false));

        }

        return __list;
    }

    return { };
}

/************************************************************/

CHAI_CAN::CHAI_CAN(const QString &interfaceName)
{
    bool __ok { false };

    if (auto __chanel = interfaceName.split("::").last().toUInt(&__ok); !__ok)
    {
        setError(QStringLiteral("Wrong devica name format"), QCanBusDevice::CanBusError::ConfigurationError);
    }
    else
    {
        if (Chai_OpenLib())
        {
            _channel = __chanel;
        }
        else
        {
            setError(QStringLiteral("CHAI lib can't be loading"), QCanBusDevice::CanBusError::ConfigurationError);
        }
    }        
}

CHAI_CAN::~CHAI_CAN()
{
    Chai_CloseLib();
}

bool CHAI_CAN::open()
{
    if (auto __err = Chai_OpenChannel(_channel, Chai_OpenFlags::ID_11 | Chai_OpenFlags::ID_29); __err)
    {
        setError(Chai_InterpretError(__err), QCanBusDevice::CanBusError::ConnectionError);
        setState(CanBusDeviceState::UnconnectedState);

        return false;
    }

    if (auto __err = Chai_SetBaudRate(_channel, static_cast<Chai_BaudRate>(configurationParameter(ConfigurationKey::BitRateKey).toUInt() / 1000u)); __err)
    {
        setError(Chai_InterpretError(__err), CanBusError::ConfigurationError);
        setState(CanBusDeviceState::UnconnectedState);

        return false;
    }

    QList<QCanBusDevice::Filter> __filters = qvariant_cast<QList<QCanBusDevice::Filter>>(configurationParameter(ConfigurationKey::RawFilterKey));
    for (auto& __filter : qAsConst(__filters))
    {
        if (auto __err = Chai_SetFilter(_channel, __filter.frameId, __filter.frameIdMask); __err)
        {
            setError(Chai_InterpretError(__err), CanBusError::ConfigurationError);
            setState(CanBusDeviceState::UnconnectedState);

            return false;
        }
    }

    if (auto __result = Chai_RunChannel(_channel); __result == 0)
    {
        EnableReadNotification  (true);
        EnableCheckNotification (true);

        setState(CanBusDeviceState::ConnectedState);

        return true;
    }
    else
    {
        setError(Chai_InterpretError(__result), CanBusError::ConfigurationError);
        setState(CanBusDeviceState::UnconnectedState);
    }

    return false;
}

void CHAI_CAN::close()
{        
    if (auto __result = Chai_CloseChannel(_channel); __result == 0u)
    {
        EnableWriteNotification (false);
        EnableReadNotification  (false);
        EnableCheckNotification (false);

        setState(CanBusDeviceState::UnconnectedState);
    }
    else
    {
        setError(Chai_InterpretError(__result), CanBusError::OperationError);
    }
}

bool CHAI_CAN::writeFrame(const QCanBusFrame &newData)
{
    if (state() != CanBusDeviceState::ConnectedState)
        return false;

    enqueueOutgoingFrame(newData);

    EnableWriteNotification(true);

    return true;
}

QString CHAI_CAN::interpretErrorFrame([[maybe_unused]]const QCanBusFrame &errorFrame)
{        
    return { "UNSUPPROTED_FUNCTIONAL" };
}

void CHAI_CAN::resetController()
{
    if (state() == ConnectedState)
    {
        if (auto __err = Chai_Reset(_channel); __err < 0)
            setError(Chai_InterpretError(__err), CanBusError::OperationError);
    }
}

bool CHAI_CAN::waitForFramesReceived(int msecs)
{
    if (auto __result = Chai_WaitForFramesRecived(_channel, msecs); __result > 0)
    {
        QVector<QCanBusFrame> __frames;

        Chai_ReadFrames(_channel, __frames, __result);

        enqueueReceivedFrames(__frames);
    }
    else if (__result < 0)
    {
        setError(Chai_InterpretError(__result), QCanBusDevice::ReadError);
        disconnectDevice();
    }

    return false;
}

bool CHAI_CAN::CheckHardware()
{
    if (Chai_IsHardwareConnected(_channel) == false)
    {
        disconnectDevice();

        setError("CAN connection lost. Check the device connection", QCanBusDevice::CanBusError::ConnectionError);        

        return false;
    }

    return true;
}

void CHAI_CAN::WriteFrames()
{
    if (!hasOutgoingFrames())
    {
        EnableWriteNotification(false);
        EnableCheckNotification(true);
        return;
    }

    quint32     __framesWritten { 0u };

    while(hasOutgoingFrames())
    {
        auto __frame = dequeueOutgoingFrame();

        if (auto __err = Chai_WriteFrame(_channel, __frame); __err)
        {
            setError(Chai_InterpretError(__err), QCanBusDevice::WriteError);

            disconnectDevice();
        }
        else
        {
            ++__framesWritten;
        }        
    }

    emit framesWritten(__framesWritten);

    if (hasOutgoingFrames())
    {
        EnableWriteNotification(true);
        EnableCheckNotification(false);
    }
}

/***************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************/

void CHAI_CAN::EnableWriteNotification(bool enabled, quint32 period)
{
    if (_outgoingTimer)
    {
        if (enabled)
        {
            if (_outgoingTimer->isActive() == false)
                _outgoingTimer->start(period);
        }
        else
        {
            _outgoingTimer->stop();
        }
    }
    else if (enabled)
    {
        _outgoingTimer = new OutgoingEventNotifyer { this };
        _outgoingTimer->start(period);
    }
}

void CHAI_CAN::EnableReadNotification(bool enabled, quint32 period)
{
    if (_incomingTimer)
    {
        if (enabled)
        {
            if (_incomingTimer->isActive() == false)
            {
                _incomingTimer->start(period);
            }
        }
        else
        {
            _incomingTimer->stop();
        }
    }
    else if (enabled)
    {
        _incomingTimer = new IncomingEventNotifyer { this };
        _incomingTimer->start(period);
    }
}

void CHAI_CAN::EnableCheckNotification(bool enabled, quint32 period)
{
    if (_checkTimer)
    {
        if (enabled)
        {
            if (_checkTimer->isActive() == false)
                _checkTimer->start(period);
        }
        else
        {
            _checkTimer->stop();
        }
    }
    else if (enabled)
    {
        _checkTimer = new CheckTimer { this };
        _checkTimer->start(period);
    }
}

/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

void OutgoingEventNotifyer::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == this->timerId())
    {
        _plugin->WriteFrames();
        return;
    }

    QTimer::timerEvent(e);
}

void IncomingEventNotifyer::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == this->timerId())
    {
        _plugin->waitForFramesReceived(0);

        return;
    }

    QTimer::timerEvent(e);
}

void CheckTimer::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == timerId())
    {
        _plugin->CheckHardware();

        return;
    }

    QTimer::timerEvent(e);
}

/***************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************/
