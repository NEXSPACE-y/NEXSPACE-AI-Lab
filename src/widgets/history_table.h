#pragma once

#include "../adapters/engine_adapter.h"
#include <QDateTime>
#include <QWidget>
#include <QDateEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QTableWidget>

namespace nexspace {

class HistoryTable : public QWidget {
    Q_OBJECT
public:
    explicit HistoryTable(QWidget* parent = nullptr);

    void setHistory(const QVector<HistoryEntry>& history);

private:
    void applyFilters();
    void buildRow(const HistoryEntry& row, int index);

    QVector<HistoryEntry> m_fullHistory;
    QDateEdit* m_fromDate = nullptr;
    QDateEdit* m_toDate = nullptr;
    QLineEdit* m_search = nullptr;
    QComboBox* m_agentFilter = nullptr;
    QComboBox* m_resultFilter = nullptr;
    QComboBox* m_errorFilter = nullptr;
    QTableWidget* m_table = nullptr;
};

} // namespace nexspace
