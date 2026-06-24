#include "ticket_board.h"
#include "../domain.h"
#include <QColor>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QStringList>

namespace nexspace {

TicketBoard::TicketBoard(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* waitingGroup = new QVBoxLayout();
    auto* processingGroup = new QVBoxLayout();
    auto* auditGroup = new QVBoxLayout();
    auto* doneGroup = new QVBoxLayout();

    auto* wTitle = new QLabel("待機");
    auto* pTitle = new QLabel("出陣中");
    auto* aTitle = new QLabel("監察待ち");
    auto* dTitle = new QLabel("帰陣 / 差戻し");

    m_waiting = new QListWidget();
    m_processing = new QListWidget();
    m_audit = new QListWidget();
    m_doneOrReject = new QListWidget();

    for (auto* list : {m_waiting, m_processing, m_audit, m_doneOrReject}) {
        list->setSelectionMode(QAbstractItemView::SingleSelection);
        list->setAlternatingRowColors(true);
    }

    waitingGroup->addWidget(wTitle);
    waitingGroup->addWidget(m_waiting);
    processingGroup->addWidget(pTitle);
    processingGroup->addWidget(m_processing);
    auditGroup->addWidget(aTitle);
    auditGroup->addWidget(m_audit);
    doneGroup->addWidget(dTitle);
    doneGroup->addWidget(m_doneOrReject);

    auto* waitingContainer = new QWidget();
    auto* processingContainer = new QWidget();
    auto* auditContainer = new QWidget();
    auto* doneContainer = new QWidget();
    waitingContainer->setLayout(waitingGroup);
    processingContainer->setLayout(processingGroup);
    auditContainer->setLayout(auditGroup);
    doneContainer->setLayout(doneGroup);

    layout->addWidget(waitingContainer, 0, 0);
    layout->addWidget(processingContainer, 0, 1);
    layout->addWidget(auditContainer, 0, 2);
    layout->addWidget(doneContainer, 0, 3);

    connect(m_waiting, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        emit ticketSelected(item->data(Qt::UserRole).toString());
    });
    connect(m_processing, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        emit ticketSelected(item->data(Qt::UserRole).toString());
    });
    connect(m_audit, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        emit ticketSelected(item->data(Qt::UserRole).toString());
    });
    connect(m_doneOrReject, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        emit ticketSelected(item->data(Qt::UserRole).toString());
    });
}

QString TicketBoard::ticketDisplayText(const Ticket& ticket) const {
    const QString auditLabel = ticket.auditResult.has_value()
        ? toJapaneseAuditResult(ticket.auditResult.value())
        : "未判定";
    QString agent = ticket.assignedAgentId.isEmpty() ? "未割当" : ticket.assignedAgentId;
    QString department = ticket.currentDepartmentId.isEmpty() ? "未割当" : toJapaneseDepartment(ticket.currentDepartmentId);
    QString next = ticket.nextDepartmentId.isEmpty() ? "未設定" : toJapaneseDepartment(ticket.nextDepartmentId);
    QStringList history;
    for (const auto& node : ticket.routeHistory) {
        history.append(node);
    }
    const QString route = history.isEmpty() ? "なし" : history.join(" → ");
    return QString("[%1] %2\n種別:%3 優先:%4 状態:%5 担当:%6 監察:%7\n現在部署:%8 次工程:%9\n処理履歴:%10\n作成:%11 更新:%12")
        .arg(ticket.id,
             ticket.title,
             ticket.type,
             toLabel(ticket.priority),
             toJapaneseStatus(ticket.status),
             agent,
             auditLabel,
             department,
             next,
             route,
             ticket.createdAt,
             ticket.updatedAt);
}

void TicketBoard::addColumnItems(QListWidget* list, const QString& statusLabel, const QVector<Ticket>& tickets) {
    const QColor accent = [&statusLabel]() {
        if (statusLabel == "待機") return QColor("#1f2f4a");
        if (statusLabel == "出陣中") return QColor("#1f4a4a");
        if (statusLabel == "監察待ち") return QColor("#3f2f3f");
        return QColor("#303f2f");
    }();

    for (const auto& t : tickets) {
        auto* item = new QListWidgetItem(ticketDisplayText(t));
        item->setData(Qt::UserRole, t.id);
        item->setBackground(accent);
        list->addItem(item);
    }
}

void TicketBoard::setTickets(const QVector<Ticket>& tickets) {
    m_waiting->clear();
    m_processing->clear();
    m_audit->clear();
    m_doneOrReject->clear();

    QVector<Ticket> waiting;
    QVector<Ticket> processing;
    QVector<Ticket> audit;
    QVector<Ticket> done;

    for (const auto& t : tickets) {
        switch (t.status) {
            case TicketStatus::WAITING:
            case TicketStatus::ASSIGNED:
                waiting.push_back(t);
                break;
            case TicketStatus::PROCESSING:
                processing.push_back(t);
                break;
            case TicketStatus::AUDIT_PENDING:
                audit.push_back(t);
                break;
            case TicketStatus::DONE:
            case TicketStatus::AUDIT_PASSED:
            case TicketStatus::AUDIT_REJECTED:
            case TicketStatus::ERROR:
                done.push_back(t);
                break;
            default:
                waiting.push_back(t);
        }
    }

    addColumnItems(m_waiting, "待機", waiting);
    addColumnItems(m_processing, "出陣中", processing);
    addColumnItems(m_audit, "監察待ち", audit);
    addColumnItems(m_doneOrReject, "帰陣/差戻し", done);
}

} // namespace nexspace
