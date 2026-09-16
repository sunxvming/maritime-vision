#include "ApiClient.h"
#include <QJsonDocument>
#include <QDebug>
#include <QJsonArray>
#include "app/appconfig.h"

// ─── Singleton ────────────────────────────────────────────────────────────────

ApiClient *ApiClient::s_instance = nullptr;

ApiClient *ApiClient::instance()
{
    if (!s_instance) {
        s_instance = new ApiClient(AppConfig::WebServerUrl);
    }
    return s_instance;
}

void ApiClient::init(const QString &baseUrl)
{
    if (!s_instance) {
        s_instance = new ApiClient(baseUrl);
    } else {
        s_instance->setBaseUrl(baseUrl);
    }
}

// ─── Constructor / Destructor ─────────────────────────────────────────────────

static QString joinUrl(const QString &base, const QString &path)
{
    if (path.startsWith("http://") || path.startsWith("https://")) return path;
    QString b = base, p = path;
    if (b.endsWith('/')) b.chop(1);
    if (p.startsWith('/')) p.remove(0, 1);
    if (b.isEmpty()) return p;
    return b + '/' + p;
}

ApiClient::ApiClient(const QString &baseUrl, QObject *parent)
    : QObject(parent)
    , m_httpClient(new AeaQt::HttpClient(this))
    , m_baseUrl(baseUrl)
{
}

ApiClient::~ApiClient() {}

AeaQt::HttpClient* ApiClient::httpClient() const { return m_httpClient; }
void ApiClient::setBaseUrl(const QString &baseUrl) { m_baseUrl = baseUrl; }
QString ApiClient::baseUrl() const { return m_baseUrl; }

void ApiClient::setToken(const QString &token)
{
    if (token.startsWith("Bearer ")) m_token = token;
    else if (token.isEmpty()) m_token.clear();
    else m_token = "Bearer " + token;
}

QString ApiClient::token() const { return m_token; }

// ─── Response handling ────────────────────────────────────────────────────────

void ApiClient::handleResponse(const QJsonObject &resp, SuccessCB onSuccess, FailedCB onFailed)
{
    // Server format: { code, message, data }
    int code = resp.value("code").toInt(-1);
    QString message = resp.value("message").toString();
    if (code != 0) {
        if (onFailed) onFailed(message);
        return;
    }
    if (resp.contains("data") && resp.value("data").isObject()) {
        if (onSuccess) onSuccess(resp.value("data").toObject());
        return;
    }
    if (onSuccess) onSuccess(resp);
}

// ─── Generic HTTP helpers ─────────────────────────────────────────────────────

void ApiClient::get(const QString &path, SuccessCB onSuccess, FailedCB onFailed)
{
    QString url = joinUrl(m_baseUrl, path);
    auto req = m_httpClient->get(url);
    if (!m_token.isEmpty()) req.header("Authorization", m_token);
    req.onSuccess([this, onSuccess, onFailed](const QJsonObject &resp) {
        this->handleResponse(resp, onSuccess, onFailed);
    }).onFailed([onFailed](const QString &err) {
        if (onFailed) onFailed(err);
    }).exec();
}

void ApiClient::post(const QString &path, const QJsonObject &body, SuccessCB onSuccess, FailedCB onFailed)
{
    QString url = joinUrl(m_baseUrl, path);
    auto req = m_httpClient->post(url).bodyWithJson(body);
    if (!m_token.isEmpty()) req.header("Authorization", m_token);
    req.onSuccess([this, onSuccess, onFailed](const QJsonObject &resp) {
        this->handleResponse(resp, onSuccess, onFailed);
    }).onFailed([onFailed](const QString &err) {
        if (onFailed) onFailed(err);
    }).exec();
}

void ApiClient::put(const QString &path, const QJsonObject &body, SuccessCB onSuccess, FailedCB onFailed)
{
    QString url = joinUrl(m_baseUrl, path);
    auto req = m_httpClient->put(url).bodyWithJson(body);
    if (!m_token.isEmpty()) req.header("Authorization", m_token);
    req.onSuccess([this, onSuccess, onFailed](const QJsonObject &resp) {
        this->handleResponse(resp, onSuccess, onFailed);
    }).onFailed([onFailed](const QString &err) {
        if (onFailed) onFailed(err);
    }).exec();
}

void ApiClient::del(const QString &path, SuccessCB onSuccess, FailedCB onFailed)
{
    QString url = joinUrl(m_baseUrl, path);
    auto req = m_httpClient->del(url);
    if (!m_token.isEmpty()) req.header("Authorization", m_token);
    req.onSuccess([this, onSuccess, onFailed](const QJsonObject &resp) {
        this->handleResponse(resp, onSuccess, onFailed);
    }).onFailed([onFailed](const QString &err) {
        if (onFailed) onFailed(err);
    }).exec();
}

// ─── IpcInfo CRUD ─────────────────────────────────────────────────────────────

void ApiClient::getAllIpc(IpcListCB onSuccess, FailedCB onFailed)
{
    // Response: { code:0, data: { list:[...], total:N } }
    get("/api/v1/ipc", [onSuccess](const QJsonObject &data) {
        if (onSuccess) onSuccess(data.value("list").toArray());
    }, onFailed);
}

void ApiClient::getIpc(int ipcId, SuccessCB onSuccess, FailedCB onFailed)
{
    get(QString("/api/v1/ipc/%1").arg(ipcId), onSuccess, onFailed);
}

void ApiClient::addIpc(const QJsonObject &data, SuccessCB onSuccess, FailedCB onFailed)
{
    post("/api/v1/ipc", data, onSuccess, onFailed);
}

void ApiClient::updateIpc(int ipcId, const QJsonObject &data, SuccessCB onSuccess, FailedCB onFailed)
{
    put(QString("/api/v1/ipc/%1").arg(ipcId), data, onSuccess, onFailed);
}

void ApiClient::deleteIpc(int ipcId, SuccessCB onSuccess, FailedCB onFailed)
{
    del(QString("/api/v1/ipc/%1").arg(ipcId), onSuccess, onFailed);
}

void ApiClient::updateIpcPosition(int ipcId, const QJsonObject &data, SuccessCB onSuccess, FailedCB onFailed)
{
    put(QString("/api/v1/ipc/%1/position").arg(ipcId), data, onSuccess, onFailed);
}

// ─── Other API calls ──────────────────────────────────────────────────────────

void ApiClient::filter_coefficients(double low, double high, double samprate, int order,
                                     SuccessCB onSuccess, FailedCB onFailed)
{
    QString url = joinUrl(m_baseUrl, "/phase/json/filter_coefficients");
    QJsonObject body;
    body.insert("low", low);
    body.insert("high", high);
    body.insert("fs", samprate);
    body.insert("order", order);
    m_httpClient->post(url).bodyWithJson(body)
        .onSuccess([onSuccess](const QJsonObject &resp) { if (onSuccess) onSuccess(resp); })
        .onFailed([onFailed](const QString &err) { if (onFailed) onFailed(err); })
        .exec();
}

// ─── Image loading ────────────────────────────────────────────────────────────

void ApiClient::loadImage(const QString &path, ImageCB onSuccess, FailedCB onFailed)
{
    // Build full HTTP URL if the path is relative
    QString fullUrl = joinUrl(m_baseUrl, path);

    m_httpClient->get(fullUrl)
        .onSuccess([onSuccess](const QByteArray &data) {
            if (onSuccess) onSuccess(data);
        })
        .onFailed([onFailed](const QString &err) {
            if (onFailed) onFailed(err);
        })
        .exec();
}
