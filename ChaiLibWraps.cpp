#include "ChaiLibWraps.h"

#include <QCanBusDevice>
#include <QSharedPointer>
#include <QHash>
#include <QSet>

#include "chai.h"

/***********************************************************************************************************************/
/***********************************************************************************************************************/

constexpr qint16 INVALID_CHANNEL { -10 };
constexpr qint16 INVALID_STATE   { -11 };

static bool             LIB_IS_OPEN     { false };
constexpr qsizetype     CAN_MSG_BUFF    { 1024 };

/******************************************************/

class Chai_Channel final
{
/************************************/
/************************************/

    quint8              _channelNumber;
    quint8              _flags;
    Chai_ChannelState   _state;

    QVector<canmsg_t>   _frameBuffer;
    qsizetype           _lastRecived;
    qint16              _lastError;

    chipstat_t          _channelStat;

/************************************/

    class ChannelStoper
    {
        /******************************************************/
        /******************************************************/

        Chai_Channel* _channel;

        /******************************************************/
    public:

        ChannelStoper(Chai_Channel* channel)
            : _channel      { channel }            
        {
            if (_channel)
                _channel->Stop();
        }

        ~ChannelStoper()
        {
            if (_channel)
                _channel->Run();
        }

        Q_DISABLE_COPY_MOVE(ChannelStoper);

        /******************************************************/
        /******************************************************/
    };

/************************************/
public:

    /************************************/

    Chai_Channel(quint8 number)
        : _channelNumber    { number }
        , _flags            { 0 }
        , _state            { Chai_ChannelState::UNINIT }
        , _lastRecived      { 0 }
        , _lastError        { 0 }
    {

    }

    ~Chai_Channel()
    {
        Stop();

        if (_state == Chai_ChannelState::INIT)
            CiClose(_channelNumber);
    }

    Q_DISABLE_COPY(Chai_Channel);

    /************************************/

    qint16 Run()
    {
        if (_state == Chai_ChannelState::INIT)
        {
            if (_lastError = CiStart(_channelNumber); _lastError == 0)
            {
                _state = Chai_ChannelState::RUNNING;

                _frameBuffer.resize(CAN_MSG_BUFF);
            }
        }
        else
        {
            _lastError = INVALID_STATE;
        }

        return _lastError;
    }

    qint16 Open(quint8 flags)
    {
        if (_state == Chai_ChannelState::UNINIT)
        {
            if (_lastError = CiOpen(_channelNumber, flags); _lastError == 0)
            {
                _flags = flags;
                _state = Chai_ChannelState::INIT;
            }
        }
        else
        {
            _lastError = INVALID_STATE;
        }

        return _lastError;
    }

    qint16 Stop()
    {
        if (_state != Chai_ChannelState::UNINIT)
        {
            if (_lastError = CiStop(_channelNumber); _lastError == 0)
                _state = Chai_ChannelState::INIT;

            _frameBuffer.clear();
        }
        else
        {
            _lastError = INVALID_STATE;
        }

        return _lastError;
    }

    /************************************/

    qint16 Write(const QCanBusFrame &frame)
    {
        if (_state == Chai_ChannelState::RUNNING)
        {
            canmsg_t    __legacyFrame;

            __legacyFrame.len = frame.payload().size();
            __legacyFrame.id  = frame.frameId();

            if (frame.hasExtendedFrameFormat())
                __legacyFrame.flags = MSG_FF;
            else
                __legacyFrame.flags = MSG_RTR;

            memcpy_s(__legacyFrame.data, __legacyFrame.len, frame.payload().constData(), __legacyFrame.len);

            return CiTransmit(_channelNumber, &__legacyFrame);
        }
        else
        {
            _lastError = INVALID_STATE;
        }

        return _lastError;
    }

    qint16 WaitFor(quint32 mSecs)
    {
        if (_state == Chai_ChannelState::RUNNING)
        {
            canwait_t __wait;
            __wait.chan     = _channelNumber;
            __wait.wflags   = CI_WAIT_RC | CI_WAIT_ER;

            qint32 __result { 0 };

            __result = CiWaitEvent(&__wait, 1, mSecs);

            if ( __result >= 0)
            {
                if (__wait.rflags & CI_WAIT_RC)
                {
                    if (__result = CiRead(_channelNumber, _frameBuffer.data(), _frameBuffer.size()); __result >= 0)
                        _lastRecived = __result;

                }
                else if (__wait.rflags & CI_WAIT_ER)
                {
                    // TODO

                    //ClearError();
                }
            }

            return __result;
        }
        else
        {
            _lastError = INVALID_STATE;
        }

        return _lastError;
    }

