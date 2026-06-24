#include "department_registry.h"

namespace nexspace {

namespace {

Department buildDepartmentFromDef(const QString& id,
                                 const QString& name,
                                 const QString& parentId,
                                 bool isParent,
                                 const QStringList& children,
                                 const QString& role,
                                 const QStringList& allowedInput,
                                 const QStringList& allowedOutput,
                                 const QStringList& forbiddenActions,
                                 bool auditRequired,
                                 bool inspectionRequired,
                                 bool recordRequired) {
    Department department;
    department.id = id;
    department.name = name;
    department.parentId = parentId;
    department.isParent = isParent;
    department.department_id = id;
    department.department_name = name;
    department.parent_department_id = parentId;
    department.child_department_ids = children;
    department.department_role = role;
    department.allowed_input_ticket_types = allowedInput;
    department.allowed_output_ticket_types = allowedOutput;
    department.forbidden_actions = forbiddenActions;
    department.audit_required = auditRequired;
    department.inspection_required = inspectionRequired;
    department.record_required = recordRequired;
    return department;
}

} // namespace

QVector<Department> DepartmentRegistry::buildDepartments() {
    return {
        buildDepartmentFromDef(
            "01_社長室", "社長室", QString(),
            true,
            {"02_秘書室"},
            "chief_decision",
            {"user_input", "policy_update", "strategic_request"},
            {"summary", "execution_plan"},
            {"unauthorized_escalation"},
            true,
            false,
            true),
        buildDepartmentFromDef(
            "02_秘書室", "秘書室", "01_社長室",
            false,
            {},
            "secretariat",
            {"user_input"},
            {"preprocess_ticket"},
            {"missing_input", "format_violation"},
            true,
            true,
            true),
        buildDepartmentFromDef(
            "03_監査室", "監査室", QString(),
            true,
            {"08_検査部"},
            "audit_control",
            {"evidence", "status_report", "all_department_summary"},
            {"audit_summary"},
            {"scope_deviation", "unauthorized_change", "forged_ticket"},
            true,
            true,
            true),
        buildDepartmentFromDef(
            "08_検査部", "検査部", "03_監査室",
            false,
            {},
            "inspection",
            {"draft", "execution_result"},
            {"inspection_summary"},
            {"insufficient_evidence", "format_mismatch"},
            false,
            true,
            true),
        buildDepartmentFromDef(
            "04_戦略室", "戦略室", QString(),
            true,
            {"05_制作室", "06_営業部"},
            "strategy",
            {"user_input", "review_request", "draft"},
            {"strategy_plan", "division_result"},
            {"unscheduled_journey", "unauthorized_bypass"},
            true,
            false,
            true),
        buildDepartmentFromDef(
            "05_制作室", "制作室", "04_戦略室",
            true,
            {"07_技術部"},
            "production",
            {"strategy_plan", "document_request"},
            {"production_output", "implementation_request"},
            {"invalid_spec", "out_of_scope"},
            true,
            true,
            true),
        buildDepartmentFromDef(
            "06_営業部", "営業部", "04_戦略室",
            false,
            {},
            "sales",
            {"production_output", "customer_feedback"},
            {"sales_document"},
            {"pricing_error", "unverified_claim"},
            true,
            true,
            true),
        buildDepartmentFromDef(
            "07_技術部", "技術部", "05_制作室",
            false,
            {},
            "engineering",
            {"production_output", "implementation_request"},
            {"implementation_output", "technical_review"},
            {"unsafe_action", "invalid_api"},
            true,
            true,
            true),
        buildDepartmentFromDef(
            "09_総務部", "総務部", QString(),
            true,
            {"10_庶務課"},
            "admin",
            {"policy", "rule_request"},
            {"governance_report"},
            {"rule_violation", "unapproved_exception"},
            false,
            false,
            true),
        buildDepartmentFromDef(
            "10_庶務課", "庶務課", "09_総務部",
            false,
            {},
            "records",
            {"department_summary", "audit_summary", "final_report"},
            {"record_archive"},
            {"uncompressed_log", "untracked_change"},
            false,
            false,
            true)
    };
}

const QVector<Department>& DepartmentRegistry::allDepartments() {
    static QVector<Department> departments = buildDepartments();
    return departments;
}

QStringList DepartmentRegistry::allDepartmentIds() {
    QStringList ids;
    for (const auto& d : allDepartments()) {
        ids.append(d.department_id);
    }
    return ids;
}

const Department* DepartmentRegistry::findDepartment(const QString& departmentId) {
    for (const auto& d : allDepartments()) {
        if (d.department_id == departmentId) {
            return &d;
        }
    }
    return nullptr;
}

QStringList DepartmentRegistry::routeForTicket(const Ticket& ticket) {
    return DepartmentRouter::routeForTicket(ticket);
}

QString DepartmentRegistry::initialDepartmentForTicket(const Ticket& ticket) {
    const QStringList route = DepartmentRouter::routeForTicket(ticket);
    return route.isEmpty() ? "02_秘書室" : route.first();
}

bool DepartmentRegistry::isValidTransition(const QString& fromDepartmentId,
                                          const QString& toDepartmentId,
                                          const QStringList& route) {
    return DepartmentRouter::isValidTransition(fromDepartmentId, toDepartmentId, route);
}

bool DepartmentRegistry::hasParent(const QString& departmentId) {
    const Department* d = findDepartment(departmentId);
    return d != nullptr && !d->parent_department_id.isEmpty();
}

bool DepartmentRegistry::canAuditDepartment(const QString& targetDepartmentId) {
    return findDepartment(targetDepartmentId) != nullptr;
}

} // namespace nexspace
