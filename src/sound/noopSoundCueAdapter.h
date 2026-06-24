#pragma once

#include "soundCueAdapter.h"

namespace nexspace {

class NoopSoundCueAdapter : public SoundCueAdapter {
public:
    void emitCue(const SoundCuePayload& payload) override;
    SoundCueSettings getSettings() const override;
    void updateSettings(const SoundCueSettings& settings) override;

private:
    SoundCueSettings m_settings;
};

} // namespace nexspace
