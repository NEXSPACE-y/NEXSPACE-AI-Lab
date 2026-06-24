#include "department_flow_engine.h"
#include "department_registry.h"
#include <QRegularExpression>

namespace nexspace {

namespace {

QString utcNow() {
    return QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss");
}

QString shorten(const QString& text, int maxLength) {
    return text.size() <= maxLength ? text : text.left(maxLength) + "…";
}

QString joinRouteHistory(const QVector<QString>& items) {
    QStringList values;
    values.reserve(items.size());
    for (const auto& item : items) {
        values.append(item);
    }
    return values.join("→");
}

} // namespace

DepartmentFlowEngine::DepartmentFlowEngine() {
    m_info = {EngineAdapterType::Mock, true, "MockDepartmentEngineAdapter"};
    registerDepartmentRegistry();
    seed();
    m_nextTicketNumber = nextTicketNumberFromExisting();
    recalcSystemStatus();
    updateDepartmentStatus();
}

EngineAdapterInfo DepartmentFlowEngine::info() const {
    return m_info;
}

void DepartmentFlowEngine::registerDepartmentRegistry() {
    m_departments = DepartmentRegistry::allDepartments();
}

QStringList DepartmentFlowEngine::routeForTicket(const Ticket& ticket) const {
    return DepartmentRegistry::routeForTicket(ticket);
}

QStringList DepartmentFlowEngine::buildInitialRoute(const Ticket& ticket) const {
    return routeForTicket(ticket);
}

void DepartmentFlowEngine::appendDepartmentHistory(Ticket& ticket, const QString& from, const QString& to) {
    Q_UNUSED(from)
    if (to.isEmpty()) {
        return;
    }
    if (ticket.routeHistory.isEmpty()) {
        ticket.routeHistory.append(to);
    } else if (ticket.routeHistory.last() != to) {
        ticket.routeHistory.append(to);
    }
    while (ticket.routeHistory.size() > 16) {
        ticket.routeHistory.removeFirst();
    }
}

void DepartmentFlowEngine::initializeTicketStatuses(Ticket& ticket) {
    if (ticket.audit_status.isEmpty()) {
        ticket.audit_status = toString(DepartmentWorkStatus::NOT_STARTED);
    }
    if (ticket.inspection_status.isEmpty()) {
        ticket.inspection_status = toString(DepartmentWorkStatus::NOT_STARTED);
    }
    if (ticket.record_status.isEmpty()) {
        ticket.record_status = toString(DepartmentWorkStatus::NOT_STARTED);
    }
    if (!ticket.currentDepartmentId.isEmpty() && ticket.record_status == toString(DepartmentWorkStatus::NOT_STARTED)
        && ticket.currentDepartmentId != "10_庶務課") {
        ticket.record_status = toString(DepartmentWorkStatus::IN_PROGRESS);
    }
}

void DepartmentFlowEngine::syncTicketDepartmentAliases(Ticket& ticket) {
    const QString resolvedCurrentDepartment = ticket.currentDepartmentId.isEmpty() ? ticket.current_department_id : ticket.currentDepartmentId;
    const QString resolvedPreviousDepartment = ticket.previousDepartmentId.isEmpty() ? ticket.previous_department_id : ticket.previousDepartmentId;
    const QString resolvedNextDepartment = ticket.nextDepartmentId.isEmpty() ? ticket.next_department_id : ticket.nextDepartmentId;
    const QVector<QString> preservedRouteHistory = ticket.routeHistory;
    const QVector<QString> preservedRouteHistoryLegacy = ticket.route_history;
    QVector<QString> mergedRouteHistory = preservedRouteHistory;
    if (mergedRouteHistory.isEmpty() && !preservedRouteHistoryLegacy.isEmpty()) {
        mergedRouteHistory.reserve(preservedRouteHistoryLegacy.size());
        for (const auto& item : preservedRouteHistoryLegacy) {
            mergedRouteHistory.append(item);
        }
    }

    ticket.currentDepartmentId = resolvedCurrentDepartment;
    ticket.current_department_id = resolvedCurrentDepartment;
    ticket.previousDepartmentId = resolvedPreviousDepartment;
    ticket.previous_department_id = resolvedPreviousDepartment;
    ticket.nextDepartmentId = resolvedNextDepartment;
    ticket.next_department_id = resolvedNextDepartment;
    ticket.routeHistory = mergedRouteHistory;
    ticket.route_history = mergedRouteHistory;

    if (ticket.audit_status.isEmpty()) {
        if (ticket.auditResult.has_value()) {
            ticket.audit_status = (ticket.auditResult.value() == AuditResult::PASS) ? toString(DepartmentWorkStatus::PASSED) : toString(DepartmentWorkStatus::WARN);
        } else {
            ticket.audit_status = toString(DepartmentWorkStatus::NOT_STARTED);
        }
    }
    if (ticket.inspection_status.isEmpty()) {
        ticket.inspection_status = toString(DepartmentWorkStatus::NOT_STARTED);
    }
    if (ticket.record_status.isEmpty()) {
        ticket.record_status = toString(DepartmentWorkStatus::NOT_STARTED);
    }
}

void DepartmentFlowEngine::initializeTicketDepartmentState(Ticket& ticket, int phase, const QStringList& route) {
    const QString resolvedCurrentDepartment = ticket.currentDepartmentId.isEmpty() ? ticket.current_department_id : ticket.currentDepartmentId;
    const QString resolvedPreviousDepartment = ticket.previousDepartmentId.isEmpty() ? ticket.previous_department_id : ticket.previousDepartmentId;
    const QString resolvedNextDepartment = ticket.nextDepartmentId.isEmpty() ? ticket.next_department_id : ticket.nextDepartmentId;

    if (!resolvedCurrentDepartment.isEmpty()) {
        ticket.currentDepartmentId = resolvedCurrentDepartment;
        ticket.current_department_id = resolvedCurrentDepartment;
        ticket.previousDepartmentId = resolvedPreviousDepartment;
        ticket.previous_department_id = resolvedPreviousDepartment;
        ticket.nextDepartmentId = resolvedNextDepartment;
        ticket.next_department_id = resolvedNextDepartment;
        if (!ticket.routeHistory.isEmpty()) {
            ticket.route_history = ticket.routeHistory;
        } else if (!ticket.route_history.isEmpty()) {
            ticket.routeHistory = ticket.route_history;
        }

        int index = route.indexOf(ticket.currentDepartmentId);
        if (ticket.nextDepartmentId.isEmpty() && index >= 0 && index + 1 < route.size()) {
            ticket.nextDepartmentId = route[index + 1];
        }
        initializeTicketStatuses(ticket);
        syncTicketDepartmentAliases(ticket);
        return;
    }

    ticket.currentDepartmentId = DepartmentRegistry::initialDepartmentForTicket(ticket);
    ticket.previousDepartmentId.clear();
    const int seedIndex = route.indexOf(ticket.currentDepartmentId);
    ticket.nextDepartmentId = (seedIndex >= 0 && seedIndex + 1 < route.size()) ? route[seedIndex + 1] : QString();
    ticket.routeHistory.clear();
    if (!ticket.currentDepartmentId.isEmpty()) {
        ticket.routeHistory.append(ticket.currentDepartmentId);
        emitDepartmentEvent(ticket, "票が受付部署へ到着", DepartmentEventType::TicketReceived, "received");
    }
    initializeTicketStatuses(ticket);
    syncTicketDepartmentAliases(ticket);
}

void DepartmentFlowEngine::emitDepartmentEvent(const Ticket& ticket, const QString& note, DepartmentEventType type, const QString& status) {
    QString fromDepartmentId = ticket.previousDepartmentId;
    if (fromDepartmentId.isEmpty()) {
        fromDepartmentId = QString("start");
    }

    const QString eventTimestamp = utcNow();
    const QString eventId = QString("D-%1").arg(m_departmentEventSeq++, 4, 10, QLatin1Char('0'));
    const QString aliasId = QString("D-%1").arg(m_departmentEventSeq + 10, 4, 10, QLatin1Char('0'));
    m_departmentEvents.prepend({eventId,
                               toString(type),
                               ticket.id,
                               ticket.currentDepartmentId,
                               fromDepartmentId,
                               ticket.currentDepartmentId,
                               type,
                               status,
                               note,
                               eventTimestamp,
                               eventTimestamp,
                               aliasId,
                               ticket.id,
                               note});
    if (m_departmentEvents.size() > 300) {
        m_departmentEvents.resize(300);
    }
}

void DepartmentFlowEngine::moveToDepartment(Ticket& ticket,
                                        const QString& nextDepartmentId,
                                        bool checkRouteViolation,
                                        DepartmentEventType eventType,
                                        const QString& eventStatus,
                                        int step) {
    if (ticket.currentDepartmentId == nextDepartmentId) {
        return;
    }

    const QStringList route = routeForTicket(ticket);
    if (checkRouteViolation) {
        if (!DepartmentRegistry::isValidTransition(ticket.currentDepartmentId, nextDepartmentId, route)) {
            return;
        }
    }

    const QString fromDepartmentId = ticket.currentDepartmentId;
    emitDepartmentEvent(ticket,
                        QString("%1 -> %2").arg(fromDepartmentId, nextDepartmentId),
                        eventType,
                        eventStatus);

    ticket.previousDepartmentId = ticket.currentDepartmentId;
    ticket.currentDepartmentId = nextDepartmentId;

    appendDepartmentHistory(ticket, fromDepartmentId, ticket.currentDepartmentId);

    const int nextIndex = route.indexOf(ticket.currentDepartmentId);
    if (nextIndex >= 0 && nextIndex + 1 < route.size()) {
        ticket.nextDepartmentId = route[nextIndex + 1];
    } else {
        ticket.nextDepartmentId.clear();
    }
    syncTicketDepartmentAliases(ticket);

    if (ticket.currentDepartmentId == "08_検査部") {
        emitDepartmentEvent(ticket, "検査部での査定を開始", DepartmentEventType::InspectionStarted, "started");
        ticket.inspection_status = toString(DepartmentWorkStatus::IN_PROGRESS);
        evaluateInspectionAndAdvance(ticket, route, step);
    } else if (ticket.currentDepartmentId == "03_監査室") {
        emitDepartmentEvent(ticket, "監査室での監査を開始", DepartmentEventType::AuditStarted, "started");
        ticket.audit_status = toString(DepartmentWorkStatus::IN_PROGRESS);
    } else if (ticket.currentDepartmentId == "10_庶務課") {
        appendRecordSummary(ticket.id, ticket.currentDepartmentId,
                            QString("署名済み: %1, 履歴=%2")
                                .arg(ticket.id)
                                .arg(joinRouteHistory(ticket.routeHistory)));
    }
}

void DepartmentFlowEngine::appendRecordSummary(const QString& ticketId, const QString& departmentId, const QString& summary) {
    m_departmentRecords.prepend({QString("R-%1").arg(m_departmentRecordSeq++, 4, 10, QLatin1Char('0')),
                                 ticketId,
                                 departmentId,
                                 shorten(summary, 120),
                                 utcNow()});
    if (m_departmentRecords.size() > 300) {
        m_departmentRecords.resize(300);
    }
}

void DepartmentFlowEngine::appendAuditSummary(const QString& ticketId,
                                          const QString& targetDepartmentId,
                                          const QString& result,
                                          const QStringList& flags,
                                          const ViolationLevel level,
                                          const QString& requiredAction) {
    m_compressedAudits.prepend({result, flags, toString(level), requiredAction, ticketId, targetDepartmentId});
    if (m_compressedAudits.size() > 300) {
        m_compressedAudits.resize(300);
    }
}

void DepartmentFlowEngine::pushAuditLog(const QString& ticketId, const QString& agentId, AuditResult result, const QString& category,
                                    const QString& reason, const QString& action, bool retryable, const QString& departmentId) {
    const QString joinedFlags = category.isEmpty() ? "normal" : category;
    m_auditLogs.prepend({QString("L-%1").arg(m_auditLogs.size() + 1, 4, 10, QLatin1Char('0')),
                         ticketId,
                         agentId,
                         departmentId,
                         result,
                         category,
                         reason,
                         action,
                         utcNow(),
                         retryable,
                         joinedFlags,
                         toString(ViolationLevel::low),
                         action,
                         departmentId});
    if (m_auditLogs.size() > 300) {
        m_auditLogs.resize(300);
    }
}

void DepartmentFlowEngine::evaluateInspectionAndAdvance(Ticket& ticket, const QStringList& route, int step) {
    Q_UNUSED(route)

    if (ticket.currentDepartmentId != "08_検査部") {
        return;
    }

    const bool hasWarning = (step % 4) == 1;
    if (hasWarning) {
        ticket.inspection_status = toString(DepartmentWorkStatus::WARN);
        emitDepartmentEvent(ticket, "検査で軽微な注意を検知", DepartmentEventType::InspectionCompleted, "warn");
    } else {
        ticket.inspection_status = toString(DepartmentWorkStatus::PASSED);
        emitDepartmentEvent(ticket, "検査合格", DepartmentEventType::InspectionCompleted, "passed");
    }
}

void DepartmentFlowEngine::evaluateAuditAndAdvance(Ticket& ticket, const QStringList& route, int step) {
    Q_UNUSED(route)

    if (ticket.currentDepartmentId != "03_監査室") {
        return;
    }

    const bool alreadyRejected = m_auditRejectedBefore.value(ticket.id, false);
    const bool auditReject = !alreadyRejected && ((step % 3 == 0) || ticket.id.endsWith("005"));
    if (auditReject) {
        m_auditRejectedBefore.insert(ticket.id, true);
        ticket.audit_status = toString(DepartmentWorkStatus::REJECTED);
        ticket.status = TicketStatus::AUDIT_REJECTED;
        ticket.auditResult = AuditResult::REJECT;
        ticket.auditCategory = "scope_deviation";
        emitDepartmentEvent(ticket, "監査NG: 仕様逸脱", DepartmentEventType::AuditCompleted, "rejected");
        const QString auditedDepartment = ticket.previousDepartmentId.isEmpty() ? ticket.currentDepartmentId : ticket.previousDepartmentId;
        appendAuditSummary(ticket.id, auditedDepartment, "rejected", {"scope_deviation", "unapproved_change"}, ViolationLevel::high, "仕様逸脱のため再処理");
        appendRecordSummary(ticket.id, "03_監査室", "監査NGにより差戻し");
        pushAuditLog(ticket.id, ticket.assignedAgentId, AuditResult::REJECT, "scope_deviation", "要件逸脱あり", "再審査へ戻す", true, "03_監査室");
        moveToDepartment(ticket, "04_戦略室", false, DepartmentEventType::TicketRejected, "rejected");
        emitDepartmentEvent(ticket, "差戻し済みのため再処理に戻す", DepartmentEventType::TicketRejected, "returned");
        m_phaseByTicket.insert(ticket.id, 3);
        ticket.status = TicketStatus::PROCESSING;
    } else {
        if (alreadyRejected) {
            m_auditRejectedBefore.remove(ticket.id);
        }
        ticket.audit_status = toString(DepartmentWorkStatus::PASSED);
        ticket.status = TicketStatus::AUDIT_PASSED;
        ticket.auditResult = AuditResult::PASS;
        emitDepartmentEvent(ticket, "監査完了", DepartmentEventType::AuditCompleted, "passed");
        const QString auditedDepartment = ticket.previousDepartmentId.isEmpty() ? ticket.currentDepartmentId : ticket.previousDepartmentId;
        appendAuditSummary(ticket.id, auditedDepartment, "passed", {"requirement_match", "format_ok"}, ViolationLevel::none, "庶務課へ記録して完了");
        pushAuditLog(ticket.id, ticket.assignedAgentId, AuditResult::PASS, "requirement_match", "監査通過", "次工程へ進めて良い", false, "03_監査室");
        pushHistory(ticket, "検査・監査を通過し最終記録待ち", false);
        m_phaseByTicket.insert(ticket.id, 6);
    }
}

void DepartmentFlowEngine::syncTicketDepartmentIfNeeded(Ticket& ticket, int phase, const QStringList& route) {
    initializeTicketDepartmentState(ticket, phase, route);
}

void DepartmentFlowEngine::seed() {
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
    t2.type = "営業文面";
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
            phase = 6;
        } else if (ticket.status == TicketStatus::DONE) {
            phase = 7;
        }
        m_phaseByTicket.insert(ticket.id, phase);
        syncTicketDepartmentIfNeeded(ticket, phase, route);
        if (ticket.status == TicketStatus::DONE) {
            pushHistory(ticket, "監査済み: 帰陣完了", false);
            appendRecordSummary(ticket.id, "10_庶務課", QString("%1 を記録アーカイブ済み").arg(ticket.id));
            ticket.record_status = toString(DepartmentWorkStatus::COMPLETED);
        }
    }

    pushAuditLog("TK-004", "A-201", AuditResult::PASS, "format_error", "最終監査:形式確認完了", "次工程へ進めて良い", true, "08_検査部");
    pushAuditLog("TK-005", "A-201", AuditResult::REJECT, "scope_deviation", "監査で仕様逸脱を検知", "再検査あり", false, "08_検査部");
    pushAuditLog("TK-006", "A-201", AuditResult::PASS, "requirement_matching", "完了済み監査結果", "監査保存完了", true, "03_監査室");

    appendAuditSummary("TK-004", "03_監査室", "passed", {"requirement_match"}, ViolationLevel::none, "最終承認");
    appendAuditSummary("TK-005", "03_監査室", "rejected", {"scope_deviation"}, ViolationLevel::high, "再処理へ戻す");

    pushHistory(m_tickets.last(), "監査結果が通過し処理終了", false);
    updateDepartmentStatus();
}

