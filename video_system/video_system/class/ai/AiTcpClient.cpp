#include "AiTcpClient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

AiTcpClient *AiTcpClient::s_instance = nullptr;

AiTcpClient *AiTcpClient::instance()
{
    return s_instance;
}

void AiTcpClient::init(const QString &host, quint16 port)
{
    if (!s_instance) {
        s_instance = new AiTcpClient(host, port, qApp);
    }
}

AiTcpClient::AiTcpClient(const QString &host, quint16 port, QObject *parent)
    : QObject(parent)
    , m_host(host)
    , m_port(port)
    , m_socket(new QTcpSocket(this))
    , m_reconnectTimer(new QTimer(this))
{
    m_reconnectTimer->setInterval(5000);
    m_reconnectTimer->setSingleShot(false);

    connect(m_socket, &QTcpSocket::connected, this, &AiTcpClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &AiTcpClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &AiTcpClient::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &AiTcpClient::onError);
    connect(m_reconnectTimer, &QTimer::timeout, this, &AiTcpClient::tryReconnect);

    tryReconnect();
}

bool AiTcpClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void AiTcpClient::onConnected()
{
    m_reconnectTimer->stop();
    m_buffer.clear();
    emit connected();
}

void AiTcpClient::onDisconnected()
{
    m_reconnectTimer->start();
    emit disconnected();
}

void AiTcpClient::onError(QAbstractSocket::SocketError)
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        m_reconnectTimer->start();
    }
}

void AiTcpClient::tryReconnect()
{
    if (m_socket->state() == QAbstractSocket::UnconnectedState) {
        m_socket->connectToHost(m_host, m_port);
    }
}

void AiTcpClient::onReadyRead()
{
    m_buffer.append(m_socket->readAll());

    while (true) {
        int idx = m_buffer.indexOf('\n');
        if (idx < 0) break;

        QByteArray message = m_buffer.left(idx);
        m_buffer.remove(0, idx + 1);

        if (!message.isEmpty()) {
            processMessage(message);
        }
    }
}

void AiTcpClient::processMessage(const QByteArray &message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message);
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    QString msgType = obj["type"].toString();

    if (msgType == "detection") {
        DetectionResult result;
        result.cameraId = obj["camera_id"].toString();
        result.timestamp = obj["timestamp"].toVariant().toLongLong();

        QJsonArray dets = obj["detections"].toArray();
        for (const QJsonValue &val : dets) {
            QJsonObject detObj = val.toObject();
            Detection det;
            det.trackId   = detObj["track_id"].toInt();
            det.label     = detObj["label"].toString();
            det.confidence = (float)detObj["confidence"].toDouble();
            det.alarmType = detObj["alarm_type"].toString();

            QJsonArray bbox = detObj["bbox"].toArray();
            if (bbox.size() == 4) {
                det.bbox.x      = (float)bbox[0].toDouble();
                det.bbox.y      = (float)bbox[1].toDouble();
                det.bbox.width  = (float)bbox[2].toDouble();
                det.bbox.height = (float)bbox[3].toDouble();
            }
            result.detections.append(det);
        }
        emit detectionReceived(result);

    } else if (msgType == "alarm") {
        AlarmEvent alarm;
        alarm.id = obj["id"].toString();
        alarm.cameraId = obj["camera_id"].toString();
        alarm.cameraName = obj["camera_name"].toString();
        alarm.alarmType = obj["alarm_type"].toString();
        alarm.description = obj["description"].toString();
        alarm.timestamp = QDateTime::fromSecsSinceEpoch(obj["timestamp"].toVariant().toLongLong());

        if (obj.contains("track_id")) {
            alarm.trackId = obj["track_id"].toInt();
        }
        if (obj.contains("confidence")) {
            alarm.confidence = obj["confidence"].toDouble();
        }
        if (obj.contains("bbox")) {
            QJsonArray bbox = obj["bbox"].toArray();
            if (bbox.size() == 4) {
                alarm.bbox_x = (float)bbox[0].toDouble();
                alarm.bbox_y = (float)bbox[1].toDouble();
                alarm.bbox_w = (float)bbox[2].toDouble();
                alarm.bbox_h = (float)bbox[3].toDouble();
                alarm.hasBbox = true;
            }
        }
        if (obj.contains("screenshot_path")) {
            alarm.screenshotPath = obj["screenshot_path"].toString();
        }
        if (obj.contains("algorithm_id")) {
            alarm.algorithmId = obj["algorithm_id"].toInt();
        }
        if (obj.contains("voice_text")) {
            alarm.voiceText = obj["voice_text"].toString();
        }

        emit alarmReceived(alarm);

    }  else if (msgType == "camera_status") {
        QString camId = obj["camera_id"].toString();
        bool online   = obj["online"].toBool();
        double fps    = obj["fps"].toDouble();
        emit cameraStatusChanged(camId, online, fps);
    }
}
