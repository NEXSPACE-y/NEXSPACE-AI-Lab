#pragma once

#include "engine_adapter.h"

namespace nexspace {

class NoopEngineAdapter : public EngineAdapter {
public:
    NoopEngineAdapter();

    EngineAdapterInfo info() const override;
    SystemStatus getSystemStatus() const override;
    QVector<Ticket> getTickets() const override;
    std::optional<Ticket> getTicketById(const QString& id) const override;
    QVector<Agent> getAgents() const override;
    QVector<AuditLog> getAuditLogs() const override;
    QVector<HistoryEntry> getHistory() const override;
    void tick() override;
    std::optional<Ticket> createTicketFromCommand(const CreateTicketInput& input) override;

private:
    EngineAdapterInfo m_info;
};

} // namespace nexspace
