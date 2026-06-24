#pragma once

#include "engine_adapter.h"
#include <QDateTime>
#include <QHash>
#include <QStringList>
#include <QVector>

namespace nexspace {

class MockEngineAdapter : public EngineAdapter {
public:
    MockEngineAdapter();

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
    void tick() override;

private:
    void seed();
    void advanceTicketFlow(Ticket& ticket, int step);
    void pushAuditLog(const QString& ticketId, const QString& agentId, AuditResult result, const QString& category,
                      const QString& reason, const QString& action, bool retryable, const QString& departmentId = QString());
    void pushHistory(const Ticket& ticket, const QString& processResult, bool hasError);
    QStringList routeForTicket(const Ticket& ticket) const;
    QStringList buildInitialRoute(const Ticket& ticket) const;
    void appendDepartmentHistory(Ticket& ticket, const QString& from, const QString& to);
    void moveToDepartment(Ticket& ticket, const QString& nextDepartmentId, bool checkRouteViolation = true);
    void updateDepartmentStatus();
    void emitDepartmentEvent(const QString& ticketId, const QString& from, const QString& to, DepartmentEventType type, const QString& note);
    void registerDepartmentRegistry();
    void syncTicketDepartmentIfNeeded(Ticket& ticket, int phase);
    void recalcSystemStatus();

    EngineAdapterInfo m_info;
    QVector<Ticket> m_tickets;
    QVector<Agent> m_agents;
    QVector<AuditLog> m_auditLogs;
    QVector<HistoryEntry> m_history;
    QVector<Department> m_departments;
    QVector<DepartmentEvent> m_departmentEvents;
    QVector<DepartmentRecord> m_departmentRecords;
    QHash<QString, int> m_phaseByTicket;
    SystemStatus m_systemStatus;
    int m_departmentEventSeq = 0;
    int m_departmentRecordSeq = 0;
    int m_tick = 0;
};

} // namespace nexspace
