#ifndef ONVIFHEAD_H
#define ONVIFHEAD_H

#include <QtGui>
#include <QtXml>
#include <QtNetwork>

#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
#include <QtWidgets>
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
#include <QtCore5Compat>
#endif

#pragma execution_character_set("utf-8")

#ifndef TIMEMS
#define TIMEMS qPrintable(QTime::currentTime().toString("hh:mm:ss zzz"))
#endif

//返回结果字符串 分隔符
#define OnvifValueSplit ":"
#define OnvifResultSplit "  "

//抓包过滤 ip.addr==192.168.0.64 and http  前缀还有 ip.dst ip.src
//更多规则 https://www.cnblogs.com/nmap/p/6291683.html

#include "onvifstruct.h"
#include "onvifxml.h"
#include "onvifhelper.h"

#include "onvifsearch.h"
#include "onvifdevice.h"
#include "onvifrequest.h"
#include "onvifquery.h"

class OnvifDevice;
#include "onvifbase.h"
#include "onvifptz.h"
#include "onvifvideo.h"
#include "onvifevent.h"
#include "onvifsnap.h"
#include "onvifother.h"

#endif // ONVIFHEAD_H
