#include "../部署構成/department_registry.h"
#include "../src/adapters/mock_engine_adapter.h"

#include <algorithm>
#include <QHash>
#include <QSet>
#include <QVector>
#include <cstdlib>
#include <iostream>

using namespace nexspace;

namespace {

void ensure(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "TEST FAILED: " << message << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

} // namespace

int main() {
    const auto& departments = DepartmentRegistry::allDepartments();
    ensure(departments.size() == 10, "部署レジストリは10部署であること");
    ensure(DepartmentRegistry::allDepartmentIds().size() == departments.size(), "部署ID一覧が重複していないこと");

    QSet<QString> seenIds;
    for (const auto& dept : departments) {
        ensure(!seenIds.contains(dept.department_id), "部署IDが重複していないこと");
        seenIds.insert(dept.department_id);
    }

    QHash<QString, QString> parentByChild;
    QHash<QString, Department> departmentById;
    for (const auto& dept : departments) {
        parentByChild.insert(dept.department_id, dept.parent_department_id);
        departmentById.insert(dept.department_id, dept);
    }

    ensure(parentByChild.value("02_秘書室") == "01_社長室", "秘書室の親が社長室であること");
    ensure(parentByChild.value("07_技術部") == "05_制作室", "技術部の親が制作室であること");
    ensure(parentByChild.value("06_営業部") == "04_戦略室", "営業部の親が戦略室であること");
    ensure(parentByChild.value("10_庶務課") == "09_総務部", "庶務課の親が総務部であること");

    ensure(departmentById.value("01_社長室").child_department_ids.contains("02_秘書室"), "社長室の子部署に秘書室があること");
    ensure(departmentById.value("03_監査室").child_department_ids.contains("08_検査部"), "監査室の子部署に検査部があること");
    ensure(departmentById.value("04_戦略室").child_department_ids.contains("05_制作室"), "戦略室の子部署に制作室があること");
    ensure(departmentById.value("04_戦略室").child_department_ids.contains("06_営業部"), "戦略室の子部署に営業部があること");
    ensure(departmentById.value("05_制作室").child_department_ids.contains("07_技術部"), "制作室の子部署に技術部があること");
    ensure(departmentById.value("09_総務部").child_department_ids.contains("10_庶務課"), "総務部の子部署に庶務課があること");
    for (const auto& dept : departments) {
        ensure(DepartmentRegistry::canAuditDepartment(dept.department_id), "監査室が各部署を監査対象にできること");
    }

    MockEngineAdapter adapter;
    CreateTicketInput input;
    input.commandText = "仕様の齟齬を確認する依頼";
    input.type = "技術依頼";
    const auto createdOpt = adapter.createTicketFromCommand(input);
    ensure(createdOpt.has_value(), "createTicketFromCommand が新規札を生成できること");
    const auto createdById = adapter.getTicketById(createdOpt.value().id);
    ensure(createdById.has_value(), "生成した新規札が検索可能であること");
    ensure(createdById.value().currentDepartmentId == "02_秘書室" || createdById.value().current_department_id == "02_秘書室",
           "作成API経由の新規札が秘書室から開始されること");
    ensure(!createdById.value().routeHistory.isEmpty() || !createdById.value().route_history.isEmpty(), "作成API経由の新規札に初期経路履歴が付与されること");

    const auto initialTicketOpt = adapter.getTicketById("TK-001");
    ensure(initialTicketOpt.has_value(), "シード札 TK-001 が存在すること");
    const auto initialTicket = initialTicketOpt.value();
    ensure(initialTicket.currentDepartmentId == "02_秘書室" || initialTicket.current_department_id == "02_秘書室", "新規札の初期部署が秘書室であること");

    const QVector<QString> baseHistory = initialTicket.routeHistory.isEmpty() ? initialTicket.route_history : initialTicket.routeHistory;
    ensure(!baseHistory.isEmpty(), "初期札が経路履歴を持つこと");

    bool routeHistoryUpdated = false;
    bool inspectionObserved = false;
    bool auditObserved = false;
    bool auditRejectObserved = false;
    bool recordSavedToGeneralAffairs = false;
    bool separateStatusObserved = false;
    bool rejectedNotCompleted = true;
    bool receivedEventObserved = false;
    bool startedEventObserved = false;
    bool completedEventObserved = false;
    bool rejectedEventObserved = false;
    bool inspectionEventObserved = false;
    bool auditStartedObserved = false;
    bool auditCompletedObserved = false;
    bool compressedAuditObserved = false;

    for (int i = 0; i < 40; ++i) {
        adapter.tick();
        const auto tickets = adapter.getTickets();
        for (const auto& ticket : tickets) {
            if (ticket.id == "TK-001" && ticket.routeHistory.size() > baseHistory.size()) {
                routeHistoryUpdated = true;
            }
            if (!ticket.inspection_status.isEmpty() && ticket.inspection_status != "not_started") {
                inspectionObserved = true;
            }
            if (!ticket.audit_status.isEmpty() && ticket.audit_status != "not_started") {
                auditObserved = true;
            }
            if ((ticket.inspection_status == "warn" || ticket.inspection_status == "passed" || ticket.inspection_status == "rejected") &&
                (ticket.audit_status == "warn" || ticket.audit_status == "passed" || ticket.audit_status == "rejected")) {
                if (ticket.inspection_status != ticket.audit_status) {
                    separateStatusObserved = true;
                }
            }
            if (ticket.status == TicketStatus::AUDIT_REJECTED || ticket.audit_status == "rejected") {
                auditRejectObserved = true;
            }
        }

        const auto events = adapter.getDepartmentEvents();
        for (const auto& event : events) {
            ensure(!event.event_id.isEmpty(), "イベントIDが付与されること");
            ensure(!event.timestamp.isEmpty(), "イベント時刻が付与されること");
            ensure(!event.message_summary.isEmpty(), "イベント要約が付与されること");
            if (event.event_type == "department.ticket.received") {
                receivedEventObserved = true;
            }
            if (event.event_type == "department.ticket.started") {
                startedEventObserved = true;
            }
            if (event.event_type == "department.ticket.completed") {
                completedEventObserved = true;
            }
            if (event.event_type == "department.ticket.rejected") {
                rejectedEventObserved = true;
            }
            if (event.event_type == "department.inspection.started" || event.event_type == "department.inspection.completed") {
                inspectionEventObserved = true;
            }
            if (event.event_type == "department.audit.started") {
                auditStartedObserved = true;
            }
            if (event.event_type == "department.audit.completed") {
                auditCompletedObserved = true;
                ensure(!event.ticket_id.isEmpty(), "イベントにticket_id が含まれること");
            }
            if (event.department_id == "10_庶務課" && event.type == DepartmentEventType::RecordSaved) {
                recordSavedToGeneralAffairs = true;
            }
        }

        for (const auto& ticket : tickets) {
            if (ticket.status == TicketStatus::DONE && ticket.audit_status == toString(DepartmentWorkStatus::REJECTED)) {
                rejectedNotCompleted = false;
            }
        }
    }

    const auto compressedAudits = adapter.getCompressedAuditTickets();
    for (const auto& audit : compressedAudits) {
        ensure(!audit.audit_result.isEmpty(), "圧縮監査札に監査結果があること");
        ensure(!audit.audit_flags.isEmpty(), "圧縮監査札に監査フラグがあること");
        ensure(!audit.violation_level.isEmpty(), "圧縮監査札に違反レベルがあること");
        ensure(!audit.required_action.isEmpty(), "圧縮監査札に是正アクションがあること");
        ensure(!audit.target_ticket_id.isEmpty(), "圧縮監査札に対象札IDがあること");
        ensure(!audit.target_department_id.isEmpty(), "圧縮監査札に対象部署があること");
        compressedAuditObserved = true;
        break;
    }

    ensure(routeHistoryUpdated, "基本ルート進行でroute_historyが更新されること");
    ensure(inspectionObserved, "検査ステータスが更新されること");
    ensure(auditObserved, "監査ステータスが更新されること");
    ensure(separateStatusObserved, "検査と監査ステータスが分離して管理されていること");
    ensure(auditRejectObserved, "監査NGのケースが少なくとも1件発生すること");
    ensure(recordSavedToGeneralAffairs, "庶務課への記録イベントが発火すること");
    ensure(receivedEventObserved, "department.ticket.received イベントを発火できること");
    ensure(startedEventObserved, "department.ticket.started イベントを発火できること");
    ensure(completedEventObserved, "department.ticket.completed イベントを発火できること");
    ensure(rejectedEventObserved, "department.ticket.rejected イベントを発火できること");
    ensure(inspectionEventObserved, "検査開始/完了イベントを発火できること");
    ensure(auditStartedObserved, "監査開始イベントを発火できること");
    ensure(auditCompletedObserved, "監査完了イベントを発火できること");
    ensure(compressedAuditObserved, "圧縮監査札が生成されること");
    ensure(rejectedNotCompleted, "監査NGが完了扱いにならないこと");

    std::cout << "Department goal tests passed." << std::endl;
    return 0;
}

