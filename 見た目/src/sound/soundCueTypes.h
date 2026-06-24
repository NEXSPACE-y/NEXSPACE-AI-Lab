#pragma once

#include <QString>

namespace nexspace {

enum class SoundCueEvent {
    TicketProcessStarted,
    TicketProcessCompleted,
    AuditPassed,
    AuditRejected,
    SystemCriticalError
};

enum class SoundCueMode {
    Off,
    Minimal,
    Normal,
    Gunji
};

struct SoundCueSettings {
    bool enabled = false;
    SoundCueMode mode = SoundCueMode::Off;
    double masterVolume = 0.5;
    double eventVolume = 0.4;
    double errorVolume = 0.5;
    int cooldownMs = 5000;
    bool nightMuteEnabled = true;
    QString nightMuteStart = "22:00";
    QString nightMuteEnd = "08:00";
};

struct SoundCuePayload {
    SoundCueEvent event;
    QString ticketId;
    QString agentId;
    QString auditLogId;
    QString severity;
    QString createdAt;
};

inline QString toString(SoundCueEvent event) {
    switch (event) {
        case SoundCueEvent::TicketProcessStarted:
            return "ticket.process_started";
        case SoundCueEvent::TicketProcessCompleted:
            return "ticket.process_completed";
        case SoundCueEvent::AuditPassed:
            return "audit.passed";
        case SoundCueEvent::AuditRejected:
            return "audit.rejected";
        case SoundCueEvent::SystemCriticalError:
            return "system.critical_error";
        default:
            return "ticket.process_started";
    }
}

inline QString toString(SoundCueMode mode) {
    switch (mode) {
        case SoundCueMode::Off: return "off";
        case SoundCueMode::Minimal: return "minimal";
        case SoundCueMode::Normal: return "normal";
        case SoundCueMode::Gunji: return "gunji";
        default: return "off";
    }
}

} // namespace nexspace
