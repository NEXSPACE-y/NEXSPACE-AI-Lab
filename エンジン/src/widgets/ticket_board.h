#pragma once

#include "../adapters/engine_adapter.h"
#include <QWidget>
#include <QListWidget>

namespace nexspace {

class TicketBoard : public QWidget {
    Q_OBJECT
public:
    explicit TicketBoard(QWidget* parent = nullptr);

    void setTickets(const QVector<Ticket>& tickets);

signals:
    void ticketSelected(const QString& ticketId);

private:
    void addColumnItems(QListWidget* list, const QString& statusLabel, const QVector<Ticket>& tickets);
    QString ticketDisplayText(const Ticket& ticket) const;

    QListWidget* m_waiting = nullptr;
    QListWidget* m_processing = nullptr;
    QListWidget* m_audit = nullptr;
    QListWidget* m_doneOrReject = nullptr;
};

} // namespace nexspace
