#include "soundCueService.h"

namespace nexspace {

SoundCueService::SoundCueService(SoundCueAdapter& adapter)
    : m_adapter(adapter),
      m_settings(adapter.getSettings()) {
}

SoundCueSettings SoundCueService::settings() const {
    return m_settings;
}

void SoundCueService::setSettings(const SoundCueSettings& settings) {
    m_settings = settings;
    m_adapter.updateSettings(settings);
}

QString SoundCueService::eventKey(const SoundCueEvent event, const QString& id) const {
    return QString("%1::%2").arg(toString(event), id);
}

bool SoundCueService::isNightMuted() const {
    if (!m_settings.nightMuteEnabled) {
        return false;
    }
    auto now = QDateTime::currentDateTime();
    auto start = QDateTime::fromString(m_settings.nightMuteStart, "HH:mm");
    auto end = QDateTime::fromString(m_settings.nightMuteEnd, "HH:mm");
    if (!start.isValid() || !end.isValid()) {
        return false;
    }

    QTime nowTime = now.time();
    if (m_settings.nightMuteStart < m_settings.nightMuteEnd) {
        return nowTime >= start.time() && nowTime <= end.time();
    }
    return nowTime >= start.time() || nowTime <= end.time();
}

void SoundCueService::emitIfAllowed(const SoundCueEvent event, const SoundCuePayload& payload) {
    if (!m_settings.enabled || m_settings.mode == SoundCueMode::Off) {
        return;
    }
    if (m_settings.mode == SoundCueMode::Off) {
        return;
    }
    if (isNightMuted() && (event == SoundCueEvent::TicketProcessCompleted || event == SoundCueEvent::TicketProcessStarted)) {
        return;
    }

    const QString key = eventKey(event, payload.ticketId + payload.auditLogId);
    const QDateTime now = QDateTime::currentDateTime();
    if (m_lastEmit.contains(key)) {
        const qint64 elapsed = m_lastEmit.value(key).msecsTo(now);
        if (elapsed < m_settings.cooldownMs) {
            return;
        }
    }
    m_lastEmit.insert(key, now);
    m_adapter.emitCue(payload);
}

void SoundCueService::handleTicketTransition(const Ticket& before, const Ticket& after) {
    if (before.status != TicketStatus::PROCESSING && after.status == TicketStatus::PROCESSING) {
        SoundCuePayload payload{
            SoundCueEvent::TicketProcessStarted,
            after.id,
            after.assignedAgentId,
            QString(),
            "info",
            QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
        };
        emitIfAllowed(SoundCueEvent::TicketProcessStarted, payload);
    }

    if ((before.status != TicketStatus::DONE && before.status != TicketStatus::AUDIT_PASSED) &&
        (after.status == TicketStatus::DONE || after.status == TicketStatus::AUDIT_PASSED)) {
        SoundCuePayload payload{
            SoundCueEvent::TicketProcessCompleted,
            after.id,
            after.assignedAgentId,
            QString(),
            "info",
            QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
        };
        emitIfAllowed(SoundCueEvent::TicketProcessCompleted, payload);
    }

    if (after.status == TicketStatus::ERROR) {
        SoundCuePayload payload{
            SoundCueEvent::SystemCriticalError,
            after.id,
            after.assignedAgentId,
            QString(),
            "critical",
            QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
        };
        emitIfAllowed(SoundCueEvent::SystemCriticalError, payload);
    }
}

void SoundCueService::handleNewAuditLogs(const QVector<AuditLog>& before, const QVector<AuditLog>& after) {
    QSet<QString> beforeIds;
    beforeIds.reserve(before.size());
    for (const auto& log : before) {
        beforeIds.insert(log.id);
    }

    for (const auto& log : after) {
        if (beforeIds.contains(log.id) || m_processedLogIds.contains(log.id)) {
            continue;
        }
        if (log.result == AuditResult::PASS) {
            SoundCuePayload payload{SoundCueEvent::AuditPassed, log.ticketId, log.targetAgentId, log.id, "info", log.createdAt};
            emitIfAllowed(SoundCueEvent::AuditPassed, payload);
        } else if (log.result == AuditResult::REJECT) {
            SoundCuePayload payload{SoundCueEvent::AuditRejected, log.ticketId, log.targetAgentId, log.id, "warning", log.createdAt};
            emitIfAllowed(SoundCueEvent::AuditRejected, payload);
        } else if (log.result == AuditResult::SYSTEM_ERROR) {
            SoundCuePayload payload{SoundCueEvent::SystemCriticalError, log.ticketId, log.targetAgentId, log.id, "critical", log.createdAt};
            emitIfAllowed(SoundCueEvent::SystemCriticalError, payload);
        }
        m_processedLogIds.insert(log.id);
    }
}

void SoundCueService::onDataRefresh(const QVector<Ticket>& beforeTickets,
                                   const QVector<Ticket>& afterTickets,
                                   const QVector<AuditLog>& beforeLogs,
                                   const QVector<AuditLog>& afterLogs) {
    if (!m_settings.enabled || m_settings.mode == SoundCueMode::Off) {
        return;
    }

    QHash<QString, Ticket> beforeById;
    for (const auto& ticket : beforeTickets) {
        beforeById.insert(ticket.id, ticket);
    }

    for (const auto& after : afterTickets) {
        const bool exists = beforeById.contains(after.id);
        if (!exists) {
            continue;
        }
        handleTicketTransition(beforeById.value(after.id), after);
    }
    handleNewAuditLogs(beforeLogs, afterLogs);
}

} // namespace nexspace
