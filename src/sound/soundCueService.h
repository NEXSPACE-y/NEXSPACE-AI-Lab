#pragma once

#include "../adapters/engine_adapter.h"
#include "soundCueAdapter.h"
#include <QDateTime>
#include <QHash>
#include <QSet>

namespace nexspace {

class SoundCueService {
public:
    explicit SoundCueService(SoundCueAdapter& adapter);

    SoundCueSettings settings() const;
    void setSettings(const SoundCueSettings& settings);

    void onDataRefresh(const QVector<Ticket>& beforeTickets,
                       const QVector<Ticket>& afterTickets,
                       const QVector<AuditLog>& beforeLogs,
                       const QVector<AuditLog>& afterLogs);

private:
    void emitIfAllowed(const SoundCueEvent event, const SoundCuePayload& payload);
    void handleTicketTransition(const Ticket& before, const Ticket& after);
    void handleNewAuditLogs(const QVector<AuditLog>& before, const QVector<AuditLog>& after);
    QString eventKey(const SoundCueEvent event, const QString& id) const;
    bool isNightMuted() const;

    SoundCueAdapter& m_adapter;
    SoundCueSettings m_settings;
    QHash<QString, QDateTime> m_lastEmit;
    QSet<QString> m_processedLogIds;
};

} // namespace nexspace
