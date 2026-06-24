#pragma once

#include "../adapters/engine_adapter.h"
#include <QWidget>
#include <QLabel>

namespace nexspace {

class TicketDetailPanel : public QWidget {
    Q_OBJECT
public:
    explicit TicketDetailPanel(QWidget* parent = nullptr);

    void setTicket(const std::optional<Ticket>& ticket, const QVector<AuditLog>& logs);
    void clear();

private:
    QLabel* m_id = nullptr;
    QLabel* m_title = nullptr;
    QLabel* m_summary = nullptr;
    QLabel* m_type = nullptr;
    QLabel* m_priority = nullptr;
    QLabel* m_status = nullptr;
    QLabel* m_agent = nullptr;
    QLabel* m_auditResult = nullptr;
    QLabel* m_auditCategory = nullptr;
    QLabel* m_recommend = nullptr;
    QLabel* m_created = nullptr;
    QLabel* m_updated = nullptr;
    QLabel* m_internal = nullptr;
};

} // namespace nexspace
