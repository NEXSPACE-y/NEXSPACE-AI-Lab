#include "mock_engine_department_adapter.h"

namespace nexspace {

MockDepartmentEngineAdapter::MockDepartmentEngineAdapter() : m_core(std::make_unique<DepartmentFlowEngine>()) {}

MockDepartmentEngineAdapter::~MockDepartmentEngineAdapter() = default;

EngineAdapterInfo MockDepartmentEngineAdapter::info() const {
    return m_core->info();
}

SystemStatus MockDepartmentEngineAdapter::getSystemStatus() const {
    return m_core->getSystemStatus();
}

QVector<Ticket> MockDepartmentEngineAdapter::getTickets() const {
    return m_core->getTickets();
}

std::optional<Ticket> MockDepartmentEngineAdapter::getTicketById(const QString& id) const {
    return m_core->getTicketById(id);
}

QVector<Agent> MockDepartmentEngineAdapter::getAgents() const {
    return m_core->getAgents();
}

QVector<AuditLog> MockDepartmentEngineAdapter::getAuditLogs() const {
    return m_core->getAuditLogs();
}

QVector<HistoryEntry> MockDepartmentEngineAdapter::getHistory() const {
    return m_core->getHistory();
}

QVector<Department> MockDepartmentEngineAdapter::getDepartments() const {
    return m_core->getDepartments();
}

QVector<DepartmentEvent> MockDepartmentEngineAdapter::getDepartmentEvents() const {
    return m_core->getDepartmentEvents();
}

QVector<DepartmentRecord> MockDepartmentEngineAdapter::getDepartmentRecords() const {
    return m_core->getDepartmentRecords();
}

QVector<CompressedAuditTicket> MockDepartmentEngineAdapter::getCompressedAuditTickets() const {
    return m_core->getCompressedAuditTickets();
}

void MockDepartmentEngineAdapter::tick() {
    m_core->tick();
}

std::optional<Ticket> MockDepartmentEngineAdapter::createTicketFromCommand(const CreateTicketInput& input) {
    return m_core->createTicketFromCommand(input);
}

} // namespace nexspace
