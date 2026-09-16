#include "quihelper.h"
#include "quihelpercore.h"
#include "quihelperdata.h"
#include "quihelperfile.h"
#include "quihelpernet.h"
#include "quihelperform.h"
#include "quihelperimage.h"
#include "quihelperother.h"

int QUIHelper::getScreenIndex()
{
    return QUIHelperCore::getScreenIndex();
}

QRect QUIHelper::getScreenRect(bool available)
{
    return QUIHelperCore::getScreenRect(available);
}

QRect QUIHelper::checkCenterRect(QRect &rect, bool available)
{
    return QUIHelperCore::checkCenterRect(rect, available);
}

int QUIHelper::deskWidth()
{
    return QUIHelperCore::deskWidth();
}

int QUIHelper::deskHeight()
{
    return QUIHelperCore::deskHeight();
}

QSize QUIHelper::deskSize()
{
    return QUIHelperCore::deskSize();
}

QWidget *QUIHelper::centerBaseForm = 0;
void QUIHelper::setFormInCenter(QWidget *form)
{
    QUIHelperCore::setFormInCenter(form, centerBaseForm);
}

void QUIHelper::showForm(QWidget *form)
{
    QUIHelperCore::showForm(form);
}

QString QUIHelper::appName()
{
    return QUIHelperCore::appName();
}

QString QUIHelper::appPath()
{
    return QUIHelperCore::appPath();
}

QString QUIHelper::getCompilerString()
{
    return QUIHelperCore::getCompilerString();
}

QString QUIHelper::getUuid()
{
    return QUIHelperCore::getUuid();
}

void QUIHelper::checkPath(const QString &dirName)
{
    QUIHelperCore::checkPath(dirName);
}

void QUIHelper::sleep(int msec)
{
    QUIHelperCore::sleep(msec);
}

void QUIHelper::checkRun()
{
    QUIHelperCore::checkRun();
}

void QUIHelper::setCode(bool utf8)
{
    QUIHelperCore::setCode(utf8);
}

QFont QUIHelper::addFont(const QString &fontFile, const QString &fontName)
{
    return QUIHelperCore::addFont(fontFile, fontName);
}

void QUIHelper::setFont(const QString &fontFile, const QString &fontName, int fontSize)
{
    QUIHelperCore::setFont(fontFile, fontName, fontSize);
}

void QUIHelper::setTranslator()
{
    QUIHelperCore::setTranslator();
}

void QUIHelper::setTranslator(const QString &qmFile)
{
    QUIHelperCore::setTranslator(qmFile);
}

bool QUIHelper::checkPermission(const QString &permission)
{
    return QUIHelperCore::checkPermission(permission);
}

void QUIHelper::initAndroidPermission()
{
    QUIHelperCore::initAndroidPermission();
}

void QUIHelper::initAll(bool utf8)
{
    QUIHelperCore::initAll(utf8);
}

void QUIHelper::initMain(bool desktopSettingsAware, bool useOpenGLES)
{
    QUIHelperCore::initMain(desktopSettingsAware, useOpenGLES);
}

int QUIHelper::strHexToDecimal(const QString &strHex)
{
    return QUIHelperData::strHexToDecimal(strHex);
}

int QUIHelper::strDecimalToDecimal(const QString &strDecimal)
{
    return QUIHelperData::strDecimalToDecimal(strDecimal);
}

int QUIHelper::strBinToDecimal(const QString &strBin)
{
    return QUIHelperData::strBinToDecimal(strBin);
}

QString QUIHelper::strHexToStrBin(const QString &strHex)
{
    return QUIHelperData::strHexToStrBin(strHex);
}

QString QUIHelper::decimalToStrBin1(int decimal)
{
    return QUIHelperData::decimalToStrBin1(decimal);
}

QString QUIHelper::decimalToStrBin2(int decimal)
{
    return QUIHelperData::decimalToStrBin2(decimal);
}

QString QUIHelper::decimalToStrHex(int decimal)
{
    return QUIHelperData::decimalToStrHex(decimal);
}

QByteArray QUIHelper::intToByte(int data)
{
    return QUIHelperData::intToByte(data);
}

QByteArray QUIHelper::intToByteRec(int data)
{
    return QUIHelperData::intToByteRec(data);
}

int QUIHelper::byteToInt(const QByteArray &data)
{
    return QUIHelperData::byteToInt(data);
}