QVector<Ticket> DepartmentFlowEngine::getTickets() const {
    QVector<Ticket> clonedTickets = m_tickets;
    for (auto& ticket : clonedTickets) {
        synchronizeDepartmentAliases(ticket);
    }
    return clonedTickets;
}

std::optional<Ticket> DepartmentFlowEngine::getTicketById(const QString& id) const {
    for (const auto& ticket : m_tickets) {
        if (ticket.id == id) {
            auto ticketCopy = ticket;
            synchronizeDepartmentAliases(ticketCopy);
            return ticketCopy;
        }
    }
    return std::nullopt;
}

int DepartmentFlowEngine::nextTicketNumberFromExisting() const {
    int maxSeq = 0;
    static const QRegularExpression idPattern(QStringLiteral("^TK-(\\d+)$"));
    for (const auto& ticket : m_tickets) {
        const QRegularExpressionMatch match = idPattern.match(ticket.id);
        if (!match.hasMatch()) {
            continue;
        }
        const int seq = match.captured(1).toInt();
        if (seq > maxSeq) {
            maxSeq = seq;
        }
    }
    return maxSeq + 1;
}

Ticket DepartmentFlowEngine::buildTicketFromCommand(const CreateTicketInput& input) {
    const QString trimmedText = input.commandText.trimmed();
    const QString sourceTitle = input.title.value_or(QString());

    Ticket ticket;
    ticket.id = QString("TK-%1").arg(m_nextTicketNumber++, 3, 10, QLatin1Char('0'));
    ticket.title = sourceTitle.isEmpty() ? trimmedText.left(48) : sourceTitle;
    ticket.summary = trimmedText;
    ticket.type = input.type.isEmpty() ? QStringLiteral("依頼札") : input.type;
    ticket.priority = input.priority;
    ticket.status = TicketStatus::WAITING;
    ticket.assignedAgentId.clear();
    ticket.auditResult.reset();
    ticket.auditCategory.clear();
    ticket.createdAt = utcNow();
    ticket.updatedAt = ticket.createdAt;

    const QStringList route = routeForTicket(ticket);
    initializeTicketDepartmentState(ticket, 0, route);
    return ticket;
}

