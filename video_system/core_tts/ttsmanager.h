#pragma once
#include "ttsengine.h"
#include <memory>
#include <QString>

// Singleton facade. Call TtsManager::instance().speak(text) from anywhere.
// By default uses QtTtsEngine. Call setEngine() to swap in a different backend.
class TtsManager
{
public:
    static TtsManager &instance();

    void speak(const QString &text);
    void stop();
    bool isAvailable() const;

    void setEngine(std::unique_ptr<TtsEngine> engine);

private:
    TtsManager();
    std::unique_ptr<TtsEngine> m_engine;
};
