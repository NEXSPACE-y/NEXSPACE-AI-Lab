#pragma once

#include "adapters/engine_adapter.h"
#include "department_registry.h"

#include <QDateTime>
#include <QHash>
#include <optional>
#include <QStringList>
#include <QVector>

namespace nexspace {

class DepartmentFlowEngine {
public:
    DepartmentFlowEngine();

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
    void seed();
    Ticket buildTicketFromCommand(const CreateTicketInput& input);
    int nextTicketNumberFromExisting() const;
    void advanceTicketFlow(Ticket& ticket, int step);
    void pushAuditLog(const QString& ticketId, const QString& agentId, AuditResult result, const QString& category,
                      const QString& reason, const QString& action, bool retryable, const QString& departmentId = QString());
    void pushHistory(const Ticket& ticket, const QString& processResult, bool hasError);
    QStringList routeForTicket(const Ticket& ticket) const;
    QStringList buildInitialRoute(const Ticket& ticket) const;
    void appendDepartmentHistory(Ticket& ticket, const QString& from, const QString& to);
    void syncTicketDepartmentAliases(Ticket& ticket);
    void emitDepartmentEvent(const Ticket& ticket, const QString& note, DepartmentEventType type, const QString& status = "ok");
    void moveToDepartment(Ticket& ticket, const QString& nextDepartmentId, bool checkRouteViolation = true, DepartmentEventType eventType = DepartmentEventType::TicketForwarded, const QString& eventStatus = "forwarded", int step = -1);
    void updateDepartmentStatus();
    void registerDepartmentRegistry();
    void syncTicketDepartmentIfNeeded(Ticket& ticket, int phase, const QStringList& route);
    void recalcSystemStatus();
    void initializeTicketDepartmentState(Ticket& ticket, int phase, const QStringList& route);
    void initializeTicketStatuses(Ticket& ticket);
    void appendRecordSummary(const QString& ticketId, const QString& departmentId, const QString& summary = QString());
    void appendAuditSummary(const QString& ticketId, const QString& targetDepartmentId, const QString& result, const QStringList& flags, const ViolationLevel level, const QString& requiredAction);
    void evaluateInspectionAndAdvance(Ticket& ticket, const QStringList& route, int step);
    void evaluateAuditAndAdvance(Ticket& ticket, const QStringList& route, int step);

    EngineAdapterInfo m_info;
    QVector<Ticket> m_tickets;
    QVector<Agent> m_agents;
    QVector<AuditLog> m_auditLogs;
    QVector<HistoryEntry> m_history;
    QVector<Department> m_departments;
    QVector<DepartmentEvent> m_departmentEvents;
    QVector<DepartmentRecord> m_departmentRecords;
    QVector<CompressedAuditTicket> m_compressedAudits;
    QHash<QString, int> m_phaseByTicket;
    QHash<QString, bool> m_auditRejectedBefore;
    SystemStatus m_systemStatus;
    int m_departmentEventSeq = 0;
    int m_departmentRecordSeq = 0;
    int m_tick = 0;
    int m_nextTicketNumber = 1;
};

} // namespace nexspace
