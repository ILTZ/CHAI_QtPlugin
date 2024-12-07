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
    UNEXISTS    = 0,
    UNINIT      = 1,
    INIT        = 2,
    RUNNING     = 3,
};

enum Chai_ChannelMode
{
    INVALID     = 0,
    LISTEN_ONLY = 1,
    READ_WRITE  = 2,
};

struct Chai_DeviceInfo
{
    QString name;
    QString manufactory;

    quint32 hardwareVersion;
    quint8  boardNumber;
    qint16  chip;
};

struct Chai_CANErrors
{
    quint16 EWL;
    quint16 BOFF;
    quint16 HWOVR;
    quint16 SWOVR;
    quint16 WTOUT;
};

/*****************************************************************************************************/
/*****************************************************************************************************/

// Need to be open before start call any lib fncs (and reopen to get actual conencted device info)
// and after connecting CAN-usb.
bool Chai_OpenLib();

// "Close" lib and free using resources
void Chai_CloseLib();

/*****************************************************************************************************/
/*****************************************************************************************************/

// Get state of chousen channel. If channel isn't open -> return Chai_ChannelState::UNEXISTS
Chai_ChannelState Chai_GetState         (quint8 channel);

// Get mode of chousen channel. If channel isn't open -> return Chai_ChannelMode::INVALID
Chai_ChannelMode  Chai_GetMode          (quint8 channel);
qint16            Chai_SetMode          (quint8 channel, Chai_ChannelMode mode);

/*****************************************************************************************************/
/*****************************************************************************************************/

// Open channel (need to call before any interaction with channel)
qint16  Chai_OpenChannel                (quint8 channel, quint8 flags);
qint16  Chai_CloseChannel               (quint8 channel);

/*****************************************************************************************************/
/*****************************************************************************************************/

// Turn channel in "CAN_RUNNING" state (ready to transmit and recive frames).
qint16  Chai_RunChannel                 (quint8 channel);

// Turn channel in "CAN_INIT" state (ready to configurate device (set baud rate, filters, etc.)).
qint16  Chai_StopChannel                (quint8 channel);

/*****************************************************************************************************/
/*****************************************************************************************************/

qint16  Chai_SetBaudRate                (quint8 channel, Chai_BaudRate bRate);

qint16  Chai_SetFilter                  (quint8 channel, ulong id, ulong idMask);

/*****************************************************************************************************/
/*****************************************************************************************************/

// Hardware reset for chousing channel (filters and baud rate is saving)
qint16  Chai_Reset                      (quint8 channel);

// Clear hardware errors and return struct that contains error counts
Chai_CANErrors Chai_ClearErrors         (quint8 channel);

// Check connection between your device and CAN-usb addapter
bool    Chai_IsHardwareConnected        (quint8 channel);

/*****************************************************************************************************/
/*****************************************************************************************************/

// Stop thread in <mSec> msecs.
// return <amount of frames in recive queue in success> >= 0
qint16  Chai_WaitForFramesRecived       (quint8 channel, quint32 mSec);

// Call it after get amount of recived frames from Chai_WaitForFrameRecived(quint8 channel, quint32 mSecs) to read frames.
// Return amount of frames that was reading (may be less then framesToRead).
qint16  Chai_ReadFrames                 (quint8 channel, QVector<QCanBusFrame>& outputFrames, qsizetype framesToRead);

// Stop thread in <timerou> msecs. Return amount of frames that was append in <outputFrames> array.
qint16  Chai_WaitForFramesRecived       (quint8 channel, quint32 timeout, QVector<QCanBusFrame>& outputFrames);

qint16  Chai_WriteFrame                 (quint8 channel, const QCanBusFrame& frame);

/*****************************************************************************************************/
/*****************************************************************************************************/

// If CAN-usb was connected to your device after your call Chai_OpenLib() -> return empty list.
QList<Chai_DeviceInfo>  Chai_GetDeviceList      (QString* error);

QString                 Chai_InterpretError     (qint32 errorCode);

/*****************************************************************************************************/
/*****************************************************************************************************/

#endif // CHAILIBWRAPS_H
