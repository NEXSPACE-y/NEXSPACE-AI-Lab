#pragma once

#include "../adapters/engine_adapter.h"
#include <QWidget>
#include <QLabel>

namespace nexspace {

class TopStatusBar : public QWidget {
    Q_OBJECT
public:
    explicit TopStatusBar(QWidget* parent = nullptr);

    void setAdapterInfo(const EngineAdapterInfo& info);
    void setSystemStatus(const SystemStatus& status);

private:
    QLabel* m_title = nullptr;
    QLabel* m_state = nullptr;
    QLabel* m_waiting = nullptr;
    QLabel* m_processing = nullptr;
    QLabel* m_auditPending = nullptr;
    QLabel* m_rejected = nullptr;
    QLabel* m_done = nullptr;
    QLabel* m_error = nullptr;
    QLabel* m_uptime = nullptr;
};

} // namespace nexspace
