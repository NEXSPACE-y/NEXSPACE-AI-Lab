#include "noopSoundCueAdapter.h"
#include <QDebug>

namespace nexspace {

void NoopSoundCueAdapter::emitCue(const SoundCuePayload& payload) {
    Q_UNUSED(payload);
    // 将来実装時の差替え口。現状は未接続として音源再生を行わない。
}

SoundCueSettings NoopSoundCueAdapter::getSettings() const {
    return m_settings;
}

void NoopSoundCueAdapter::updateSettings(const SoundCueSettings& settings) {
    m_settings = settings;
}

} // namespace nexspace