std::optional<Ticket> DepartmentFlowEngine::createTicketFromCommand(const CreateTicketInput& input) {
    if (input.commandText.trimmed().isEmpty()) {
        return std::nullopt;
    }

    Ticket ticket = buildTicketFromCommand(input);
    m_phaseByTicket.insert(ticket.id, 0);
    m_tickets.prepend(ticket);
    recalcSystemStatus();
    updateDepartmentStatus();
    return ticket;
}

QVector<Agent> DepartmentFlowEngine::getAgents() const {
    return m_agents;
}

QVector<AuditLog> DepartmentFlowEngine::getAuditLogs() const {
    return m_auditLogs;
}

QVector<HistoryEntry> DepartmentFlowEngine::getHistory() const {
    return m_history;
}

QVector<Department> DepartmentFlowEngine::getDepartments() const {
    return m_departments;
}

QVector<DepartmentEvent> DepartmentFlowEngine::getDepartmentEvents() const {
    return m_departmentEvents;
}

QVector<DepartmentRecord> DepartmentFlowEngine::getDepartmentRecords() const {
    return m_departmentRecords;
}

QVector<CompressedAuditTicket> DepartmentFlowEngine::getCompressedAuditTickets() const {
    return m_compressedAudits;
}

SystemStatus DepartmentFlowEngine::getSystemStatus() const {
    return m_systemStatus;
}

