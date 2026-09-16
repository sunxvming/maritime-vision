#include "ttsmanager.h"
#include "qtttsengine.h"

TtsManager &TtsManager::instance()
{
    static TtsManager s_instance;
    return s_instance;
}

TtsManager::TtsManager()
    : m_engine(std::make_unique<QtTtsEngine>())
{
}

void TtsManager::speak(const QString &text)
{
    if (m_engine) m_engine->speak(text);
}

void TtsManager::stop()
{
    if (m_engine) m_engine->stop();
}

bool TtsManager::isAvailable() const
{
    return m_engine && m_engine->isAvailable();
}

void TtsManager::setEngine(std::unique_ptr<TtsEngine> engine)
{
    m_engine = std::move(engine);
}
