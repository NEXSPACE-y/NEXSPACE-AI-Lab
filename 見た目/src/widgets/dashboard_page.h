#pragma once

#include "../adapters/engine_adapter.h"
#include <QWidget>
#include <QLabel>
#include <QTableWidget>

namespace nexspace {

class DashboardPage : public QWidget {
    Q_OBJECT
public:
    explicit DashboardPage(QWidget* parent = nullptr);

    void updateData(const SystemStatus& status,
                    const QVector<Agent>& agents,
                    const QVector<AuditLog>& latestAuditLogs);

    QLabel* m_systemStatus = nullptr;
    QLabel* m_waiting = nullptr;
    QLabel* m_processing = nullptr;
    QLabel* m_auditPending = nullptr;
    QLabel* m_rejected = nullptr;
    QLabel* m_done = nullptr;
    QLabel* m_error = nullptr;
    QLabel* m_activeAi = nullptr;
    QLabel* m_aiError = nullptr;
    QTableWidget* m_auditTable = nullptr;
};

} // namespace nexspace