int QUIHelper::byteToIntRec(const QByteArray &data)
{
    return QUIHelperData::byteToIntRec(data);
}

quint32 QUIHelper::byteToUInt(const QByteArray &data)
{
    return QUIHelperData::byteToUInt(data);
}

quint32 QUIHelper::byteToUIntRec(const QByteArray &data)
{
    return QUIHelperData::byteToUIntRec(data);
}

QByteArray QUIHelper::ushortToByte(ushort data)
{
    return QUIHelperData::ushortToByte(data);
}

QByteArray QUIHelper::ushortToByteRec(ushort data)
{
    return QUIHelperData::ushortToByteRec(data);
}

int QUIHelper::byteToUShort(const QByteArray &data)
{
    return QUIHelperData::byteToUShort(data);
}

int QUIHelper::byteToUShortRec(const QByteArray &data)
{
    return QUIHelperData::byteToUShortRec(data);
}

QString QUIHelper::getValue(quint8 value)
{
    return QUIHelperData::getValue(value);
}

QString QUIHelper::trimmed(const QString &text, int type)
{
    return QUIHelperData::trimmed(text, type);
}

QString QUIHelper::getXorEncryptDecrypt(const QString &value, char key)
{
    return QUIHelperData::getXorEncryptDecrypt(value, key);
}

quint8 QUIHelper::getOrCode(const QByteArray &data)
{
    return QUIHelperData::getOrCode(data);
}

quint8 QUIHelper::getCheckCode(const QByteArray &data)
{
    return QUIHelperData::getCheckCode(data);
}

void QUIHelper::getFullData(QByteArray &buffer)
{
    QUIHelperData::getFullData(buffer);
}

QByteArray QUIHelper::getCrcCode(const QByteArray &data)
{
    return QUIHelperData::getCrcCode(data);
}

QString QUIHelper::byteArrayToAsciiStr(const QByteArray &data)
{
    return QUIHelperData::byteArrayToAsciiStr(data);
}

QByteArray QUIHelper::asciiStrToByteArray(const QString &data)
{
    return QUIHelperData::asciiStrToByteArray(data);
}

QByteArray QUIHelper::hexStrToByteArray(const QString &data)
{
    return QUIHelperData::hexStrToByteArray(data);
}

QString QUIHelper::byteArrayToHexStr(const QByteArray &data)
{
    return QUIHelperData::byteArrayToHexStr(data);
}

QString QUIHelper::getOpenFileName(const QString &filter, const QString &dirName, const QString &fileName, bool native, int width, int height)
{
    return QUIHelperFile::getOpenFileName(filter, dirName, fileName, native, width, height);
}

QString QUIHelper::getSaveFileName(const QString &filter, const QString &dirName, const QString &fileName, bool native, int width, int height)
{
    return QUIHelperFile::getSaveFileName(filter, dirName, fileName, native, width, height);
}

QString QUIHelper::getExistingDirectory(const QString &dirName, bool native, int width, int height)
{
    return QUIHelperFile::getExistingDirectory(dirName, native, width, height);
}

QString QUIHelper::getFileNameWithExtension(const QString &fileName)
{
    return QUIHelperFile::getFileNameWithExtension(fileName);
}

QStringList QUIHelper::getFolderFileNames(const QStringList &filter)
{
    return QUIHelperFile::getFolderFileNames(filter);
}

bool QUIHelper::copyFile(const QString &sourceFile, const QString &targetFile)
{
    return QUIHelperFile::copyFile(sourceFile, targetFile);
}

void QUIHelper::deleteDirectory(const QString &path)
{
    QUIHelperFile::deleteDirectory(path);
}

QString QUIHelper::getIP(const QString &url)
{
    return QUIHelperNet::getIP(url);
}

bool QUIHelper::isIP(const QString &ip)
{
    return QUIHelperNet::isIP(ip);
}

bool QUIHelper::isMac(const QString &mac)
{
    return QUIHelperNet::isMac(mac);
}

bool QUIHelper::isTel(const QString &tel)
{
    return QUIHelperNet::isTel(tel);
}

bool QUIHelper::isEmail(const QString &email)
{
    return QUIHelperNet::isEmail(email);
}

QString QUIHelper::ipv4IntToString(quint32 ip)
{
    return QUIHelperNet::ipv4IntToString(ip);
}

