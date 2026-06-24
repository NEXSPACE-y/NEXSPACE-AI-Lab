#pragma once

#include "../adapters/engine_adapter.h"
#include <QWidget>
#include <QTreeWidget>
#include <QTableWidget>

namespace nexspace {

class DepartmentPage : public QWidget {
    Q_OBJECT
public:
    explicit DepartmentPage(QWidget* parent = nullptr);

    void setDepartments(const QVector<Department>& departments);
    void setDepartmentEvents(const QVector<DepartmentEvent>& events);
    void setDepartmentRecords(const QVector<DepartmentRecord>& records);
    void setTickets(const QVector<Ticket>& tickets);

private:
    void refreshDepartmentTree(const QVector<Department>& departments);
    void refreshFlowTable(const QVector<Ticket>& tickets);
    void refreshEventTable(const QVector<DepartmentEvent>& events);
    void refreshRecordTable(const QVector<DepartmentRecord>& records);

    QTreeWidget* m_departmentTree = nullptr;
    QTableWidget* m_ticketFlowTable = nullptr;
    QTableWidget* m_eventTable = nullptr;
    QTableWidget* m_recordTable = nullptr;
};

} // namespace nexspace
