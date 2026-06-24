#pragma once

#include "engine_adapter.h"
#include <memory>

namespace nexspace {

class MockDepartmentEngineAdapter;


class MockEngineAdapter : public EngineAdapter {
public:
    MockEngineAdapter();
    ~MockEngineAdapter() override;

    EngineAdapterInfo info() const override;
    SystemStatus getSystemStatus() const override;
    QVector<Ticket> getTickets() const override;
    std::optional<Ticket> getTicketById(const QString& id) const override;
    QVector<Agent> getAgents() const override;
    QVector<AuditLog> getAuditLogs() const override;
    QVector<HistoryEntry> getHistory() const override;
    QVector<Department> getDepartments() const override;
    QVector<DepartmentEvent> getDepartmentEvents() const override;
    QVector<DepartmentRecord> getDepartmentRecords() const override;
    QVector<CompressedAuditTicket> getCompressedAuditTickets() const;
    void tick() override;
    std::optional<Ticket> createTicketFromCommand(const CreateTicketInput& input) override;

private:
    std::unique_ptr<MockDepartmentEngineAdapter> m_impl;
};

} // namespace nexspace