    qint16 Read(QVector<QCanBusFrame> &outputFrames, qsizetype framesToRead)
    {
        if (_state == Chai_ChannelState::RUNNING)
        {
            if (_lastRecived == 0)
                return 0;

            if (framesToRead > _lastRecived)
                framesToRead = _lastRecived;

            outputFrames.clear();
            outputFrames.reserve(framesToRead);

            QCanBusFrame            __frame;
            QByteArray              __payload;
            canmsg_t*               __legacy    { nullptr };

            for (auto i = 0; i < framesToRead; ++i)
            {
                __legacy = &_frameBuffer[i];

                __payload.resize(__legacy->len);
                memcpy_s(__payload.data(), __payload.size(), __legacy->data, __legacy->len);

                __frame.setFrameId              (__legacy->id);
                __frame.setPayload              (__payload);
                __frame.setExtendedFrameFormat  (msg_iseff(__legacy));

                outputFrames.append(__frame);

                --_lastRecived;
            }

            return framesToRead;
        }
        else
        {
            _lastError = INVALID_STATE;
        }

        return _lastError;
    }

    /************************************/

    qint16 SetBaudRate(Chai_BaudRate baudRate)
    {
        if (_state != Chai_ChannelState::UNINIT)
        {
            auto SetBRate = [=](quint8 channel, Chai_BaudRate bRate)
            {
                qint16 __err { 0 };

                switch (bRate)
                {
                    case CHAI_BR_10K:       __err = CiSetBaud(channel, BCI_10K);   break;

                    case CHAI_BR_20K:       __err = CiSetBaud(channel, BCI_20K);   break;

                    case CHAI_BR_50K:       __err = CiSetBaud(channel, BCI_50K);   break;

                    case CHAI_BR_100K:      __err = CiSetBaud(channel, BCI_100K);  break;

                    case CHAI_BR_125K:      __err = CiSetBaud(channel, BCI_125K);  break;

                    case CHAI_BR_250K:      __err = CiSetBaud(channel, BCI_250K);  break;

                    case CHAI_BR_500K:      __err = CiSetBaud(channel, BCI_500K);  break;

                    case CHAI_BR_800K:      __err = CiSetBaud(channel, BCI_800K);  break;

                    case CHAI_BR_1M:        __err = CiSetBaud(channel, BCI_1M);    break;

                    default:                __err = CiSetBaud(channel, BCI_1M);    break;
                }

                return __err;
            };

            if (_state == Chai_ChannelState::RUNNING)
            {
                ChannelStoper __stoper(this);

                _lastError = SetBRate(_channelNumber, baudRate);
            }
            else
            {
                _lastError = SetBRate(_channelNumber, baudRate);
            }
        }
        else
        {
            _lastError = INVALID_STATE;
        }

        return _lastError;
    }

    qint16 SetFilter(ulong id, ulong idMask)
    {
        if (_state != Chai_ChannelState::UNINIT)
        {
            auto SetChFilter = [=](quint8 channel, ulong id, ulong idMask)
            {
                return CiSetFilter(channel, id, idMask);
            };

            if (_state == Chai_ChannelState::RUNNING)
            {
                ChannelStoper __stoper(this);

                _lastError =  SetChFilter(_channelNumber, id, idMask);
            }
            else
            {
                _lastError =  SetChFilter(_channelNumber, id, idMask);
            }
        }
        else
        {
            _lastError = INVALID_STATE;
        }

        return _lastError;
    }

    bool HardwareIsOk()
    {
        return (CiChipStat(_channelNumber, &_channelStat) == 0);
    };

    qint16 HardwareReset()
    {
        if (_state != Chai_ChannelState::UNINIT)
        {
            _lastError = CiHwReset(_channelNumber);
        }
        else
        {
            _lastError = INVALID_STATE;
        }

        return _lastError;
    };

    /************************************/

    quint8              Number()    const { return _channelNumber;  }
    quint8              Flags()     const { return _flags;          }
    Chai_ChannelState   State()     const { return _state;          }
    qint16              LastError() const { return _lastError;      }

/************************************/
/************************************/
};

/******************************************************/

QHash<quint8, QSharedPointer<Chai_Channel>> CHANNELS;

/******************************************************/

bool Chai_OpenLib()
{
    if (!LIB_IS_OPEN)
        LIB_IS_OPEN = !CiInit();

    return LIB_IS_OPEN;
}

void Chai_CloseLib()
{
    if (LIB_IS_OPEN)
    {
        CHANNELS.clear();

        LIB_IS_OPEN = false;
    }
}

