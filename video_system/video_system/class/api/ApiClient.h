// ApiClient.h
#ifndef ApiClient_H
#define ApiClient_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>
#include "core_utils/HttpClient.h"

class ApiClient : public QObject
{
    Q_OBJECT
public:
    explicit ApiClient(const QString &baseUrl = QString(), QObject *parent = nullptr);
    ~ApiClient();

    // Singleton access — call init() once at startup
    static ApiClient *instance();
    static void init(const QString &baseUrl);

    AeaQt::HttpClient* httpClient() const;
    void setBaseUrl(const QString &baseUrl);
    QString baseUrl() const;
    void setToken(const QString &token);
    QString token() const;

    using SuccessCB  = std::function<void(const QJsonObject &data)>;
    using FailedCB   = std::function<void(const QString &error)>;
    using IpcListCB  = std::function<void(const QJsonArray &list)>;
    using ImageCB    = std::function<void(const QByteArray &imageData)>;

    // Generic HTTP helpers
    void get(const QString &path, SuccessCB onSuccess, FailedCB onFailed);
    void post(const QString &path, const QJsonObject &body, SuccessCB onSuccess, FailedCB onFailed);
    void put(const QString &path, const QJsonObject &body, SuccessCB onSuccess, FailedCB onFailed);
    void del(const QString &path, SuccessCB onSuccess, FailedCB onFailed);

    // IpcInfo CRUD  ─── path prefix: /api/v1/ipc
    void getAllIpc(IpcListCB onSuccess, FailedCB onFailed);
    void getIpc(int ipcId, SuccessCB onSuccess, FailedCB onFailed);
    void addIpc(const QJsonObject &data, SuccessCB onSuccess, FailedCB onFailed);
    void updateIpc(int ipcId, const QJsonObject &data, SuccessCB onSuccess, FailedCB onFailed);
    void deleteIpc(int ipcId, SuccessCB onSuccess, FailedCB onFailed);
    void updateIpcPosition(int ipcId, const QJsonObject &data, SuccessCB onSuccess, FailedCB onFailed);

    void filter_coefficients(double low, double high, double samprate, int order,
                             SuccessCB onSuccess, FailedCB onFailed);

    // Image loading
    void loadImage(const QString &path, ImageCB onSuccess, FailedCB onFailed);

private:
    void handleResponse(const QJsonObject &resp, SuccessCB onSuccess, FailedCB onFailed);

    AeaQt::HttpClient *m_httpClient;
    QString m_token;
    QString m_baseUrl;

    static ApiClient *s_instance;
};

#endif // ApiClient_H