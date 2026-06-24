#include "agent_list_page.h"
#include "../domain.h"
#include <QHeaderView>
#include <QTableWidget>
#include <QVBoxLayout>

namespace nexspace {

AgentListPage::AgentListPage(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    m_table = new QTableWidget(this);
    m_table->setColumnCount(8);
    m_table->setHorizontalHeaderLabels({"AI名", "役割", "状態", "処理中札数", "完了札数", "監察指摘数", "エラー数", "最終稼働時刻"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_table);
}

void AgentListPage::setAgents(const QVector<Agent>& agents) {
    m_table->setRowCount(agents.size());
    for (int i = 0; i < agents.size(); ++i) {
        const auto& a = agents[i];
        m_table->setItem(i, 0, new QTableWidgetItem(a.name));
        m_table->setItem(i, 1, new QTableWidgetItem(toJapaneseRole(a.role)));
        m_table->setItem(i, 2, new QTableWidgetItem(a.status));
        m_table->setItem(i, 3, new QTableWidgetItem(QString::number(a.activeTickets)));
        m_table->setItem(i, 4, new QTableWidgetItem(QString::number(a.completedTickets)));
        m_table->setItem(i, 5, new QTableWidgetItem(QString::number(a.auditWarnings)));
        m_table->setItem(i, 6, new QTableWidgetItem(QString::number(a.errorTickets)));
        m_table->setItem(i, 7, new QTableWidgetItem(a.lastActiveAt));
    }
}

} // namespace nexspace
