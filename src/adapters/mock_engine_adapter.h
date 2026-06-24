#pragma once

#include "engine_adapter.h"
#include <QDateTime>
#include <QHash>
#include <QVector>

namespace nexspace {

class MockEngineAdapter : public EngineAdapter {
public:
    MockEngineAdapter();

    EngineAdapterInfo info() const override;
    SystemStatus getSystemStatus() const override;
    QVector<Ticket> getTickets() const override;
    std::optional<Ticket> getTicketById(const QString& id) const override;
    QVector<Agent> getAgents() const override;
    QVector<AuditLog> getAuditLogs() const override;
    QVector<HistoryEntry> getHistory() const override;
    void tick() override;

private:
    void seed();
    void advanceTicketFlow(Ticket& ticket, int step);
    void pushAuditLog(const QString& ticketId, const QString& agentId, AuditResult result, const QString& category,
                      const QString& reason, const QString& action, bool retryable);
    void pushHistory(const Ticket& ticket, const QString& processResult, bool hasError);
    void recalcSystemStatus();

    EngineAdapterInfo m_info;
    QVector<Ticket> m_tickets;
    QVector<Agent> m_agents;
    QVector<AuditLog> m_auditLogs;
    QVector<HistoryEntry> m_history;
    QHash<QString, int> m_phaseByTicket;
    SystemStatus m_systemStatus;
    int m_tick = 0;
};

} // namespace nexspace
