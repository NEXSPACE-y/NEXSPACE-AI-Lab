#include "mock_engine_adapter.h"

namespace nexspace {

namespace {
QString utcNow() {
    return QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss");
}
}

MockEngineAdapter::MockEngineAdapter() {
    m_info = {EngineAdapterType::Mock, true, "MockEngineAdapter"};
    seed();
    recalcSystemStatus();
}

EngineAdapterInfo MockEngineAdapter::info() const {
    return m_info;
}

void MockEngineAdapter::seed() {
    Agent orchestrator = {"A-000", "上杉", AgentRole::Orchestrator, "running", 0, 0, 0, 0, utcNow()};
    Agent worker1 = {"A-101", "柴田", AgentRole::Worker, "running", 2, 12, 1, 0, utcNow()};
    Agent worker2 = {"A-102", "福島", AgentRole::Worker, "idle", 0, 8, 0, 0, utcNow()};
    Agent auditor = {"A-201", "観測官", AgentRole::Auditor, "running", 1, 6, 3, 1, utcNow()};
    Agent observer = {"A-301", "物見", AgentRole::Observer, "running", 0, 4, 0, 0, utcNow()};
    Agent system = {"SYS", "本陣", AgentRole::System, "running", 0, 30, 0, 0, utcNow()};
    m_agents = {orchestrator, worker1, worker2, auditor, observer, system};

    m_tickets = {
        {"TK-001", "予算異動依頼", "予算明細の修正指示を生成し、監査を通過後に保存する", "依頼札", TicketPriority::high, TicketStatus::WAITING, "", std::nullopt, "", utcNow(), utcNow()},
        {"TK-002", "顧客連絡文面", "契約条項を崩さない対外文面の下処理", "業務連絡札", TicketPriority::normal, TicketStatus::PROCESSING, "A-101", std::nullopt, "", utcNow(), utcNow()},
        {"TK-003", "内部調整依頼", "監査保留案件の説明追加", "割当札", TicketPriority::critical, TicketStatus::AUDIT_PENDING, "A-102", std::nullopt, "", utcNow(), utcNow()},
        {"TK-004", "報告文整形", "最終監査パスが必須の文言整形", "処理後札", TicketPriority::low, TicketStatus::AUDIT_PASSED, "A-101", AuditResult::PASS, "requirement_missing", utcNow(), utcNow()},
        {"TK-005", "外部API連携", "形式不備の可能性がある返答を再検査", "最終報告札", TicketPriority::critical, TicketStatus::AUDIT_REJECTED, "A-201", AuditResult::REJECT, "scope_deviation", utcNow(), utcNow()},
        {"TK-006", "会計明細", "完了済み案件の監査ログ再確認", "監査結果札", TicketPriority::normal, TicketStatus::DONE, "A-201", AuditResult::PASS, "format_error", utcNow(), utcNow()}
    };

    m_phaseByTicket = {
        {"TK-001", 0},
        {"TK-002", 2},
        {"TK-003", 3},
        {"TK-004", 5},
        {"TK-005", 4},
        {"TK-006", 6}
    };

    pushAuditLog("TK-004", "A-201", AuditResult::PASS, "format_error", "最終監査:形式確認完了", "次工程へ進めて良い", true);
    pushAuditLog("TK-005", "A-201", AuditResult::WARN, "scope_deviation", "監査で仕様逸脱を検知", "再処理要", false);
    pushAuditLog("TK-006", "A-201", AuditResult::PASS, "requirement_missing", "完了済み監査結果", "監査保存完了", true);

    pushHistory(m_tickets.last(), "監査結果が通過し処理終了", false);
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

SystemStatus MockEngineAdapter::getSystemStatus() const {
    return m_systemStatus;
}

void MockEngineAdapter::pushAuditLog(const QString& ticketId, const QString& agentId, AuditResult result, const QString& category,
                                    const QString& reason, const QString& action, bool retryable) {
    m_auditLogs.prepend({QString("L-%1").arg(m_auditLogs.size() + 1, 4, 10, QLatin1Char('0')),
                         ticketId,
                         agentId,
                         result,
                         category,
                         reason,
                         action,
                         utcNow(),
                         retryable});
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

    switch (phase) {
        case 0:
            if (ticket.status != TicketStatus::WAITING) {
                ticket.status = TicketStatus::WAITING;
                ticket.assignedAgentId.clear();
                ticket.auditResult.reset();
                ticket.auditCategory.clear();
            }
            break;
        case 1:
            ticket.status = TicketStatus::ASSIGNED;
            ticket.assignedAgentId = "A-101";
            ticket.auditCategory = "tool_mismatch";
            break;
        case 2:
            ticket.status = TicketStatus::PROCESSING;
            ticket.assignedAgentId = "A-101";
            break;
        case 3:
            ticket.status = TicketStatus::AUDIT_PENDING;
            break;
        case 4:
            if (step % 3 == 0) {
                ticket.status = TicketStatus::AUDIT_REJECTED;
                ticket.auditResult = AuditResult::REJECT;
                ticket.auditCategory = "scope_deviation";
                pushAuditLog(ticket.id, ticket.assignedAgentId, AuditResult::REJECT, ticket.auditCategory,
                            "仕様逸脱: 形式に起点がある指示を検出", "差戻し実施", true);
                pushHistory(ticket, "仕様逸脱で差戻し", true);
            } else {
                ticket.status = TicketStatus::AUDIT_PASSED;
                ticket.auditResult = AuditResult::PASS;
                ticket.auditCategory = "requirement_missing";
                pushAuditLog(ticket.id, ticket.assignedAgentId, AuditResult::PASS, ticket.auditCategory,
                            "監査通過", "処理継続", true);
            }
            break;
        case 5:
            if (ticket.status == TicketStatus::AUDIT_REJECTED) {
                ticket.status = TicketStatus::PROCESSING;
                ticket.auditResult = std::nullopt;
                ticket.auditCategory.clear();
            } else {
                ticket.status = TicketStatus::DONE;
                ticket.auditResult = AuditResult::PASS;
                pushHistory(ticket, "監査済み: 帰陣完了", false);
            }
            break;
        case 6:
            ticket.status = TicketStatus::DONE;
            break;
        default:
            break;
    }

    ticket.updatedAt = utcNow();
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
    }
    recalcSystemStatus();
}

} // namespace nexspace
