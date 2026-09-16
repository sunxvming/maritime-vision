#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include "DetectionResult.h"

// Singleton TCP client that receives AI detection/alarm events from ai_service (port 9002).
// Call AiTcpClient::init(host, port) once at startup; use AiTcpClient::instance() thereafter.
class AiTcpClient : public QObject
{
    Q_OBJECT
public:
    static AiTcpClient *instance();
    static void init(const QString &host, quint16 port);

    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void detectionReceived(const DetectionResult &result);
    void alarmReceived(const AlarmEvent &alarm);
    void cameraStatusChanged(const QString &cameraId, bool online, double fps);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);
    void tryReconnect();

private:
    explicit AiTcpClient(const QString &host, quint16 port, QObject *parent = nullptr);
    void processMessage(const QByteArray &message);

    QString      m_host;
    quint16      m_port;
    QTcpSocket  *m_socket;
    QByteArray   m_buffer;
    QTimer      *m_reconnectTimer;

    static AiTcpClient *s_instance;
};
