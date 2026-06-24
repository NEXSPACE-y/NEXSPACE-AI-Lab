#pragma once

#include "../adapters/engine_adapter.h"
#include <QWidget>
#include <QTableWidget>

namespace nexspace {

class AuditLogTable : public QWidget {
    Q_OBJECT
public:
    explicit AuditLogTable(QWidget* parent = nullptr);

    void setAuditLogs(const QVector<AuditLog>& logs);

private:
    QTableWidget* m_table = nullptr;
};

} // namespace nexspace
