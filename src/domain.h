#pragma once

#include <optional>
#include <QString>
#include <QStringList>
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

enum class DepartmentWorkStatus {
    NOT_STARTED,
    IN_PROGRESS,
    PASSED,
    WARN,
    REJECTED,
    RETURNED,
    COMPLETED
};

enum class ViolationLevel {
    none,
    low,
    medium,
    high,
    critical
};

enum class DepartmentEventType {
    TicketReceived,
    TicketStarted,
    TicketCompleted,
    TicketRejected,
    TicketForwarded,
    InspectionStarted,
    InspectionCompleted,
    AuditStarted,
    AuditCompleted,
    RecordSaved,
    Error
};

struct Department {
    QString id;
    QString name;
    QString parentId;
    bool isParent = false;

    int activeTickets = 0;
    int completedTickets = 0;
    int auditWarnings = 0;
    int inspectionWarnings = 0;
    int recordCount = 0;

    QString department_id;
    QString department_name;
    QString parent_department_id;
    QStringList child_department_ids;
    QString department_role;
    QStringList allowed_input_ticket_types;
    QStringList allowed_output_ticket_types;
    QStringList forbidden_actions;
    bool audit_required = false;
    bool inspection_required = false;
    bool record_required = false;
};

struct DepartmentEvent {
    QString event_id;
    QString event_type;
    QString ticket_id;
    QString department_id;
    QString fromDepartmentId;
    QString toDepartmentId;
    DepartmentEventType type = DepartmentEventType::TicketForwarded;
    QString status;
    QString message_summary;
    QString timestamp;
    QString createdAt;

    QString id;
    QString ticketId;
    QString note;
};

struct DepartmentRecord {
    QString id;
    QString ticketId;
    QString departmentId;
    QString summary;
    QString createdAt;
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

    QString currentDepartmentId;
    QString previousDepartmentId;
    QString nextDepartmentId;
    QVector<QString> routeHistory;

    QString current_department_id;
    QString previous_department_id;
    QString next_department_id;
    QVector<QString> route_history;

    QString audit_status;
    QString inspection_status;
    QString record_status;

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
    QString targetDepartmentId;
    AuditResult result = AuditResult::PASS;
    QString category;
    QString reasonSummary;
    QString recommendedAction;
    QString createdAt;
    bool retryable = false;

