#pragma once
#include <QString>

// Abstract TTS engine interface. Implement this to add new TTS backends.
class TtsEngine
{
public:
    virtual ~TtsEngine() = default;
    virtual void speak(const QString &text) = 0;
    virtual void stop() = 0;
    virtual bool isAvailable() const = 0;
};