qint16 Chai_ActivateChannel(quint8 channel, bool activate)
{
    if (CHANNELS.contains(channel))
    {
        if (activate)
        {
            return CHANNELS[channel]->Run();
        }
        else
        {
            return CHANNELS[channel]->Stop();
        }
    }

    return INVALID_CHANNEL;
}

qint16 Chai_OpenChannel(quint8 channel, quint8 flags)
{
    if (CHANNELS.contains(channel))
    {
        auto __ch = CHANNELS[channel];

        if (__ch->State() == Chai_ChannelState::UNINIT)
            return __ch->Open(flags);
        else
            return 0;
    }
    else
    {
        auto __it = CHANNELS.insert(channel, QSharedPointer<Chai_Channel>(new Chai_Channel(channel)));

        if (auto __err = __it->get()->Open(flags); __err != 0)
        {
            CHANNELS.remove(channel);

            return __err;
        }
    }

    return 0;
}

qint16 Chai_SetBaudRate(quint8 channel, Chai_BaudRate bRate)
{    
    if (CHANNELS.contains(channel))
        return CHANNELS[channel]->SetBaudRate(bRate);

    return INVALID_CHANNEL;
}

qint16 Chai_SetFilter(quint8 channel, ulong id, ulong idMask)
{
    if (CHANNELS.contains(channel))
        return CHANNELS[channel]->SetFilter(id, idMask);

    return INVALID_CHANNEL;
}


QString Chai_InterpretError(qint32 errorCode)
{
    QString __error;

    switch(errorCode)
    {
        case INVALID_STATE:
        {
            __error = "Channel have invalid state.";
        }
        break;

        case INVALID_CHANNEL:
        {
            __error = "This channel isn't exists";
        }
        break;

        default:
        {
            auto __buffSize { std::numeric_limits<quint8>::max() };
            char __errBuff[__buffSize];

            CiStrError(errorCode, __errBuff, __buffSize);

            __error = QString::fromUtf8(__errBuff);
        }
        break;
    }

    return __error;
}

qint16  Chai_Reset(quint8 channel)
{
    if (CHANNELS.contains(channel))
        CHANNELS[channel]->HardwareReset();

    return INVALID_CHANNEL;
}

qint16 Chai_WaitFrames(quint8 channel, quint32 mSec)
{
    if (CHANNELS.contains(channel))
        return CHANNELS[channel]->WaitFor(mSec);

    return INVALID_CHANNEL;
}

quint16 Chai_ReadFrames(quint8 channel, QVector<QCanBusFrame> &outputFrames, qsizetype framesToRead)
{
    if (CHANNELS.contains(channel))
        CHANNELS[channel]->Read(outputFrames, framesToRead);

    return INVALID_CHANNEL;
}

bool Chai_IsHardwareConnected(quint8 channel)
{
    if (CHANNELS.contains(channel))
        return CHANNELS[channel]->HardwareIsOk();

    return false;
}

QList<Chai_DeviceInfo> Chai_GetDeviceList(QString *error)
{
    if (Chai_OpenLib())
    {
        QList<Chai_DeviceInfo> __deviceList;
        canboard_t             __info;

        for (auto i = 0u; i < CI_BRD_NUMS; ++i)
        {
            __info.brdnum = i;

            if (auto __err = CiBoardInfo(&__info); __err < 0)
            {
                if (error)
                {
                    error->append(Chai_InterpretError(__err));
                    error->append(';');
                }

                continue;
            }

            for (auto j = 0u; j < 4; ++j)
            {
                if (__info.chip[j] >= 0)
                    __deviceList.emplaceBack(__info.name, __info.manufact, __info.hwver, __info.brdnum, j);
            }
        }

        if (__deviceList.isEmpty()) // Not a single device is connected.
            Chai_CloseLib();   // The lib will be to need to be reopened to load device info once it's connected.

        return __deviceList;
    }

    return { };
}

qint16 Chai_WriteFrame(quint8 channel, const QCanBusFrame &frame)
{
    if (CHANNELS.contains(channel))
        return CHANNELS[channel]->Write(frame);

    return INVALID_CHANNEL;
}

Chai_ChannelState Chai_GetState(quint8 channel)
{
    if (CHANNELS.contains(channel))
        return CHANNELS[channel]->State();

    return Chai_ChannelState::UNINIT;
}

qint16 Chai_CloseChannel(quint8 channel)
{
    if (CHANNELS.contains(channel))
        CHANNELS.remove(channel);

    return 0;
}
