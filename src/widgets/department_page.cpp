#include "department_page.h"

#include "../domain.h"
#include <QHeaderView>
#include <QGroupBox>
#include <QLabel>
#include <QColor>
#include <QVBoxLayout>
#include <QTreeWidgetItem>

namespace nexspace {

namespace {

QString routeText(const QVector<QString>& route) {
    if (route.isEmpty()) return "未設定";
    QStringList nodes;
    for (const auto& node : route) {
        nodes.append(node);
    }
    return nodes.join(" → ");
}

} // namespace

DepartmentPage::DepartmentPage(QWidget* parent)
    : QWidget(parent) {
    auto* root = new QVBoxLayout(this);

    auto* structureGroup = new QGroupBox("部署構成");
    auto* structureLayout = new QVBoxLayout(structureGroup);
    m_departmentTree = new QTreeWidget(this);
    m_departmentTree->setColumnCount(7);
    m_departmentTree->setHeaderLabels({"部署ID", "部署名", "稼働 / 完了", "完了", "監査警告", "検査警告", "記録"});
    m_departmentTree->header()->setStretchLastSection(true);
    structureLayout->addWidget(m_departmentTree);

    auto* flowGroup = new QGroupBox("札の処理フロー");
    auto* flowLayout = new QVBoxLayout(flowGroup);
    m_ticketFlowTable = new QTableWidget(this);
    m_ticketFlowTable->setColumnCount(7);
    m_ticketFlowTable->setHorizontalHeaderLabels({"令札ID", "現在部署", "次工程", "状態", "担当AI", "監察", "履歴"});
    m_ticketFlowTable->horizontalHeader()->setStretchLastSection(true);
    m_ticketFlowTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    flowLayout->addWidget(m_ticketFlowTable);

    auto* eventGroup = new QGroupBox("部署イベント");
    auto* eventLayout = new QVBoxLayout(eventGroup);
    m_eventTable = new QTableWidget(this);
    m_eventTable->setColumnCount(6);
    m_eventTable->setHorizontalHeaderLabels({"時刻", "令札ID", "部署(From)", "部署(To)", "種類", "内容"});
    m_eventTable->horizontalHeader()->setStretchLastSection(true);
    m_eventTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    eventLayout->addWidget(m_eventTable);

    auto* recordGroup = new QGroupBox("庶務課ログ");
    auto* recordLayout = new QVBoxLayout(recordGroup);
    m_recordTable = new QTableWidget(this);
    m_recordTable->setColumnCount(4);
    m_recordTable->setHorizontalHeaderLabels({"時刻", "令札ID", "部署", "要約"});
    m_recordTable->horizontalHeader()->setStretchLastSection(true);
    m_recordTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    recordLayout->addWidget(m_recordTable);

    root->addWidget(structureGroup, 2);
    root->addWidget(flowGroup, 3);
    root->addWidget(eventGroup, 2);
    root->addWidget(recordGroup, 2);
}

void DepartmentPage::setDepartments(const QVector<Department>& departments) {
    refreshDepartmentTree(departments);
}

void DepartmentPage::setDepartmentEvents(const QVector<DepartmentEvent>& events) {
    refreshEventTable(events);
}

void DepartmentPage::setDepartmentRecords(const QVector<DepartmentRecord>& records) {
    refreshRecordTable(records);
}

void DepartmentPage::setTickets(const QVector<Ticket>& tickets) {
    refreshFlowTable(tickets);
}

void DepartmentPage::refreshDepartmentTree(const QVector<Department>& departments) {
    m_departmentTree->clear();

    QHash<QString, QTreeWidgetItem*> itemById;
    for (const auto& dept : departments) {
        auto* item = new QTreeWidgetItem();
        item->setText(0, dept.id);
        item->setText(1, dept.name);
        item->setText(2, QString("%1 / %2").arg(dept.activeTickets).arg(dept.completedTickets));
        item->setText(3, QString::number(dept.completedTickets));
        item->setText(4, QString::number(dept.auditWarnings));
        item->setText(5, QString::number(dept.inspectionWarnings));
        item->setText(6, QString::number(dept.recordCount));
        item->setForeground(0, QColor(dept.isParent ? "#ffe58a" : "#dfe8ff"));
        itemById.insert(dept.id, item);
    }

    QVector<QTreeWidgetItem*> topItems;
    for (const auto& dept : departments) {
        auto item = itemById.value(dept.id, nullptr);
        if (!item) {
            continue;
        }
        if (dept.parentId.isEmpty() || !itemById.contains(dept.parentId)) {
            topItems.append(item);
        } else {
            itemById.value(dept.parentId)->addChild(item);
        }
    }

    for (auto* item : topItems) {
        m_departmentTree->addTopLevelItem(item);
        item->setExpanded(true);
    }

    m_departmentTree->resizeColumnToContents(0);
    m_departmentTree->resizeColumnToContents(1);
}

void DepartmentPage::refreshFlowTable(const QVector<Ticket>& tickets) {
    m_ticketFlowTable->setRowCount(tickets.size());
    for (int i = 0; i < tickets.size(); ++i) {
        const auto& ticket = tickets[i];
        m_ticketFlowTable->setItem(i, 0, new QTableWidgetItem(ticket.id));
        m_ticketFlowTable->setItem(i, 1, new QTableWidgetItem(toJapaneseDepartment(ticket.currentDepartmentId)));
        m_ticketFlowTable->setItem(i, 2, new QTableWidgetItem(toJapaneseDepartment(ticket.nextDepartmentId)));
        m_ticketFlowTable->setItem(i, 3, new QTableWidgetItem(toJapaneseStatus(ticket.status)));
        m_ticketFlowTable->setItem(i, 4, new QTableWidgetItem(ticket.assignedAgentId.isEmpty() ? "未割当" : ticket.assignedAgentId));
        m_ticketFlowTable->setItem(i, 5, new QTableWidgetItem(ticket.auditResult.has_value()
            ? toJapaneseAuditResult(ticket.auditResult.value())
            : "未判定"));
        m_ticketFlowTable->setItem(i, 6, new QTableWidgetItem(routeText(ticket.routeHistory)));
    }
}

void DepartmentPage::refreshEventTable(const QVector<DepartmentEvent>& events) {
    m_eventTable->setRowCount(events.size());
    for (int i = 0; i < events.size(); ++i) {
        const auto& e = events[i];
        m_eventTable->setItem(i, 0, new QTableWidgetItem(e.createdAt));
        m_eventTable->setItem(i, 1, new QTableWidgetItem(e.ticketId));
        m_eventTable->setItem(i, 2, new QTableWidgetItem(toJapaneseDepartment(e.fromDepartmentId)));
        m_eventTable->setItem(i, 3, new QTableWidgetItem(toJapaneseDepartment(e.toDepartmentId)));
        m_eventTable->setItem(i, 4, new QTableWidgetItem(toJapaneseDepartmentEvent(e.type)));
        m_eventTable->setItem(i, 5, new QTableWidgetItem(e.note));
    }
}

void DepartmentPage::refreshRecordTable(const QVector<DepartmentRecord>& records) {
    m_recordTable->setRowCount(records.size());
    for (int i = 0; i < records.size(); ++i) {
        const auto& r = records[i];
        m_recordTable->setItem(i, 0, new QTableWidgetItem(r.createdAt));
        m_recordTable->setItem(i, 1, new QTableWidgetItem(r.ticketId));
        m_recordTable->setItem(i, 2, new QTableWidgetItem(toJapaneseDepartment(r.departmentId)));
        m_recordTable->setItem(i, 3, new QTableWidgetItem(r.summary));
    }
}

} // namespace nexspace
