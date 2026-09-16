#ifndef SAVEINCLUDE_H
#define SAVEINCLUDE_H

#include "qobject.h"
#include "qfile.h"
#include "qdatetime.h"
#include "qdebug.h"

#ifdef Q_CC_MSVC
#pragma execution_character_set("utf-8")
#endif

#ifndef TIMEMS
#define TIMEMS qPrintable(QTime::currentTime().toString("HH:mm:ss zzz"))
#endif

#endif // SAVEINCLUDE_H
