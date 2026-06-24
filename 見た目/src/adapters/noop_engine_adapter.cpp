#include "noop_engine_adapter.h"

namespace nexspace {

NoopEngineAdapter::NoopEngineAdapter() {
    m_info = {EngineAdapterType::Noop, false, "NoopEngineAdapter(未接続)"};
}

EngineAdapterInfo NoopEngineAdapter::info() const {
    return m_info;
}

SystemStatus NoopEngineAdapter::getSystemStatus() const {
    return {"error", 0, 0, 0, 0, 0, 0, 0};
}

QVector<Ticket> NoopEngineAdapter::getTickets() const {
    return {};
}

std::optional<Ticket> NoopEngineAdapter::getTicketById(const QString& id) const {
    Q_UNUSED(id)
    return std::nullopt;
}

QVector<Agent> NoopEngineAdapter::getAgents() const {
    return {};
}

QVector<AuditLog> NoopEngineAdapter::getAuditLogs() const {
    return {};
}

QVector<HistoryEntry> NoopEngineAdapter::getHistory() const {
    return {};
}

void NoopEngineAdapter::tick() {
}

std::optional<Ticket> NoopEngineAdapter::createTicketFromCommand(const CreateTicketInput& input) {
    Q_UNUSED(input)
    return std::nullopt;
}

} // namespace nexspace
