#include "system_status_panel.h"
#include <QDateTime>
#include <QVBoxLayout>
#include <QLabel>

namespace nexspace {

SystemStatusPanel::SystemStatusPanel(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    m_status = new QLabel("稼働状態: ---");
    m_uptime = new QLabel("稼働時間: 0");
    m_adapter = new QLabel("接続: ---");
    m_lastSync = new QLabel("最終同期: ---");
    m_errors = new QLabel("エラー一覧: ---");
    layout->addWidget(m_status);
    layout->addWidget(m_uptime);
    layout->addWidget(m_adapter);
    layout->addWidget(m_lastSync);
    layout->addWidget(m_errors);
    layout->addStretch();
}

void SystemStatusPanel::setData(const SystemStatus& status,
                               const EngineAdapterInfo& adapterInfo,
                               const QStringList& notes) {
    m_status->setText(QString("稼働状態: %1").arg(status.status));
    m_uptime->setText(QString("稼働時間: %1秒").arg(status.uptimeSeconds));
    const QString conn = adapterInfo.connected ? "接続中" : "未接続";
    QString adapterType = "unknown";
    switch (adapterInfo.type) {
        case EngineAdapterType::Mock:
            adapterType = "mock";
            break;
        case EngineAdapterType::Real:
            adapterType = "real";
            break;
        case EngineAdapterType::Noop:
            adapterType = "noop";
            break;
    }
    m_adapter->setText(QString("エンジン接続状態: %1 / 種別: %2 / 名称: %3")
                       .arg(conn, adapterType, adapterInfo.name));
    m_lastSync->setText(QString("最終同期時刻: %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")));
    if (notes.isEmpty()) {
        m_errors->setText("エラー一覧: 現在検知なし");
    } else {
        m_errors->setText(QString("エラー一覧: %1").arg(notes.join(" / ")));
    }
}

} // namespace nexspace
