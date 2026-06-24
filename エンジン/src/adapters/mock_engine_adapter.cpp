#include "mock_engine_adapter.h"
#include "mock_engine_department_adapter.h"

namespace nexspace {

MockEngineAdapter::MockEngineAdapter()
    : m_impl(std::make_unique<MockDepartmentEngineAdapter>()) {}

MockEngineAdapter::~MockEngineAdapter() = default;

EngineAdapterInfo MockEngineAdapter::info() const {
    return m_impl->info();
}

QVector<Ticket> MockEngineAdapter::getTickets() const {
    return m_impl->getTickets();
}

std::optional<Ticket> MockEngineAdapter::getTicketById(const QString& id) const {
    return m_impl->getTicketById(id);
}

QVector<Agent> MockEngineAdapter::getAgents() const {
    return m_impl->getAgents();
}

QVector<AuditLog> MockEngineAdapter::getAuditLogs() const {
    return m_impl->getAuditLogs();
}

QVector<HistoryEntry> MockEngineAdapter::getHistory() const {
    return m_impl->getHistory();
}

QVector<Department> MockEngineAdapter::getDepartments() const {
    return m_impl->getDepartments();
}

QVector<DepartmentEvent> MockEngineAdapter::getDepartmentEvents() const {
    return m_impl->getDepartmentEvents();
}

QVector<DepartmentRecord> MockEngineAdapter::getDepartmentRecords() const {
    return m_impl->getDepartmentRecords();
}

QVector<CompressedAuditTicket> MockEngineAdapter::getCompressedAuditTickets() const {
    return m_impl->getCompressedAuditTickets();
}

SystemStatus MockEngineAdapter::getSystemStatus() const {
    return m_impl->getSystemStatus();
}

std::optional<Ticket> MockEngineAdapter::createTicketFromCommand(const CreateTicketInput& input) {
    return m_impl->createTicketFromCommand(input);
}

void MockEngineAdapter::tick() {
    m_impl->tick();
}

} // namespace nexspace
