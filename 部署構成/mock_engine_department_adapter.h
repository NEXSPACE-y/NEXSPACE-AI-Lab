#pragma once

#include "department_flow_engine.h"

#include <memory>

namespace nexspace {

class MockDepartmentEngineAdapter {
public:
    MockDepartmentEngineAdapter();
    ~MockDepartmentEngineAdapter();

    EngineAdapterInfo info() const;
    SystemStatus getSystemStatus() const;
    QVector<Ticket> getTickets() const;
    std::optional<Ticket> getTicketById(const QString& id) const;
    QVector<Agent> getAgents() const;
    QVector<AuditLog> getAuditLogs() const;
    QVector<HistoryEntry> getHistory() const;
    QVector<Department> getDepartments() const;
    QVector<DepartmentEvent> getDepartmentEvents() const;
    QVector<DepartmentRecord> getDepartmentRecords() const;
    QVector<CompressedAuditTicket> getCompressedAuditTickets() const;
    void tick();
    std::optional<Ticket> createTicketFromCommand(const CreateTicketInput& input);

private:
    std::unique_ptr<DepartmentFlowEngine> m_core;
};

} // namespace nexspace
