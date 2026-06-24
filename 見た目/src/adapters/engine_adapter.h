#pragma once

#include "../domain.h"
#include <QString>
#include <QVector>
#include <memory>
#include <optional>

namespace nexspace {

enum class EngineAdapterType {
    Mock,
    Real,
    Noop
};

constexpr int kMaxCommandTextLength = 2000;

struct CreateTicketInput {
    std::optional<QString> title;
    QString commandText;
    QString type = "依頼札";
    TicketPriority priority = TicketPriority::normal;
    std::optional<AgentRole> requestedAgentRole = std::nullopt;
    bool auditRequired = false;
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
    virtual std::optional<Ticket> createTicketFromCommand(const CreateTicketInput& input) = 0;
};

} // namespace nexspace
