#include "app_shell.h"
#include <algorithm>
#include <QListWidget>
#include <QLabel>
#include <QSplitter>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>

namespace nexspace {

AppShell::AppShell(std::unique_ptr<EngineAdapter> adapter, QWidget* parent)
    : QMainWindow(parent), m_adapter(std::move(adapter)), m_soundService(m_soundCueAdapter) {
    buildUi();
    applyStyle();
    refreshData();

    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(3000);
    connect(m_refreshTimer, &QTimer::timeout, this, &AppShell::refreshData);
    m_refreshTimer->start();
}

void AppShell::buildUi() {
    QWidget* container = new QWidget();
    QVBoxLayout* root = new QVBoxLayout(container);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_topBar = new TopStatusBar();
    m_sidebar = new QListWidget();
    m_ticketBoard = new TicketBoard();
    m_detailPanel = new TicketDetailPanel();
    m_pages = new QStackedWidget();
    m_commandInputPage = new CommandInputPage();
    m_dashboard = new DashboardPage();
    m_agentList = new AgentListPage();
    m_auditLog = new AuditLogTable();
    m_history = new HistoryTable();
    m_settings = new SettingsPage();
    m_systemStatus = new SystemStatusPanel();
    m_commandInputBar = new CommandInputBar();

    m_sidebar->addItems({"軍議盤", "札場", "軍令入力", "配下一覧", "監察記録", "軍務履歴", "軍令設定", "本陣状態"});
    m_sidebar->setFixedWidth(170);
    m_sidebar->setCurrentRow(1);

    m_pages->addWidget(m_dashboard);
    m_pages->addWidget(m_ticketBoard);
    m_pages->addWidget(m_commandInputPage);
    m_pages->addWidget(m_agentList);
    m_pages->addWidget(m_auditLog);
    m_pages->addWidget(m_history);
    m_pages->addWidget(m_settings);
    m_pages->addWidget(m_systemStatus);

    m_bodySplitter = new QSplitter(Qt::Horizontal);
    m_bodySplitter->setChildrenCollapsible(false);
    m_bodySplitter->addWidget(m_sidebar);
    m_bodySplitter->addWidget(m_pages);
    m_bodySplitter->addWidget(m_detailPanel);
    m_bodySplitter->setStretchFactor(0, 0);
    m_bodySplitter->setStretchFactor(1, 5);
    m_bodySplitter->setStretchFactor(2, 2);
    m_bodySplitter->setSizes({170, 820, 360});

    root->addWidget(m_topBar);
    root->addWidget(m_bodySplitter, 1);
    root->addWidget(m_commandInputBar);
    setCentralWidget(container);

    connect(m_sidebar, &QListWidget::currentRowChanged, this, &AppShell::onSidebarIndexChanged);
    connect(m_ticketBoard, &TicketBoard::ticketSelected, this, &AppShell::onTicketSelected);
    connect(m_settings, &SettingsPage::settingsApplied, this, &AppShell::onSettingsApplied);
    connect(m_commandInputBar, &CommandInputBar::submitRequested, this, &AppShell::onCommandSubmitRequested);
    connect(m_commandInputPage, &CommandInputPage::submitRequested, this, &AppShell::onCommandSubmitRequested);
}

void AppShell::applyStyle() {
    setMinimumSize(1180, 720);
    setStyleSheet(R"(
        QWidget {
            background-color: #0f1629;
            color: #ede2c6;
            font-family: "Yu Gothic", "Meiryo", "Noto Sans JP", "Segoe UI", "Arial", sans-serif;
            font-size: 10pt;
        }
        QWidget#TopStatusBar {
            background-color: #131f3c;
            border-bottom: 1px solid #4b5e82;
            padding: 6px;
        }
        QListWidget {
            background-color: #18243f;
            border-right: 1px solid #32456a;
            min-width: 160px;
        }
        QSplitter::handle {
            background: #2f4270;
        }
        QSplitter::handle:horizontal {
            width: 4px;
        }
        QTableWidget, QListWidget {
            background-color: #15213c;
            alternate-background-color: #111c35;
            selection-background-color: #29417b;
        }
        QHeaderView::section {
            background: #1d2a45;
            color: #fff2c2;
            border: 1px solid #33476d;
        }
        QLineEdit, QTextEdit, QComboBox, QSpinBox, QDateEdit {
            background-color: #24345a;
            border: 1px solid #48618f;
            color: #f4ebcf;
            padding: 4px;
            selection-background-color: #3c5f9e;
        }
        QTextEdit {
            border-radius: 4px;
        }
        QPushButton {
            background-color: #29457a;
            border: 1px solid #5872aa;
            padding: 5px;
            min-width: 80px;
        }
        QGroupBox {
            border: 1px solid #32486f;
            margin-top: 6px;
            padding: 6px;
        }
        QWidget#CommandInputBar {
            background-color: #111b33;
            border-top: 1px solid #32456a;
        }
        QWidget#CommandInputBar QLabel {
            background-color: transparent;
        }
    )");
}

