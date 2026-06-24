#pragma once

#include "../domain.h"
#include <QString>
#include <QVector>
#include <memory>

namespace nexspace {

enum class EngineAdapterType {
    Mock,
    Real,
    Noop
};

struct EngineAdapterInfo {
    EngineAdapterType type = EngineAdapterType::Mock;
    bool connected = false;
    QString name = "Unknown";
};

class EngineAdapter {
public:
    virtual ~EngineAdapter() = default;

    virtual EngineAdapterInfo info() const = 0;
    virtual SystemStatus getSystemStatus() const = 0;
    virtual QVector<Ticket> getTickets() const = 0;
    virtual std::optional<Ticket> getTicketById(const QString& id) const = 0;
    virtual QVector<Agent> getAgents() const = 0;
    virtual QVector<AuditLog> getAuditLogs() const = 0;
    virtual QVector<HistoryEntry> getHistory() const = 0;
    virtual void tick() = 0;
};

} // namespace nexspace
