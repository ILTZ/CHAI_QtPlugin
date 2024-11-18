#ifndef CHAILIBWRAPS_H
#define CHAILIBWRAPS_H

#include <QCanBusDeviceInfo>
#include <QCanBusFrame>

enum Chai_OpenFlags
{
    ID_11 = 0x2,
    ID_29 = 0x4,
};

enum Chai_BaudRate
{
    CHAI_BR_10K     = 10,
    CHAI_BR_20K     = 20,
    CHAI_BR_50K     = 50,
    CHAI_BR_100K    = 100,
    CHAI_BR_125K    = 125,
    CHAI_BR_250K    = 250,
    CHAI_BR_500K    = 500,
    CHAI_BR_800K    = 800,
    CHAI_BR_1M      = 1000,
};

enum Chai_ChannelState
{
    UNINIT  = 0,
    INIT    = 1,
    RUNNING = 2,
};

struct Chai_DeviceInfo
{
    QString name;
    QString manufactory;

    quint32 hardwareVersion;
    quint8  boardNumber;
    qint16  chip;
};

bool    Chai_OpenLib                    ();

void    Chai_CloseLib                   ();

Chai_ChannelState Chai_GetState         (quint8 channel);

qint16  Chai_OpenChannel                (quint8 channel, quint8 flags);

qint16  Chai_CloseChannel               (quint8 channel);

qint16  Chai_ActivateChannel            (quint8 channel, bool activate);

qint16  Chai_SetBaudRate                (quint8 channel, Chai_BaudRate bRate);

qint16  Chai_SetFilter                  (quint8 channel, ulong id, ulong idMask);

qint16  Chai_Reset                      (quint8 channel);

qint16  Chai_WaitFrames                 (quint8 channel, quint32 mSec);

qint16  Chai_WriteFrame                 (quint8 channel, const QCanBusFrame& frame);

quint16 Chai_ReadFrames                 (quint8 channel, QVector<QCanBusFrame>& outputFrames, qsizetype framesToRead);

bool    Chai_IsHardwareConnected        (quint8 channel);

QList<Chai_DeviceInfo>        Chai_GetDeviceList(QString* error);

QString Chai_InterpretError             (qint32 errorCode);

#endif // CHAILIBWRAPS_H
