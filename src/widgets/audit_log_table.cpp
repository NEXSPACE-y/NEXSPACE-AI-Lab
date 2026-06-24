#include "audit_log_table.h"
#include "../domain.h"
#include <QHeaderView>
#include <QTableWidget>
#include <QVBoxLayout>

namespace nexspace {

namespace {
QString auditCategoryLabel(const QString& category) {
    if (category == "requirement_missing") return "要件未達";
    if (category == "scope_deviation") return "仕様逸脱";
    if (category == "unsafe_action") return "危険操作";
    if (category == "tool_mismatch") return "ツール不整合";
    if (category == "format_error") return "形式不備";
    if (category == "incomplete_output") return "未完成出力";
    if (category == "unauthorized_change") return "未承認変更";
    if (category == "system_error") return "系統異常";
    return category;
}
} // namespace

AuditLogTable::AuditLogTable(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    m_table = new QTableWidget(this);
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({"時刻", "対象令札ID", "対象AI", "判定", "指摘カテゴリ", "理由要約", "推奨対応"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_table);
}

void AuditLogTable::setAuditLogs(const QVector<AuditLog>& logs) {
    m_table->setRowCount(logs.size());
    for (int i = 0; i < logs.size(); ++i) {
        const auto& log = logs[i];
        m_table->setItem(i, 0, new QTableWidgetItem(log.createdAt));
        m_table->setItem(i, 1, new QTableWidgetItem(log.ticketId));
        m_table->setItem(i, 2, new QTableWidgetItem(log.targetAgentId));
        m_table->setItem(i, 3, new QTableWidgetItem(toJapaneseAuditResult(log.result)));
        m_table->setItem(i, 4, new QTableWidgetItem(auditCategoryLabel(log.category)));
        m_table->setItem(i, 5, new QTableWidgetItem(log.reasonSummary));
        m_table->setItem(i, 6, new QTableWidgetItem(log.recommendedAction + (log.retryable ? " / 再処理可" : " / 再処理不可")));
    }
}

} // namespace nexspace
