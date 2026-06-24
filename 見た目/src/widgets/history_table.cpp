#include "history_table.h"
#include "../domain.h"
#include <QDateEdit>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QDate>
#include <QVBoxLayout>

namespace nexspace {

HistoryTable::HistoryTable(QWidget* parent)
    : QWidget(parent) {
    auto* root = new QVBoxLayout(this);

    auto* filters = new QHBoxLayout();
    m_search = new QLineEdit();
    m_search->setPlaceholderText("令札ID/要約検索");
    m_fromDate = new QDateEdit(QDate::currentDate().addDays(-30));
    m_toDate = new QDateEdit(QDate::currentDate());
    m_fromDate->setCalendarPopup(true);
    m_toDate->setCalendarPopup(true);
    m_fromDate->setDisplayFormat("yyyy-MM-dd");
    m_toDate->setDisplayFormat("yyyy-MM-dd");

    m_agentFilter = new QComboBox();
    m_agentFilter->addItem("全AI");
    m_resultFilter = new QComboBox();
    m_resultFilter->addItems({"全結果", "PASS", "WARN", "REJECT", "ESCALATE", "SYSTEM_ERROR"});
    m_errorFilter = new QComboBox();
    m_errorFilter->addItems({"全て", "異常あり", "異常なし"});

    auto* apply = new QPushButton("適用");
    connect(apply, &QPushButton::clicked, this, &HistoryTable::applyFilters);

    filters->addWidget(m_search);
    filters->addWidget(m_fromDate);
    filters->addWidget(m_toDate);
    filters->addWidget(m_agentFilter);
    filters->addWidget(m_resultFilter);
    filters->addWidget(m_errorFilter);
    filters->addWidget(apply);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({"令札ID", "入力要約", "担当AI", "処理結果", "監察結果", "所要時間", "更新時刻"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    root->addLayout(filters);
    root->addWidget(m_table);
}

void HistoryTable::setHistory(const QVector<HistoryEntry>& history) {
    m_fullHistory = history;
    m_agentFilter->clear();
    m_agentFilter->addItem("全AI");
    for (const auto& row : history) {
        if (!row.assignedAgentId.isEmpty() && m_agentFilter->findText(row.assignedAgentId) < 0) {
            m_agentFilter->addItem(row.assignedAgentId);
        }
    }
    applyFilters();
}

void HistoryTable::buildRow(const HistoryEntry& row, int index) {
    m_table->setItem(index, 0, new QTableWidgetItem(row.ticketId));
    m_table->setItem(index, 1, new QTableWidgetItem(row.inputSummary));
    m_table->setItem(index, 2, new QTableWidgetItem(row.assignedAgentId));
    m_table->setItem(index, 3, new QTableWidgetItem(row.processResult));
    m_table->setItem(index, 4, new QTableWidgetItem(toJapaneseAuditResult(row.auditResult)));
    m_table->setItem(index, 5, new QTableWidgetItem(row.duration));
    m_table->setItem(index, 6, new QTableWidgetItem(row.updatedAt));
}

void HistoryTable::applyFilters() {
    m_table->setRowCount(0);
    QVector<HistoryEntry> filtered;
    const QString keyword = m_search->text().trimmed();
    const QString agent = m_agentFilter->currentText();
    const QString resultFilter = m_resultFilter->currentText();
    const QString errFilter = m_errorFilter->currentText();
    const QDate from = m_fromDate->date();
    const QDate to = m_toDate->date();

    for (const auto& row : m_fullHistory) {
        if (!keyword.isEmpty()) {
            if (!row.ticketId.contains(keyword) && !row.inputSummary.contains(keyword)) {
                continue;
            }
        }
        if (agent != "全AI" && row.assignedAgentId != agent) {
            continue;
        }
        if (resultFilter != "全結果" && toString(row.auditResult) != resultFilter) {
            continue;
        }
        if (errFilter == "異常あり" && !row.hasError) {
            continue;
        }
        if (errFilter == "異常なし" && row.hasError) {
            continue;
        }
        const QDate updated = QDateTime::fromString(row.updatedAt, "yyyy-MM-dd HH:mm:ss").date();
        if (updated < from || updated > to) {
            continue;
        }
        filtered.push_back(row);
    }

    m_table->setRowCount(filtered.size());
    for (int i = 0; i < filtered.size(); ++i) {
        buildRow(filtered[i], i);
    }
}

} // namespace nexspace

