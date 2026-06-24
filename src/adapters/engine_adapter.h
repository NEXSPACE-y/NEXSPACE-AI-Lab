#pragma once

#include "../domain.h"
#include <QString>
#include <QVector>
#include <optional>
#include <memory>

namespace nexspace {

constexpr int kMaxCommandTextLength = 2000;

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

struct CreateTicketInput {
    std::optional<QString> title;
    QString commandText;
    QString type;
    TicketPriority priority = TicketPriority::normal;
    std::optional<AgentRole> requestedAgentRole;
    bool auditRequired = true;
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
    virtual QVector<Department> getDepartments() const = 0;
    virtual QVector<DepartmentEvent> getDepartmentEvents() const = 0;
    virtual QVector<DepartmentRecord> getDepartmentRecords() const = 0;
    virtual std::optional<Ticket> createTicketFromCommand(const CreateTicketInput& input) = 0;
    virtual void tick() = 0;
};

} // namespace nexspace