void AppShell::refreshData() {
    if (!m_adapter) {
        return;
    }

    const EngineAdapterInfo info = m_adapter->info();
    m_topBar->setAdapterInfo(info);
    m_adapter->tick();

    m_previousTickets = m_tickets;
    m_previousLogs = m_auditLogs;

    if (info.connected) {
        m_status = m_adapter->getSystemStatus();
        m_tickets = m_adapter->getTickets();
        m_agents = m_adapter->getAgents();
        m_auditLogs = m_adapter->getAuditLogs();
        m_historyEntries = m_adapter->getHistory();
        m_status.status = "running";
    } else {
        m_status = {"error", 0, 0, 0, 0, 0, 0, 0};
        m_tickets.clear();
        m_agents.clear();
        m_auditLogs.clear();
        m_historyEntries.clear();
    }

    m_soundService.onDataRefresh(m_previousTickets, m_tickets, m_previousLogs, m_auditLogs);

    m_topBar->setSystemStatus(m_status);
    m_dashboard->updateData(m_status, m_agents, m_auditLogs);
    m_ticketBoard->setTickets(m_tickets);
    m_agentList->setAgents(m_agents);
    m_auditLog->setAuditLogs(m_auditLogs);
    m_history->setHistory(m_historyEntries);
    m_settings->setSoundSettings(m_soundService.settings());
    updateErrorIndicators();
    m_systemStatus->setData(m_status, info, m_errorNotes);
    applyCommandInputHints();
    setDetailFromSelected();
}

void AppShell::applyCommandInputHints() {
    if (!m_adapter) {
        return;
    }
    const EngineAdapterInfo info = m_adapter->info();
    m_commandInputBar->setAdapterInfo(info);
    m_commandInputPage->setAdapterInfo(info);
}

void AppShell::updateErrorIndicators() {
    m_errorNotes.clear();
    if (!m_adapter->info().connected) {
        const auto& adapterInfo = m_adapter->info();
        if (adapterInfo.type == EngineAdapterType::Noop) {
            m_errorNotes << "エンジン未接続: mock/noop 接続状態を確認中";
        } else if (adapterInfo.type == EngineAdapterType::Real) {
            m_errorNotes << "エンジン接続未実装: API未接続 (実接続は保留)";
        }
    }
    m_errorNotes << "SEはNo-op実装（再生なし）";
}

void AppShell::setDetailFromSelected() {
    if (m_selectedTicketId.isEmpty() && !m_tickets.isEmpty()) {
        m_selectedTicketId = m_tickets.first().id;
    }
    if (m_selectedTicketId.isEmpty()) {
        m_detailPanel->clear();
        return;
    }

    std::optional<Ticket> found;
    for (const auto& t : m_tickets) {
        if (t.id == m_selectedTicketId) {
            found = t;
            break;
        }
    }
    if (!found.has_value()) {
        m_selectedTicketId.clear();
        m_detailPanel->clear();
        return;
    }
    m_detailPanel->setTicket(found, m_auditLogs);
}

void AppShell::onTicketSelected(const QString& ticketId) {
    m_selectedTicketId = ticketId;
    setDetailFromSelected();
}

void AppShell::onSidebarIndexChanged(int row) {
    m_pages->setCurrentIndex(row);
}

void AppShell::onCommandSubmitRequested(const CreateTicketInput& input) {
    if (!m_adapter) {
        const QString msg = "処理基盤未接続です。";
        m_commandInputBar->setStatusMessage(msg);
        m_commandInputPage->setStatusMessage(msg);
        return;
    }

    const EngineAdapterInfo info = m_adapter->info();
    if (!info.connected || info.type != EngineAdapterType::Mock) {
        if (info.type == EngineAdapterType::Real) {
            const QString msg = "実接続の令札作成は未接続扱いです（本実装は未完了）。";
            m_commandInputBar->setStatusMessage(msg);
            m_commandInputPage->setStatusMessage(msg);
            return;
        }
        const QString msg = "現在の接続ではmock投入を受け付けていません。";
        m_commandInputBar->setStatusMessage(msg);
        m_commandInputPage->setStatusMessage(msg);
        return;
    }

    const auto created = m_adapter->createTicketFromCommand(input);
    if (!created.has_value()) {
        const QString msg = "令札化に失敗しました（mock投入不可）。";
        m_commandInputBar->setStatusMessage(msg);
        m_commandInputPage->setStatusMessage(msg);
        return;
    }

    m_selectedTicketId = created->id;
    refreshData();
    m_commandInputBar->clearInput();
    m_commandInputPage->clearDraft();
    const QString msg = QString("令札%1をmock投入しました。").arg(created->id);
    m_commandInputBar->setStatusMessage(msg);
    m_commandInputPage->setStatusMessage(msg);
    onTicketSelected(created->id);
}

void AppShell::onSettingsApplied(const QVariantMap& settings) {
    SoundCueSettings sound;
    sound.enabled = settings.value("sound.enabled", false).toBool();
    const QString mode = settings.value("sound.mode", "off").toString();
    if (mode == "minimal") {
        sound.mode = SoundCueMode::Minimal;
    } else if (mode == "normal") {
        sound.mode = SoundCueMode::Normal;
    } else if (mode == "gunji") {
        sound.mode = SoundCueMode::Gunji;
    } else {
        sound.mode = SoundCueMode::Off;
    }
    sound.masterVolume = settings.value("sound.master_volume", 0.5).toDouble();
    sound.eventVolume = settings.value("sound.event_volume", 0.4).toDouble();
    sound.errorVolume = settings.value("sound.error_volume", 0.5).toDouble();
    sound.cooldownMs = settings.value("sound.cooldown_ms", 5000).toInt();
    sound.nightMuteEnabled = settings.value("sound.night_mute_enabled", true).toBool();
    sound.nightMuteStart = settings.value("sound.night_mute_start", "22:00").toString();
    sound.nightMuteEnd = settings.value("sound.night_mute_end", "08:00").toString();
    m_soundService.setSettings(sound);

    const int interval = settings.value("ui.refresh_seconds", 3).toInt();
    m_refreshTimer->setInterval(std::max(1, interval) * 1000);
}
} // namespace nexpace
