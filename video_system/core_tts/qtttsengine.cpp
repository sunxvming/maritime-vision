#include "qtttsengine.h"

#ifdef HAVE_QT_TTS

QtTtsEngine::QtTtsEngine()
    : m_speech("sapi")  // Windows SAPI backend; empty string = platform default
{
    m_speech.setLocale(QLocale(QLocale::Chinese, QLocale::China));
    m_speech.setRate(0.0);    // -1..1, 0 = normal
    m_speech.setVolume(1.0);  // 0..1
}

void QtTtsEngine::speak(const QString &text)
{
    if (!isAvailable()) return;
    m_speech.say(text);
}

void QtTtsEngine::stop()
{
    m_speech.stop();
}

bool QtTtsEngine::isAvailable() const
{
    return m_speech.state() != QTextToSpeech::BackendError;
}

#endif
