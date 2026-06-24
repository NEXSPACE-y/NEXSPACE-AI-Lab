#pragma once

#include "soundCueTypes.h"

namespace nexspace {

class SoundCueAdapter {
public:
    virtual ~SoundCueAdapter() = default;
    virtual void emitCue(const SoundCuePayload& payload) = 0;
    virtual SoundCueSettings getSettings() const = 0;
    virtual void updateSettings(const SoundCueSettings& settings) = 0;
};

} // namespace nexspace
