#ifndef CHAI_CAN_H
#define CHAI_CAN_H

#include <QTimer>
#include <QCanBusDevice>
#include <QCanBusDeviceInfo>

class OutgoingEventNotifyer;
class IncomingEventNotifyer;
class CheckTimer;

QT_BEGIN_NAMESPACE

class CHAI_CAN : public QCanBusDevice
{    
/**************************************************************************************************************/
/**************************************************************************************************************/

    Q_OBJECT

/**************************************************************************************************************/

    quint8 _channel         { 0 };

/*********************************************************/

    QTimer* _outgoingTimer   { nullptr };
    QTimer* _incomingTimer   { nullptr };
    QTimer* _checkTimer      { nullptr };

/**************************************************************************************************************/
public:

    explicit CHAI_CAN(const QString& interfaceName);
    virtual ~CHAI_CAN();

/*********************************************************/

    virtual bool    writeFrame              (const QCanBusFrame& newData)       override final;
    virtual QString interpretErrorFrame     (const QCanBusFrame& errorFrame)    override final;
    virtual void    resetController         ()                                  override final;

/*********************************************************/

    virtual bool waitForFramesReceived      (int msecs)                         override final;

/*********************************************************/

    void            WriteFrames             ();
    bool            CheckHardware           ();

/*********************************************************/

    static QList<QCanBusDeviceInfo> interfaces(QString* error);

/**************************************************************************************************************/
protected:

    virtual bool open   () override final;
    virtual void close  () override final;

/**************************************************************************************************************/
private:

    void     EnableWriteNotification (bool enabled, quint32 period = 0u);
    void     EnableReadNotification  (bool enabled, quint32 period = 0u);

    // For periods when we can't to define the device state (in idle state)
    void     EnableCheckNotification (bool enabled, quint32 period = 100u);

/**************************************************************************************************************/
private slots:

/*********************************************************/

signals:

    void StartListening ();
    void StopListening  ();

/**************************************************************************************************************/
public slots:
/**************************************************************************************************************/
/**************************************************************************************************************/
};

QT_END_NAMESPACE

class OutgoingEventNotifyer : public QTimer
{
    /***********************************************************************/
    Q_OBJECT
    /***********************************************************************/
    CHAI_CAN* _plugin;
    /***********************************************************************/
public:
    OutgoingEventNotifyer(CHAI_CAN* plugin)
        : QTimer  { plugin }
        , _plugin { plugin }
    {

    }
protected:
    void timerEvent(QTimerEvent *e) override;
    /***********************************************************************/
};

class IncomingEventNotifyer : public QTimer
{
    /***********************************************************************/
    Q_OBJECT
    /***********************************************************************/
    CHAI_CAN* _plugin;
    /***********************************************************************/
public:
    IncomingEventNotifyer(CHAI_CAN* plugin)
        : QTimer    { plugin }
        , _plugin   { plugin }
    {

    }
protected:
    void timerEvent(QTimerEvent *e) override;
    /***********************************************************************/
};

class CheckTimer : public QTimer
{
    /***********************************************************************/
    Q_OBJECT
    /***********************************************************************/
    CHAI_CAN* _plugin;
    /***********************************************************************/
public:
    CheckTimer(CHAI_CAN* plugin)
        : QTimer    { plugin }
        , _plugin   { plugin }
    {

    }
protected:
    void timerEvent(QTimerEvent *e) override;
    /***********************************************************************/
};

#endif // CHAI_CAN_H
