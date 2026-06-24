#pragma once

#include <optional>
#include <QString>
#include <QVector>

namespace nexspace {

enum class TicketStatus {
    WAITING,
    ASSIGNED,
    PROCESSING,
    AUDIT_PENDING,
    AUDIT_PASSED,
    AUDIT_REJECTED,
    DONE,
    ERROR
};

enum class AuditResult {
    PASS,
    WARN,
    REJECT,
    ESCALATE,
    SYSTEM_ERROR
};

enum class AgentRole {
    Orchestrator,
    Worker,
    Auditor,
    Observer,
    System
};

enum class TicketPriority {
    low,
    normal,
    high,
    critical
};

struct Ticket {
    QString id;
    QString title;
    QString summary;
    QString type;
    TicketPriority priority = TicketPriority::normal;
    TicketStatus status = TicketStatus::WAITING;
    QString assignedAgentId;
    std::optional<AuditResult> auditResult;
    QString auditCategory;
    QString createdAt;
    QString updatedAt;
};

struct Agent {
    QString id;
    QString name;
    AgentRole role = AgentRole::Worker;
    QString status;
    int activeTickets = 0;
    int completedTickets = 0;
    int auditWarnings = 0;
    int errorTickets = 0;
    QString lastActiveAt;
};

struct AuditLog {
    QString id;
    QString ticketId;
    QString targetAgentId;
    AuditResult result = AuditResult::PASS;
    QString category;
    QString reasonSummary;
    QString recommendedAction;
    QString createdAt;
    bool retryable = false;
};

struct HistoryEntry {
    QString ticketId;
    QString inputSummary;
    QString assignedAgentId;
    QString processResult;
    AuditResult auditResult = AuditResult::PASS;
    QString duration;
    QString updatedAt;
    bool hasError = false;
};

struct SystemStatus {
    QString status;
    int uptimeSeconds = 0;
    int waitingTickets = 0;
    int processingTickets = 0;
    int auditPendingTickets = 0;
    int rejectedTickets = 0;
    int completedTickets = 0;
    int errorTickets = 0;
};

inline QString toJapaneseStatus(TicketStatus status) {
    switch (status) {
        case TicketStatus::WAITING: return "待機";
        case TicketStatus::ASSIGNED: return "任命済";
        case TicketStatus::PROCESSING: return "出陣中";
        case TicketStatus::AUDIT_PENDING: return "監察待ち";
        case TicketStatus::AUDIT_PASSED: return "裁可";
        case TicketStatus::AUDIT_REJECTED: return "差戻し";
        case TicketStatus::DONE: return "帰陣";
        case TicketStatus::ERROR: return "異常";
        default: return "不明";
    }
}

inline QString toJapaneseAuditResult(AuditResult result) {
    switch (result) {
        case AuditResult::PASS: return "通過";
        case AuditResult::WARN: return "注意";
        case AuditResult::REJECT: return "却下";
        case AuditResult::ESCALATE: return "上申";
        case AuditResult::SYSTEM_ERROR: return "系統異常";
        default: return "未判定";
    }
}

inline QString toJapaneseRole(AgentRole role) {
    switch (role) {
        case AgentRole::Orchestrator: return "軍師";
        case AgentRole::Worker: return "実行隊";
        case AgentRole::Auditor: return "監察官";
        case AgentRole::Observer: return "物見";
        case AgentRole::System: return "本陣";
        default: return "不明";
    }
}

inline QString toString(AuditResult result) {
    switch (result) {
        case AuditResult::PASS: return "PASS";
        case AuditResult::WARN: return "WARN";
        case AuditResult::REJECT: return "REJECT";
        case AuditResult::ESCALATE: return "ESCALATE";
        case AuditResult::SYSTEM_ERROR: return "SYSTEM_ERROR";
        default: return "PASS";
    }
}

inline QString toString(TicketPriority priority) {
    switch (priority) {
        case TicketPriority::low: return "low";
        case TicketPriority::normal: return "normal";
        case TicketPriority::high: return "high";
        case TicketPriority::critical: return "critical";
        default: return "normal";
    }
}

inline QString toLabel(TicketPriority priority) {
    switch (priority) {
        case TicketPriority::low: return "低";
        case TicketPriority::normal: return "中";
        case TicketPriority::high: return "高";
        case TicketPriority::critical: return "最優先";
        default: return "中";
    }
}

inline QString toString(TicketStatus status) {
    switch (status) {
        case TicketStatus::WAITING: return "WAITING";
        case TicketStatus::ASSIGNED: return "ASSIGNED";
        case TicketStatus::PROCESSING: return "PROCESSING";
        case TicketStatus::AUDIT_PENDING: return "AUDIT_PENDING";
        case TicketStatus::AUDIT_PASSED: return "AUDIT_PASSED";
        case TicketStatus::AUDIT_REJECTED: return "AUDIT_REJECTED";
        case TicketStatus::DONE: return "DONE";
        case TicketStatus::ERROR: return "ERROR";
        default: return "WAITING";
    }
}

inline TicketStatus parseStatus(const QString& value) {
    if (value == "ASSIGNED") return TicketStatus::ASSIGNED;
    if (value == "PROCESSING") return TicketStatus::PROCESSING;
    if (value == "AUDIT_PENDING") return TicketStatus::AUDIT_PENDING;
    if (value == "AUDIT_PASSED") return TicketStatus::AUDIT_PASSED;
    if (value == "AUDIT_REJECTED") return TicketStatus::AUDIT_REJECTED;
    if (value == "DONE") return TicketStatus::DONE;
    if (value == "ERROR") return TicketStatus::ERROR;
    return TicketStatus::WAITING;
}

inline AuditResult parseAuditResult(const QString& value) {
    if (value == "PASS") return AuditResult::PASS;
    if (value == "WARN") return AuditResult::WARN;
    if (value == "REJECT") return AuditResult::REJECT;
    if (value == "ESCALATE") return AuditResult::ESCALATE;
    if (value == "SYSTEM_ERROR") return AuditResult::SYSTEM_ERROR;
    return AuditResult::PASS;
}

inline QString parseRole(AgentRole role) {
    switch (role) {
        case AgentRole::Orchestrator: return "Orchestrator";
        case AgentRole::Worker: return "Worker";
        case AgentRole::Auditor: return "Auditor";
        case AgentRole::Observer: return "Observer";
        case AgentRole::System: return "System";
        default: return "Worker";
    }
}

} // namespace nexspace
