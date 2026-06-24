#pragma once

#include "../adapters/engine_adapter.h"
#include <QWidget>
#include <QStringList>
#include <QLabel>

namespace nexspace {

class SystemStatusPanel : public QWidget {
    Q_OBJECT
public:
    explicit SystemStatusPanel(QWidget* parent = nullptr);

    void setData(const SystemStatus& status, const EngineAdapterInfo& adapterInfo, const QStringList& notes);

private:
    QLabel* m_status = nullptr;
    QLabel* m_uptime = nullptr;
    QLabel* m_adapter = nullptr;
    QLabel* m_lastSync = nullptr;
    QLabel* m_errors = nullptr;
};

} // namespace nexspace
