#include "real_engine_adapter.h"

namespace nexspace {

RealEngineAdapter::RealEngineAdapter(const QString& endpoint)
    : m_endpoint(endpoint) {
    m_info = {EngineAdapterType::Real, false, QString("RealEngineAdapter(%1)").arg(endpoint)};
}

EngineAdapterInfo RealEngineAdapter::info() const {
    return m_info;
}

SystemStatus RealEngineAdapter::getSystemStatus() const {
    return {"error", 0, 0, 0, 0, 0, 0, 0};
}

QVector<Ticket> RealEngineAdapter::getTickets() const {
    return {};
}

std::optional<Ticket> RealEngineAdapter::getTicketById(const QString& id) const {
    Q_UNUSED(id)
    return std::nullopt;
}

QVector<Agent> RealEngineAdapter::getAgents() const {
    return {};
}

QVector<AuditLog> RealEngineAdapter::getAuditLogs() const {
    return {};
}

QVector<HistoryEntry> RealEngineAdapter::getHistory() const {
    return {};
}

void RealEngineAdapter::tick() {
    // 未接続時は noop 相当の挙動。接続実装は別途追加される前提。
}

} // namespace nexspace
