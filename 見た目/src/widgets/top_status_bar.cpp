#include "top_status_bar.h"
#include <QGridLayout>
#include <QLabel>

namespace nexspace {

TopStatusBar::TopStatusBar(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QGridLayout(this);
    setObjectName("TopStatusBar");
    setMinimumHeight(90);

    m_title = new QLabel("NEXSPACE AI Lab 軍議管制");
    m_state = new QLabel("本陣状態: ---");
    m_waiting = new QLabel("待機札数: 0");
    m_processing = new QLabel("出陣中札数: 0");
    m_auditPending = new QLabel("監察待ち札数: 0");
    m_rejected = new QLabel("差戻し札数: 0");
    m_done = new QLabel("完了札数: 0");
    m_error = new QLabel("異常数: 0");
    m_uptime = new QLabel("稼働時間: 00:00:00");

    auto add = [&](QLabel* label, int row, int col) {
        label->setObjectName("StatusValue");
        layout->addWidget(label, row, col);
    };

    add(m_title, 0, 0);
    add(m_state, 0, 1);
    add(m_waiting, 0, 2);
    add(m_processing, 0, 3);
    add(m_auditPending, 1, 0);
    add(m_rejected, 1, 1);
    add(m_done, 1, 2);
    add(m_error, 1, 3);
    add(m_uptime, 1, 4);
}

void TopStatusBar::setAdapterInfo(const EngineAdapterInfo& info) {
    QString typeLabel = "unknown";
    switch (info.type) {
        case EngineAdapterType::Mock:
            typeLabel = "mock";
            break;
        case EngineAdapterType::Real:
            typeLabel = "real";
            break;
        case EngineAdapterType::Noop:
            typeLabel = "noop";
            break;
    }
    m_state->setText(QString("本陣状態: %1 [%2 / %3]")
                     .arg(info.connected ? "接続中" : "未接続", typeLabel, info.name));
}

void TopStatusBar::setSystemStatus(const SystemStatus& status) {
    m_waiting->setText(QString("待機札数: %1").arg(status.waitingTickets));
    m_processing->setText(QString("出陣中札数: %1").arg(status.processingTickets));
    m_auditPending->setText(QString("監察待ち札数: %1").arg(status.auditPendingTickets));
    m_rejected->setText(QString("差戻し札数: %1").arg(status.rejectedTickets));
    m_done->setText(QString("完了札数: %1").arg(status.completedTickets));
    m_error->setText(QString("異常数: %1").arg(status.errorTickets));
    m_uptime->setText(QString("稼働時間: %1秒").arg(status.uptimeSeconds));
}

} // namespace nexspace
