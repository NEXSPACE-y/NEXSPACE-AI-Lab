#include "dashboard_page.h"
#include "../domain.h"
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QVBoxLayout>
#include <QTableWidget>

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
}

DashboardPage::DashboardPage(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    auto* card = new QWidget(this);
    auto* cardLayout = new QGridLayout(card);

    m_systemStatus = new QLabel("本陣状態: ---");
    m_waiting = new QLabel("待機: 0");
    m_processing = new QLabel("出陣中: 0");
    m_auditPending = new QLabel("監察待ち: 0");
    m_rejected = new QLabel("差戻し: 0");
    m_done = new QLabel("帰陣: 0");
    m_error = new QLabel("異常: 0");
    m_activeAi = new QLabel("稼働AI: 0");
    m_aiError = new QLabel("異常AI: 0");

    cardLayout->addWidget(m_systemStatus, 0, 0);
    cardLayout->addWidget(m_waiting, 0, 1);
    cardLayout->addWidget(m_processing, 0, 2);
    cardLayout->addWidget(m_auditPending, 0, 3);
    cardLayout->addWidget(m_rejected, 1, 0);
    cardLayout->addWidget(m_done, 1, 1);
    cardLayout->addWidget(m_error, 1, 2);
    cardLayout->addWidget(m_activeAi, 1, 3);
    cardLayout->addWidget(m_aiError, 2, 0);

    auto* latest = new QLabel("監察最近履歴");
    m_auditTable = new QTableWidget(this);
    m_auditTable->setColumnCount(5);
    m_auditTable->setHorizontalHeaderLabels({"時刻", "対象令札ID", "判定", "カテゴリ", "理由"});
    m_auditTable->horizontalHeader()->setStretchLastSection(true);
    m_auditTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_auditTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout->addWidget(card);
    layout->addWidget(latest);
    layout->addWidget(m_auditTable, 1);
}

void DashboardPage::updateData(const SystemStatus& status,
                               const QVector<Agent>& agents,
                               const QVector<AuditLog>& latestAuditLogs) {
    m_systemStatus->setText(QString("本陣状態: %1").arg(status.status));
    m_waiting->setText(QString("待機: %1").arg(status.waitingTickets));
    m_processing->setText(QString("出陣中: %1").arg(status.processingTickets));
    m_auditPending->setText(QString("監察待ち: %1").arg(status.auditPendingTickets));
    m_rejected->setText(QString("差戻し: %1").arg(status.rejectedTickets));
    m_done->setText(QString("帰陣: %1").arg(status.completedTickets));
    m_error->setText(QString("異常: %1").arg(status.errorTickets));

    int runningAi = 0;
    int errorAi = 0;
    for (const auto& agent : agents) {
        if (agent.status == "running") {
            ++runningAi;
        }
        if (agent.status == "error") {
            ++errorAi;
        }
    }
    m_activeAi->setText(QString("稼働AI: %1").arg(runningAi));
    m_aiError->setText(QString("異常AI: %1").arg(errorAi));

    m_auditTable->setRowCount(0);
    const int maxRows = 5;
    const int rows = qMin(maxRows, latestAuditLogs.size());
    m_auditTable->setRowCount(rows);
    for (int i = 0; i < rows; ++i) {
        const auto& log = latestAuditLogs[i];
        m_auditTable->setItem(i, 0, new QTableWidgetItem(log.createdAt));
        m_auditTable->setItem(i, 1, new QTableWidgetItem(log.ticketId));
        m_auditTable->setItem(i, 2, new QTableWidgetItem(toJapaneseAuditResult(log.result)));
        m_auditTable->setItem(i, 3, new QTableWidgetItem(auditCategoryLabel(log.category)));
        m_auditTable->setItem(i, 4, new QTableWidgetItem(log.reasonSummary));
    }
}

} // namespace nexspace