void DepartmentFlowEngine::pushHistory(const Ticket& ticket, const QString& processResult, bool hasError) {
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

void DepartmentFlowEngine::updateDepartmentStatus() {
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
            if (ticket.status == TicketStatus::DONE || ticket.status == TicketStatus::AUDIT_PASSED) {
                ++m_departments[idx].completedTickets;
            } else {
                ++m_departments[idx].activeTickets;
            }
        }
        if (ticket.currentDepartmentId == "08_検査部") {
            if (ticket.inspection_status == toString(DepartmentWorkStatus::WARN) ||
                ticket.inspection_status == toString(DepartmentWorkStatus::REJECTED)) {
                if (idx >= 0) {
                    ++m_departments[idx].inspectionWarnings;
                }
            }
        }
        if (ticket.currentDepartmentId == "03_監査室") {
            if (ticket.audit_status == toString(DepartmentWorkStatus::WARN) ||
                ticket.audit_status == toString(DepartmentWorkStatus::REJECTED)) {
                if (idx >= 0) {
                    ++m_departments[idx].auditWarnings;
                }
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

void DepartmentFlowEngine::recalcSystemStatus() {
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
            case TicketStatus::AUDIT_PASSED:
                ++m_systemStatus.completedTickets;
                break;
            case TicketStatus::ERROR:
                ++m_systemStatus.errorTickets;
                break;
        }
    }
}

void DepartmentFlowEngine::advanceTicketFlow(Ticket& ticket, int step) {
    int phase = m_phaseByTicket.value(ticket.id, 0);
    const QStringList route = routeForTicket(ticket);
    if (route.isEmpty()) {
        return;
    }

    switch (phase) {
        case 0:
            if (ticket.status != TicketStatus::WAITING) {
                ticket.status = TicketStatus::WAITING;
                ticket.assignedAgentId.clear();
                ticket.auditResult.reset();
                ticket.auditCategory.clear();
            }
            moveToDepartment(ticket, route.value(0), true, DepartmentEventType::TicketStarted, "started");
            ++phase;
            break;
        case 1:
            ticket.status = TicketStatus::ASSIGNED;
            ticket.assignedAgentId = "A-201";
            moveToDepartment(ticket, route.value(1), true, DepartmentEventType::TicketStarted, "started");
            emitDepartmentEvent(ticket, "処理担当を確定", DepartmentEventType::TicketStarted, "started");
            ticket.auditCategory = "tool_mismatch";
            ++phase;
            break;
        case 2:
            ticket.status = TicketStatus::PROCESSING;
            moveToDepartment(ticket, route.value(2), true, DepartmentEventType::TicketForwarded, "forwarded");
            ++phase;
            break;
        case 3:
            ticket.status = TicketStatus::PROCESSING;
            moveToDepartment(ticket, route.value(3), true, DepartmentEventType::TicketForwarded, "forwarded");
            ticket.auditCategory = "execution_started";
            ++phase;
            break;
        case 4: {
            ticket.status = TicketStatus::PROCESSING;
            moveToDepartment(ticket, route.value(4), true, DepartmentEventType::TicketForwarded, "forwarded", step);
            ++phase;
            break;
        }
        case 5:
            ticket.status = TicketStatus::AUDIT_PENDING;
            moveToDepartment(ticket, route.value(route.size() - 2), true, DepartmentEventType::TicketForwarded, "forwarded");
            evaluateAuditAndAdvance(ticket, route, step);
            phase = m_phaseByTicket.value(ticket.id, 6);
            break;
        case 6: {
            const QString finalDepartment = route.value(route.size() - 1);
            if (ticket.currentDepartmentId != finalDepartment) {
                moveToDepartment(ticket, finalDepartment, true, DepartmentEventType::TicketCompleted, "completed");
                ticket.status = TicketStatus::DONE;
                ticket.auditResult = AuditResult::PASS;
                ticket.audit_status = toString(DepartmentWorkStatus::COMPLETED);
                ticket.record_status = toString(DepartmentWorkStatus::IN_PROGRESS);
                emitDepartmentEvent(ticket, "庶務課へ記録送付", DepartmentEventType::RecordSaved, "saved");
                appendRecordSummary(ticket.id, finalDepartment, QString("最終成果を庶務課へ保存: %1").arg(ticket.summary));
                pushHistory(ticket, "監査済み: 帰陣完了", false);
                ++phase;
            } else {
                ++phase;
            }
            break;
        }
        case 7: {
            ticket.status = TicketStatus::DONE;
            ticket.record_status = toString(DepartmentWorkStatus::COMPLETED);
            appendRecordSummary(ticket.id, "10_庶務課", QString("入力/経路/検査/監査/結果要約: %1").arg(joinRouteHistory(ticket.routeHistory)));
            emitDepartmentEvent(ticket, "処理履歴をアーカイブ", DepartmentEventType::RecordSaved, "archived");
            ++phase;
            break;
        }
        default:
            break;
    }

    ticket.updatedAt = utcNow();
    m_phaseByTicket.insert(ticket.id, phase);
    recalcSystemStatus();
    updateDepartmentStatus();
}

void DepartmentFlowEngine::tick() {
    ++m_tick;
    if (m_tick % 2 != 0) {
        m_systemStatus.uptimeSeconds += 5;
        return;
    }

    for (auto& ticket : m_tickets) {
        const int phase = m_phaseByTicket.value(ticket.id, 0);
        if (phase >= 8) {
            continue;
        }
        const QStringList route = routeForTicket(ticket);
        syncTicketDepartmentIfNeeded(ticket, phase, route);
        advanceTicketFlow(ticket, m_tick);
        syncTicketDepartmentIfNeeded(ticket, phase, route);
        if (ticket.status == TicketStatus::AUDIT_REJECTED) {
            ++m_systemStatus.rejectedTickets;
        }
    }

    recalcSystemStatus();
    updateDepartmentStatus();
}

} // namespace nexspace
