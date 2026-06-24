#pragma once

#include "../adapters/engine_adapter.h"
#include <QWidget>
#include <QTableWidget>

namespace nexspace {

class AgentListPage : public QWidget {
    Q_OBJECT
public:
    explicit AgentListPage(QWidget* parent = nullptr);

    void setAgents(const QVector<Agent>& agents);

private:
    QTableWidget* m_table = nullptr;
};

} // namespace nexspace