quint32 QUIHelper::ipv4StringToInt(const QString &ip)
{
    return QUIHelperNet::ipv4StringToInt(ip);
}

bool QUIHelper::ipLive(const QString &ip, int port, int timeout)
{
    return QUIHelperNet::ipLive(ip, port, timeout);
}

bool QUIHelper::download(const QString &url, const QString &fileName, int timeout)
{
    return QUIHelperNet::download(url, fileName, timeout);
}

QByteArray QUIHelper::getHtml(const QString &url, int timeout)
{
    return QUIHelperNet::getHtml(url, timeout);
}

QString QUIHelper::getNetIP(const QString &html)
{
    return QUIHelperNet::getNetIP(html);
}

QString QUIHelper::getLocalIP()
{
    return QUIHelperNet::getLocalIP();
}

QStringList QUIHelper::getLocalIPs()
{
    return QUIHelperNet::getLocalIPs();
}

QString QUIHelper::urlToIP(const QString &url)
{
    return QUIHelperNet::urlToIP(url);
}

void QUIHelper::setFormShadow(QWidget *widget, QLayout *layout, const QString &color, int margin, int radius)
{
    QUIHelperForm::setFormShadow(widget, layout, color, margin, radius);
}

void QUIHelper::setFormShadow(const QString &color)
{
    QUIHelperForm::setFormShadow(color);
}

void QUIHelper::setFramelessForm(QWidget *widgetMain, bool tool, bool top, bool menu, bool x11)
{
    QUIHelperForm::setFramelessForm(widgetMain, tool, top, menu, x11);
}

void QUIHelper::setFramelessForm(QWidget *widgetMain, QWidget *widgetTitle,
                                 QLabel *labIco, QPushButton *btnClose,
                                 bool tool, bool top, bool menu, bool x11)
{
    QUIHelperForm::setFramelessForm(widgetMain, widgetTitle, labIco, btnClose, tool, top, menu, x11);
}

int QUIHelper::showMessageBox(const QString &info, int type, int timeout, bool exec)
{
    return QUIHelperForm::showMessageBox(info, type, timeout, exec);
}

void QUIHelper::showMessageBoxInfo(const QString &info, int timeout, bool exec)
{
    QUIHelperForm::showMessageBoxInfo(info, timeout, exec);
}

void QUIHelper::showMessageBoxError(const QString &info, int timeout, bool exec)
{
    QUIHelperForm::showMessageBoxError(info, timeout, exec);
}

int QUIHelper::showMessageBoxQuestion(const QString &info)
{
    return QUIHelperForm::showMessageBoxQuestion(info);
}

void QUIHelper::showTipBox(const QString &title, const QString &tip, bool fullScreen, bool center, int timeout)
{
    QUIHelperForm::showTipBox(title, tip, fullScreen, center, timeout);
}

void QUIHelper::hideTipBox()
{
    QUIHelperForm::hideTipBox();
}

QString QUIHelper::showInputBox(const QString &title, int type, int timeout,
                                const QString &placeholderText, bool pwd,
                                const QString &defaultValue)
{
    return QUIHelperForm::showInputBox(title, type, timeout, placeholderText, pwd, defaultValue);
}

int QUIHelper::showDateSelect(QString &dateStart, QString &dateEnd, const QString &format)
{
    return QUIHelperForm::showDateSelect(dateStart, dateEnd, format);
}

void QUIHelper::showAboutInfo(const AboutInfo &info, int timeout, bool exec)
{
    QUIHelperForm::showAboutInfo(info, timeout, exec);
}

void QUIHelper::showSplashInfo(const QString &info, int fontSizeMain, int fontSizeSub, int timeout, bool exec)
{
    QUIHelperForm::showSplashInfo(info, fontSizeMain, fontSizeSub, timeout, exec);
}

void QUIHelper::hideSplashInfo()
{
    QUIHelperForm::hideSplashInfo();
}

QPixmap QUIHelper::getPixmap(QWidget *widget, const QPixmap &pixmap, bool scale)
{
    return QUIHelperImage::getPixmap(widget, pixmap, scale);
}

void QUIHelper::setPixmap(QLabel *label, const QString &file, bool scale)
{
    QUIHelperImage::setPixmap(label, file, scale);
}

void QUIHelper::setLogo(QLabel *label, const QString &file,
                        int width, int height, int offset,
                        const QString &oldColor, const QString &newColor)
{
    QUIHelperImage::setLogo(label, file, width, height, offset, oldColor, newColor);
}