    QString audit_flags;
    QString violation_level;
    QString required_action;
    QString target_department_id;
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

struct CompressedAuditTicket {
    QString audit_result;
    QStringList audit_flags;
    QString violation_level;
    QString required_action;
    QString target_ticket_id;
    QString target_department_id;
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

inline QString toJapaneseDepartment(const QString& departmentId) {
    if (departmentId == "01_社長室") return "社長室";
    if (departmentId == "02_秘書室") return "秘書室";
    if (departmentId == "03_監査室") return "監査室";
    if (departmentId == "04_戦略室") return "戦略室";
    if (departmentId == "05_制作室") return "制作室";
    if (departmentId == "06_営業部") return "営業部";
    if (departmentId == "07_技術部") return "技術部";
    if (departmentId == "08_検査部") return "検査部";
    if (departmentId == "09_総務部") return "総務部";
    if (departmentId == "10_庶務課") return "庶務課";
    return departmentId.isEmpty() ? "未割当" : departmentId;
}

inline QString toString(DepartmentEventType type) {
    switch (type) {
        case DepartmentEventType::TicketReceived:
            return "department.ticket.received";
        case DepartmentEventType::TicketStarted:
            return "department.ticket.started";
        case DepartmentEventType::TicketCompleted:
            return "department.ticket.completed";
        case DepartmentEventType::TicketRejected:
            return "department.ticket.rejected";
        case DepartmentEventType::TicketForwarded:
            return "department.ticket.forwarded";
        case DepartmentEventType::InspectionStarted:
            return "department.inspection.started";
        case DepartmentEventType::InspectionCompleted:
            return "department.inspection.completed";
        case DepartmentEventType::AuditStarted:
            return "department.audit.started";
        case DepartmentEventType::AuditCompleted:
            return "department.audit.completed";
        case DepartmentEventType::RecordSaved:
            return "department.record.saved";
        case DepartmentEventType::Error:
            return "department.error";
        default:
            return "department.ticket.forwarded";
    }
}

inline QString toJapaneseDepartmentEvent(DepartmentEventType type) {
    switch (type) {
        case DepartmentEventType::TicketReceived:
            return "受理";
        case DepartmentEventType::TicketStarted:
            return "開始";
        case DepartmentEventType::TicketCompleted:
            return "完了";
        case DepartmentEventType::TicketRejected:
            return "差戻し";
        case DepartmentEventType::TicketForwarded:
            return "部署遷移";
        case DepartmentEventType::InspectionStarted:
            return "検査開始";
        case DepartmentEventType::InspectionCompleted:
            return "検査完了";
        case DepartmentEventType::AuditStarted:
            return "監査開始";
        case DepartmentEventType::AuditCompleted:
            return "監査完了";
        case DepartmentEventType::RecordSaved:
            return "記録保存";
        case DepartmentEventType::Error:
            return "異常";
        default:
            return "部署遷移";
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

inline QString toString(DepartmentWorkStatus status) {
    switch (status) {
        case DepartmentWorkStatus::NOT_STARTED: return "not_started";
        case DepartmentWorkStatus::IN_PROGRESS: return "in_progress";
        case DepartmentWorkStatus::PASSED: return "passed";
        case DepartmentWorkStatus::WARN: return "warn";
        case DepartmentWorkStatus::REJECTED: return "rejected";
        case DepartmentWorkStatus::RETURNED: return "returned";
        case DepartmentWorkStatus::COMPLETED: return "completed";
        default: return "not_started";
    }
}

inline DepartmentWorkStatus parseWorkStatus(const QString& value) {
    if (value == "in_progress") return DepartmentWorkStatus::IN_PROGRESS;
    if (value == "passed") return DepartmentWorkStatus::PASSED;
    if (value == "warn") return DepartmentWorkStatus::WARN;
    if (value == "rejected") return DepartmentWorkStatus::REJECTED;
    if (value == "returned") return DepartmentWorkStatus::RETURNED;
    if (value == "completed") return DepartmentWorkStatus::COMPLETED;
    return DepartmentWorkStatus::NOT_STARTED;
}

inline QString toString(ViolationLevel level) {
    switch (level) {
        case ViolationLevel::none:
            return "none";
        case ViolationLevel::low:
            return "low";
        case ViolationLevel::medium:
            return "medium";
        case ViolationLevel::high:
            return "high";
        case ViolationLevel::critical:
            return "critical";
        default:
            return "none";
    }
}

inline ViolationLevel parseViolationLevel(const QString& value) {
    if (value == "low") return ViolationLevel::low;
    if (value == "medium") return ViolationLevel::medium;
    if (value == "high") return ViolationLevel::high;
    if (value == "critical") return ViolationLevel::critical;
    return ViolationLevel::none;
}

inline void synchronizeDepartmentAliases(Ticket& ticket) {
    ticket.currentDepartmentId = ticket.current_department_id.isEmpty() ? ticket.currentDepartmentId : ticket.current_department_id;
    ticket.previousDepartmentId = ticket.previous_department_id.isEmpty() ? ticket.previousDepartmentId : ticket.previous_department_id;
    ticket.nextDepartmentId = ticket.next_department_id.isEmpty() ? ticket.nextDepartmentId : ticket.next_department_id;
    ticket.routeHistory = ticket.route_history.isEmpty() ? ticket.routeHistory : ticket.route_history;

    if (!ticket.currentDepartmentId.isEmpty()) ticket.current_department_id = ticket.currentDepartmentId;
    if (!ticket.previousDepartmentId.isEmpty()) ticket.previous_department_id = ticket.previousDepartmentId;
    if (!ticket.nextDepartmentId.isEmpty()) ticket.next_department_id = ticket.nextDepartmentId;
    if (!ticket.routeHistory.isEmpty()) ticket.route_history = ticket.routeHistory;

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

} // namespace nexspace
