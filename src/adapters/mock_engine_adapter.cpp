#include "mock_engine_adapter.h"

namespace nexspace {

namespace {
QString utcNow() {
    return QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss");
}

QStringList branchDepartment(const Ticket& ticket) {
    if (ticket.type.contains("技術") || ticket.summary.contains("実装") || ticket.title.contains("実装") || ticket.type.contains("要件")) {
        return {"07_技術部"};
    }
    if (ticket.type.contains("営業") || ticket.title.contains("営業") || ticket.summary.contains("顧客") || ticket.summary.contains("顧客")) {
        return {"06_営業部"};
    }
    return {"05_制作室"};
}

QStringList baseRouteFor(const QString& department) {
    if (department == "07_技術部") {
        return {"02_秘書室", "01_社長室", "04_戦略室", "05_制作室", "07_技術部", "08_検査部", "03_監査室", "10_庶務課"};
    }
    if (department == "06_営業部") {
        return {"02_秘書室", "01_社長室", "04_戦略室", "06_営業部", "08_検査部", "03_監査室", "10_庶務課"};
    }
    return {"02_秘書室", "01_社長室", "04_戦略室", "05_制作室", "08_検査部", "03_監査室", "10_庶務課"};
}
} // namespace

MockEngineAdapter::MockEngineAdapter() {
    m_info = {EngineAdapterType::Mock, true, "MockEngineAdapter"};
    registerDepartmentRegistry();
    seed();
    recalcSystemStatus();
    updateDepartmentStatus();
}

EngineAdapterInfo MockEngineAdapter::info() const {
    return m_info;
}

void MockEngineAdapter::registerDepartmentRegistry() {
    m_departments = {
        {"01_社長室", "社長室", QString(), true, 0, 0, 0, 0, 0},
        {"02_秘書室", "秘書室", "01_社長室", false, 0, 0, 0, 0, 0},
        {"03_監査室", "監査室", QString(), true, 0, 0, 0, 0, 0},
        {"08_検査部", "検査部", "03_監査室", false, 0, 0, 0, 0, 0},
        {"04_戦略室", "戦略室", QString(), true, 0, 0, 0, 0, 0},
        {"05_制作室", "制作室", "04_戦略室", true, 0, 0, 0, 0, 0},
        {"06_営業部", "営業部", "04_戦略室", false, 0, 0, 0, 0, 0},
        {"07_技術部", "技術部", "05_制作室", false, 0, 0, 0, 0, 0},
        {"09_総務部", "総務部", QString(), true, 0, 0, 0, 0, 0},
        {"10_庶務課", "庶務課", "09_総務部", false, 0, 0, 0, 0, 0}
    };
}

QStringList MockEngineAdapter::routeForTicket(const Ticket& ticket) const {
    const QStringList candidates = branchDepartment(ticket);
    return baseRouteFor(candidates.constFirst());
}

QStringList MockEngineAdapter::buildInitialRoute(const Ticket& ticket) const {
    return routeForTicket(ticket);
}

void MockEngineAdapter::appendDepartmentHistory(Ticket& ticket, const QString& from, const QString& to) {
    Q_UNUSED(from)
    if (!to.isEmpty()) {
        ticket.routeHistory.append(to);
        if (ticket.routeHistory.size() > 16) {
            ticket.routeHistory.removeFirst();
        }
    }
}

void MockEngineAdapter::emitDepartmentEvent(const QString& ticketId, const QString& from, const QString& to,
                                          DepartmentEventType type, const QString& note) {
    m_departmentEvents.prepend({QString("D-%1").arg(m_departmentEventSeq++, 4, 10, QLatin1Char('0')),
                              ticketId,
                              from,
                              to,
                              to,
                              type,
                              note,
                              utcNow()});
    if (m_departmentEvents.size() > 300) {
        m_departmentEvents.resize(300);
    }
}

void MockEngineAdapter::moveToDepartment(Ticket& ticket, const QString& nextDepartmentId, bool checkRouteViolation) {
    if (ticket.currentDepartmentId == nextDepartmentId) {
        return;
    }

    const QStringList route = routeForTicket(ticket);
    if (checkRouteViolation) {
        if (!nextDepartmentId.isEmpty() && !route.contains(nextDepartmentId)) {
            return;
        }
        if (!ticket.currentDepartmentId.isEmpty()) {
            int currentIndex = route.indexOf(ticket.currentDepartmentId);
            int nextIndex = route.indexOf(nextDepartmentId);
            if (currentIndex >= 0 && nextIndex >= 0 && nextIndex > currentIndex + 1) {
                return;
            }
        }
    }

    emitDepartmentEvent(ticket.id, ticket.currentDepartmentId, nextDepartmentId, DepartmentEventType::Routed,
                        QString("%1 -> %2").arg(ticket.currentDepartmentId, nextDepartmentId));
    ticket.previousDepartmentId = ticket.currentDepartmentId;
    ticket.currentDepartmentId = nextDepartmentId;
    appendDepartmentHistory(ticket, ticket.previousDepartmentId, ticket.currentDepartmentId);

    int nextIndex = route.indexOf(nextDepartmentId);
    if (nextIndex >= 0 && nextIndex + 1 < route.size()) {
        ticket.nextDepartmentId = route[nextIndex + 1];
    } else {
        ticket.nextDepartmentId.clear();
    }

    if (ticket.currentDepartmentId == "10_庶務課") {
        m_departmentRecords.prepend({QString("R-%1").arg(m_departmentRecordSeq++, 4, 10, QLatin1Char('0')),
                                    ticket.id,
                                    ticket.currentDepartmentId,
                                    QString("%1 の処理履歴を庶務課へ移送").arg(ticket.id),
                                    utcNow()});
        if (m_departmentRecords.size() > 300) {
            m_departmentRecords.resize(300);
        }
    }
}

void MockEngineAdapter::syncTicketDepartmentIfNeeded(Ticket& ticket, int phase) {
    if (!ticket.currentDepartmentId.isEmpty()) {
        return;
    }
    const QStringList route = buildInitialRoute(ticket);
    const int seedIndex = qMin(qMax(phase, 0), route.size() - 1);
    ticket.currentDepartmentId = route[seedIndex];
    ticket.previousDepartmentId.clear();
    ticket.nextDepartmentId = (seedIndex + 1 < route.size()) ? route[seedIndex + 1] : QString();
    ticket.routeHistory.clear();
    ticket.routeHistory = {ticket.currentDepartmentId};
}

void MockEngineAdapter::seed() {
    Agent orchestrator = {"A-000", "上杉", AgentRole::Orchestrator, "running", 0, 0, 0, 0, utcNow()};
    Agent worker1 = {"A-101", "柴田", AgentRole::Worker, "running", 2, 12, 1, 0, utcNow()};
    Agent worker2 = {"A-102", "福島", AgentRole::Worker, "idle", 0, 8, 0, 0, utcNow()};
    Agent auditor = {"A-201", "観測官", AgentRole::Auditor, "running", 1, 6, 3, 1, utcNow()};
    Agent observer = {"A-301", "物見", AgentRole::Observer, "running", 0, 4, 0, 0, utcNow()};
    Agent system = {"SYS", "本陣", AgentRole::System, "running", 0, 30, 0, 0, utcNow()};
    m_agents = {orchestrator, worker1, worker2, auditor, observer, system};

    Ticket t1;
    t1.id = "TK-001";
    t1.title = "予算異動依頼";
    t1.summary = "予算明細の修正指示を生成し、監査を通過後に保存する";
    t1.type = "依頼札";
    t1.priority = TicketPriority::high;
    t1.status = TicketStatus::WAITING;
    t1.createdAt = utcNow();
    t1.updatedAt = utcNow();

    Ticket t2;
    t2.id = "TK-002";
    t2.title = "顧客連絡文面";
    t2.summary = "契約条項を崩さない対外文面の下処理";
    t2.type = "業務連絡札";
    t2.priority = TicketPriority::normal;
    t2.status = TicketStatus::PROCESSING;
    t2.assignedAgentId = "A-101";
    t2.createdAt = utcNow();
    t2.updatedAt = utcNow();

    Ticket t3;
    t3.id = "TK-003";
    t3.title = "内部調整依頼";
    t3.summary = "監査保留案件の説明追加";
    t3.type = "割当札";
    t3.priority = TicketPriority::critical;
    t3.status = TicketStatus::AUDIT_PENDING;
    t3.assignedAgentId = "A-102";
    t3.createdAt = utcNow();
    t3.updatedAt = utcNow();

    Ticket t4;
    t4.id = "TK-004";
    t4.title = "報告文整形";
    t4.summary = "最終監査パスが必須の文言整形";
    t4.type = "処理後札";
    t4.priority = TicketPriority::low;
    t4.status = TicketStatus::AUDIT_PASSED;
    t4.assignedAgentId = "A-101";
    t4.auditResult = AuditResult::PASS;
    t4.auditCategory = "requirement_missing";
    t4.createdAt = utcNow();
    t4.updatedAt = utcNow();

    Ticket t5;
    t5.id = "TK-005";
    t5.title = "外部API連携";
    t5.summary = "形式不備の可能性がある返答を再検査";
    t5.type = "最終報告札";
    t5.priority = TicketPriority::critical;
    t5.status = TicketStatus::AUDIT_REJECTED;
    t5.assignedAgentId = "A-201";
    t5.auditResult = AuditResult::REJECT;
    t5.auditCategory = "scope_deviation";
    t5.createdAt = utcNow();
    t5.updatedAt = utcNow();

    Ticket t6;
    t6.id = "TK-006";
    t6.title = "会計明細";
    t6.summary = "完了済み案件の監査ログ再確認";
    t6.type = "監査結果札";
    t6.priority = TicketPriority::normal;
    t6.status = TicketStatus::DONE;
    t6.assignedAgentId = "A-201";
    t6.auditResult = AuditResult::PASS;
    t6.auditCategory = "format_error";
    t6.createdAt = utcNow();
    t6.updatedAt = utcNow();

    m_tickets = {t1, t2, t3, t4, t5, t6};
    for (auto& ticket : m_tickets) {
        const QStringList route = routeForTicket(ticket);
        int phase = 0;
        if (ticket.status == TicketStatus::WAITING) {
            phase = 0;
        } else if (ticket.status == TicketStatus::ASSIGNED) {
            phase = 1;
        } else if (ticket.status == TicketStatus::PROCESSING) {
            phase = 3;
        } else if (ticket.status == TicketStatus::AUDIT_PENDING) {
            phase = 4;
        } else if (ticket.status == TicketStatus::AUDIT_PASSED || ticket.status == TicketStatus::ERROR) {
            phase = 5;
        } else if (ticket.status == TicketStatus::DONE) {
            phase = 6;
        }
        m_phaseByTicket.insert(ticket.id, phase);
        syncTicketDepartmentIfNeeded(ticket, qMin(phase, route.size() - 1));
        if (phase == 6) {
            pushHistory(ticket, "監査済み: 帰陣完了", false);
        }
    }

    pushAuditLog("TK-004", "A-201", AuditResult::PASS, "format_error", "最終監査:形式確認完了", "次工程へ進めて良い", true, "08_検査部");
    pushAuditLog("TK-005", "A-201", AuditResult::WARN, "scope_deviation", "監査で仕様逸脱を検知", "再処理要", false, "08_検査部");
    pushAuditLog("TK-006", "A-201", AuditResult::PASS, "requirement_missing", "完了済み監査結果", "監査保存完了", true, "03_監査室");

    pushHistory(m_tickets.last(), "監査結果が通過し処理終了", false);
    updateDepartmentStatus();
}

QVector<Ticket> MockEngineAdapter::getTickets() const {
    return m_tickets;
}

std::optional<Ticket> MockEngineAdapter::getTicketById(const QString& id) const {
    for (const auto& ticket : m_tickets) {
        if (ticket.id == id) {
            return ticket;
        }
    }
    return std::nullopt;
}

QVector<Agent> MockEngineAdapter::getAgents() const {
    return m_agents;
}

QVector<AuditLog> MockEngineAdapter::getAuditLogs() const {
    return m_auditLogs;
}

QVector<HistoryEntry> MockEngineAdapter::getHistory() const {
    return m_history;
}

QVector<Department> MockEngineAdapter::getDepartments() const {
    return m_departments;
}

QVector<DepartmentEvent> MockEngineAdapter::getDepartmentEvents() const {
    return m_departmentEvents;
}

QVector<DepartmentRecord> MockEngineAdapter::getDepartmentRecords() const {
    return m_departmentRecords;
}

SystemStatus MockEngineAdapter::getSystemStatus() const {
    return m_systemStatus;
}

void MockEngineAdapter::pushAuditLog(const QString& ticketId, const QString& agentId, AuditResult result, const QString& category,
                                    const QString& reason, const QString& action, bool retryable, const QString& departmentId) {
    m_auditLogs.prepend({QString("L-%1").arg(m_auditLogs.size() + 1, 4, 10, QLatin1Char('0')),
                         ticketId,
                         agentId,
                         departmentId,
                         result,
                         category,
                         reason,
                         action,
                         utcNow(),
                         retryable});
    if (m_auditLogs.size() > 300) {
        m_auditLogs.resize(300);
    }
}

void MockEngineAdapter::pushHistory(const Ticket& ticket, const QString& processResult, bool hasError) {
    m_history.prepend({ticket.id,
                       ticket.summary,
                       ticket.assignedAgentId,
                       processResult,
                       ticket.auditResult.value_or(AuditResult::PASS),
                       "00:00:31",
                       utcNow(),
                       hasError});
    if (m_history.size() > 300) {
        m_history.resize(300);
    }
}

void MockEngineAdapter::updateDepartmentStatus() {
    for (auto& d : m_departments) {
        d.activeTickets = 0;
        d.completedTickets = 0;
        d.auditWarnings = 0;
        d.inspectionWarnings = 0;
        d.recordCount = 0;
    }

    QHash<QString, int> deptIndex;
    for (int i = 0; i < m_departments.size(); ++i) {
        deptIndex.insert(m_departments[i].id, i);
    }

    for (const auto& ticket : m_tickets) {
        const int idx = deptIndex.value(ticket.currentDepartmentId, -1);
        if (idx >= 0) {
            if (ticket.status == TicketStatus::DONE) {
                ++m_departments[idx].completedTickets;
            } else {
                ++m_departments[idx].activeTickets;
            }
        }
        if (ticket.currentDepartmentId == "08_検査部" && ticket.auditResult.has_value() &&
            (ticket.auditResult.value() == AuditResult::WARN || ticket.auditResult.value() == AuditResult::REJECT)) {
            if (idx >= 0) {
                ++m_departments[idx].inspectionWarnings;
            }
        }
        if (ticket.currentDepartmentId == "03_監査室" && ticket.auditResult.has_value() &&
            (ticket.auditResult.value() == AuditResult::ESCALATE || ticket.auditResult.value() == AuditResult::SYSTEM_ERROR ||
             ticket.auditResult.value() == AuditResult::REJECT)) {
            if (idx >= 0) {
                ++m_departments[idx].auditWarnings;
            }
        }
    }

    for (const auto& record : m_departmentRecords) {
        const int idx = deptIndex.value(record.departmentId, -1);
        if (idx >= 0) {
            ++m_departments[idx].recordCount;
        }
    }
}

void MockEngineAdapter::recalcSystemStatus() {
    m_systemStatus = {"running", m_systemStatus.uptimeSeconds, 0, 0, 0, 0, 0, 0};
    for (const auto& ticket : m_tickets) {
        switch (ticket.status) {
            case TicketStatus::WAITING:
            case TicketStatus::ASSIGNED:
                ++m_systemStatus.waitingTickets;
                break;
            case TicketStatus::PROCESSING:
                ++m_systemStatus.processingTickets;
                break;
            case TicketStatus::AUDIT_PENDING:
                ++m_systemStatus.auditPendingTickets;
                break;
            case TicketStatus::AUDIT_REJECTED:
                ++m_systemStatus.rejectedTickets;
                break;
            case TicketStatus::DONE:
                ++m_systemStatus.completedTickets;
                break;
            case TicketStatus::ERROR:
                ++m_systemStatus.errorTickets;
                break;
            default:
                break;
        }
    }
}

void MockEngineAdapter::advanceTicketFlow(Ticket& ticket, int step) {
    int phase = m_phaseByTicket.value(ticket.id, 0);
    phase = (phase + 1 + (step % 2)) % 7;
    m_phaseByTicket[ticket.id] = phase;
    const QStringList route = routeForTicket(ticket);

    switch (phase) {
        case 0:
            if (ticket.status != TicketStatus::WAITING) {
                ticket.status = TicketStatus::WAITING;
                ticket.assignedAgentId.clear();
                ticket.auditResult.reset();
                ticket.auditCategory.clear();
            }
            moveToDepartment(ticket, route.value(0), true);
            break;
        case 1:
            ticket.status = TicketStatus::ASSIGNED;
            ticket.assignedAgentId = "A-201";
            moveToDepartment(ticket, route.value(1), true);
            ticket.auditCategory = "tool_mismatch";
            break;
        case 2:
            ticket.status = TicketStatus::PROCESSING;
            moveToDepartment(ticket, route.value(2), true);
            break;
        case 3: {
            ticket.status = TicketStatus::PROCESSING;
            moveToDepartment(ticket, route.value(3), true);
            break;
        }
        case 4:
            ticket.status = TicketStatus::AUDIT_PENDING;
            moveToDepartment(ticket, route.value(4), true);
            break;
        case 5: {
            const QString auditDestination = route.value(route.size() - 2);
            if (step % 3 == 0) {
                const QString fallback = route.value(2);
                emitDepartmentEvent(ticket.id, ticket.currentDepartmentId, fallback, DepartmentEventType::Audit,
                                   "監査で差戻し、戦略室へ戻る");
                ticket.status = TicketStatus::PROCESSING;
                ticket.auditResult = std::nullopt;
                ticket.auditCategory.clear();
                moveToDepartment(ticket, fallback, false);
                emitDepartmentEvent(ticket.id, ticket.previousDepartmentId, ticket.currentDepartmentId, DepartmentEventType::Error,
                                   "要件不一致による再処理");
                pushAuditLog(ticket.id, ticket.assignedAgentId, AuditResult::REJECT, "scope_deviation", "要件逸脱: 再検査", "再振り分け", true, ticket.currentDepartmentId);
            } else {
                moveToDepartment(ticket, auditDestination, true);
                ticket.status = TicketStatus::AUDIT_PASSED;
                ticket.auditResult = AuditResult::PASS;
                emitDepartmentEvent(ticket.id, ticket.currentDepartmentId, auditDestination, DepartmentEventType::Audit,
                                   "監査室で監査完了");
                pushAuditLog(ticket.id, ticket.assignedAgentId, AuditResult::PASS, "requirement_missing", "監査通過", "処理継続", true, "03_監査室");
                pushHistory(ticket, "監査済み: 帰陣準備", false);
            }
            break;
        }
        case 6:
            moveToDepartment(ticket, route.value(route.size() - 1), true);
            ticket.status = TicketStatus::DONE;
            ticket.auditResult = AuditResult::PASS;
            pushHistory(ticket, "監査済み: 帰陣完了", false);
            emitDepartmentEvent(ticket.id, ticket.previousDepartmentId, ticket.currentDepartmentId, DepartmentEventType::Record,
                               "庶務課で記録確定");
            break;
        default:
            break;
    }

    ticket.updatedAt = utcNow();
    updateDepartmentStatus();
}

void MockEngineAdapter::tick() {
    ++m_tick;
    if (m_tick % 2 != 0) {
        m_systemStatus.uptimeSeconds += 5;
        return;
    }

    for (auto& ticket : m_tickets) {
        int phase = m_phaseByTicket.value(ticket.id, 0);
        if (phase >= 6) {
            continue;
        }
        advanceTicketFlow(ticket, m_tick);
        syncTicketDepartmentIfNeeded(ticket, phase);
    }

    recalcSystemStatus();
}

} // namespace nexspace
