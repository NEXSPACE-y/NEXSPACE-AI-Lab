#include "app_shell.h"
#include <QListWidget>
#include <QSplitter>
#include <QStackedWidget>
#include <algorithm>
#include <QTimer>
#include <QVBoxLayout>
#include <QVariant>
#include <QLabel>

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

    m_topBar = new TopStatusBar();
    m_sidebar = new QListWidget();
    m_ticketBoard = new TicketBoard();
    m_detailPanel = new TicketDetailPanel();
    m_pages = new QStackedWidget();
    m_dashboard = new DashboardPage();
    m_departmentPage = new DepartmentPage();
    m_agentList = new AgentListPage();
    m_auditLog = new AuditLogTable();
    m_history = new HistoryTable();
    m_settings = new SettingsPage();
    m_systemStatus = new SystemStatusPanel();

    m_sidebar->addItems({"軍議盤", "札場", "部署一覧", "配下一覧", "監察記録", "軍務履歴", "軍令設定", "本陣状態"});
    m_sidebar->setFixedWidth(170);
    m_sidebar->setCurrentRow(0);

    m_pages->addWidget(m_dashboard);
    m_pages->addWidget(m_ticketBoard);
    m_pages->addWidget(m_departmentPage);
    m_pages->addWidget(m_agentList);
    m_pages->addWidget(m_auditLog);
    m_pages->addWidget(m_history);
    m_pages->addWidget(m_settings);
    m_pages->addWidget(m_systemStatus);

    auto* body = new QSplitter(Qt::Horizontal);
    body->setChildrenCollapsible(false);
    body->addWidget(m_sidebar);
    body->addWidget(m_pages);
    body->addWidget(m_detailPanel);
    body->setStretchFactor(0, 1);
    body->setStretchFactor(1, 4);
    body->setStretchFactor(2, 2);

    root->addWidget(m_topBar);
    root->addWidget(body, 1);
    setCentralWidget(container);

    connect(m_sidebar, &QListWidget::currentRowChanged, this, &AppShell::onSidebarIndexChanged);
    connect(m_ticketBoard, &TicketBoard::ticketSelected, this, &AppShell::onTicketSelected);
    connect(m_settings, &SettingsPage::settingsApplied, this, &AppShell::onSettingsApplied);
}

void AppShell::applyStyle() {
    setMinimumSize(1400, 800);
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
        QLineEdit, QComboBox, QSpinBox, QDateEdit {
            background-color: #24345a;
            border: 1px solid #48618f;
            color: #f4ebcf;
            padding: 4px;
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
        m_departments = m_adapter->getDepartments();
        m_agents = m_adapter->getAgents();
        m_auditLogs = m_adapter->getAuditLogs();
        m_historyEntries = m_adapter->getHistory();
        m_departmentEvents = m_adapter->getDepartmentEvents();
        m_departmentRecords = m_adapter->getDepartmentRecords();
        m_status.status = "running";
    } else {
        m_status = {"error", 0, 0, 0, 0, 0, 0, 0};
        m_tickets.clear();
        m_departments.clear();
        m_agents.clear();
        m_auditLogs.clear();
        m_historyEntries.clear();
        m_departmentEvents.clear();
        m_departmentRecords.clear();
    }

    m_soundService.onDataRefresh(m_previousTickets, m_tickets, m_previousLogs, m_auditLogs);

    m_topBar->setSystemStatus(m_status);
    m_dashboard->updateData(m_status, m_agents, m_auditLogs);
    m_ticketBoard->setTickets(m_tickets);
    m_departmentPage->setDepartments(m_departments);
    m_departmentPage->setDepartmentEvents(m_departmentEvents);
    m_departmentPage->setDepartmentRecords(m_departmentRecords);
    m_departmentPage->setTickets(m_tickets);
    m_agentList->setAgents(m_agents);
    m_auditLog->setAuditLogs(m_auditLogs);
    m_history->setHistory(m_historyEntries);
    m_settings->setSoundSettings(m_soundService.settings());
    updateErrorIndicators();
    m_systemStatus->setData(m_status, info, m_errorNotes);
    setDetailFromSelected();
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

} // namespace nexspace
