#pragma once
#include "ttsengine.h"

#ifdef HAVE_QT_TTS
#include <QTextToSpeech>
#include <QLocale>

// Lightweight TTS engine backed by Qt's QTextToSpeech (uses OS TTS APIs on Windows/macOS/Linux).
class QtTtsEngine : public TtsEngine
{
public:
    explicit QtTtsEngine();
    ~QtTtsEngine() override = default;

    void speak(const QString &text) override;
    void stop() override;
    bool isAvailable() const override;

private:
    QTextToSpeech m_speech;
};

#else

// Stub when Qt5TextToSpeech is not available.
class QtTtsEngine : public TtsEngine
{
public:
    explicit QtTtsEngine() = default;
    void speak(const QString &) override {}
    void stop() override {}
    bool isAvailable() const override { return false; }
};

#endif