QRect QUIHelper::getCenterRect(const QSize &imageSize, const QRect &widgetRect, int borderWidth, int scaleMode)
{
    return QUIHelperImage::getCenterRect(imageSize, widgetRect, borderWidth, scaleMode);
}

void QUIHelper::getScaledImage(QImage &image, const QSize &widgetSize, int scaleMode, bool fast)
{
    QUIHelperImage::getScaledImage(image, widgetSize, scaleMode, fast);
}

void QUIHelper::initDb(const QString &dbName)
{
    QUIHelperOther::initDb(dbName);
}

void QUIHelper::initFile(const QString &sourceName, const QString &targetName)
{
    QUIHelperOther::initFile(sourceName, targetName);
}

bool QUIHelper::checkIniFile(const QString &iniFile)
{
    return QUIHelperOther::checkIniFile(iniFile);
}

void QUIHelper::setIconBtn(QAbstractButton *btn, const QString &png, int icon)
{
    QUIHelperOther::setIconBtn(btn, png, icon);
}

void QUIHelper::writeInfo(const QString &info, bool needWrite, const QString &filePath)
{
    QUIHelperOther::writeInfo(info, needWrite, filePath);
}

void QUIHelper::writeError(const QString &info, bool needWrite, const QString &filePath)
{
    QUIHelperOther::writeError(info, needWrite, filePath);
}

void QUIHelper::setSystemDateTime(const QString &year, const QString &month, const QString &day, const QString &hour, const QString &min, const QString &sec)
{
    QUIHelperOther::setSystemDateTime(year, month, day, hour, min, sec);
}

void QUIHelper::runWithSystem(const QString &name, const QString &path, bool autoRun)
{
    QUIHelperOther::runWithSystem(name, path, autoRun);
}

QList<QColor> QUIHelper::getColorList()
{
    return QUIHelperOther::getColorList();
}

QStringList QUIHelper::getColorNames()
{
    return QUIHelperOther::getColorNames();
}

QColor QUIHelper::getRandColor()
{
    return QUIHelperOther::getRandColor();
}

void QUIHelper::initRand()
{
    QUIHelperOther::initRand();
}

float QUIHelper::getRandFloat(float min, float max)
{
    return QUIHelperOther::getRandFloat(min, max);
}

double QUIHelper::getRandValue(int min, int max, bool contansMin, bool contansMax)
{
    return QUIHelperOther::getRandValue(min, max, contansMin, contansMax);
}

QStringList QUIHelper::getRandPoint(int count, float mainLng, float mainLat, float dotLng, float dotLat)
{
    return QUIHelperOther::getRandPoint(count, mainLng, mainLat, dotLng, dotLat);
}

int QUIHelper::getRangeValue(int oldMin, int oldMax, int oldValue, int newMin, int newMax)
{
    return QUIHelperOther::getRangeValue(oldMin, oldMax, oldValue, newMin, newMax);
}

void QUIHelper::initTableView(QTableView *tableView, int rowHeight, bool headVisible, bool edit, bool stretchLast)
{
    QUIHelperOther::initTableView(tableView, rowHeight, headVisible, edit, stretchLast);
}

void QUIHelper::openFile(const QString &fileName, const QString &msg)
{
    QUIHelperOther::openFile(fileName, msg);
}

bool QUIHelper::checkRowCount(int rowCount, int maxCount, int warnCount)
{
    return QUIHelperOther::checkRowCount(rowCount, maxCount, warnCount);
}

QString QUIHelper::appendMsg(QTextEdit *textEdit, int type, const QString &data, int maxCount, int &currentCount, bool clear, bool pause)
{
    return QUIHelperOther::appendMsg(textEdit, type, data, maxCount, currentCount, clear, pause);
}

QString QUIHelper::cutString(const QString &text, int len, int left, int right, bool file, const QString &mid)
{
    return QUIHelperOther::cutString(text, len, left, right, file, mid);
}

QString QUIHelper::getTimeString(qint64 time)
{
    return QUIHelperOther::getTimeString(time);
}

QString QUIHelper::getTimeString(QElapsedTimer timer)
{
    return QUIHelperOther::getTimeString(timer);
}

QString QUIHelper::getSizeString(quint64 size)
{
    return QUIHelperOther::getSizeString(size);
}
