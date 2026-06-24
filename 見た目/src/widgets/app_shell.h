#pragma once

#include "../adapters/engine_adapter.h"
#include "../adapters/mock_engine_adapter.h"
#include "../adapters/noop_engine_adapter.h"
#include "../adapters/real_engine_adapter.h"
#include "../sound/noopSoundCueAdapter.h"
#include "../sound/soundCueService.h"
#include "agent_list_page.h"
#include "audit_log_table.h"
#include "dashboard_page.h"
#include "history_table.h"
#include "settings_page.h"
#include "system_status_panel.h"
#include "ticket_board.h"
#include "ticket_detail_panel.h"
#include "command_input_bar.h"
#include "command_input_page.h"
#include "top_status_bar.h"
#include <memory>
#include <QVariant>
#include <QMainWindow>
#include <QListWidget>
#include <QSplitter>
#include <QStackedWidget>
#include <QTimer>

namespace nexspace {

class AppShell : public QMainWindow {
    Q_OBJECT
public:
    explicit AppShell(std::unique_ptr<EngineAdapter> adapter, QWidget* parent = nullptr);

private slots:
    void refreshData();
    void onTicketSelected(const QString& ticketId);
    void onSidebarIndexChanged(int row);
    void onSettingsApplied(const QVariantMap& settings);
    void onCommandSubmitRequested(const CreateTicketInput& input);

private:
    void buildUi();
    void applyStyle();
    void updateErrorIndicators();
    void setDetailFromSelected();
    void applyCommandInputHints();

    std::unique_ptr<EngineAdapter> m_adapter;
    NoopSoundCueAdapter m_soundCueAdapter;
    SoundCueService m_soundService;

    TopStatusBar* m_topBar = nullptr;
    QListWidget* m_sidebar = nullptr;
    QStackedWidget* m_pages = nullptr;
    TicketBoard* m_ticketBoard = nullptr;
    TicketDetailPanel* m_detailPanel = nullptr;
    DashboardPage* m_dashboard = nullptr;
    AgentListPage* m_agentList = nullptr;
    AuditLogTable* m_auditLog = nullptr;
    HistoryTable* m_history = nullptr;
    SettingsPage* m_settings = nullptr;
    SystemStatusPanel* m_systemStatus = nullptr;
    QSplitter* m_bodySplitter = nullptr;
    CommandInputBar* m_commandInputBar = nullptr;
    CommandInputPage* m_commandInputPage = nullptr;

    QVector<Ticket> m_previousTickets;
    QVector<Ticket> m_tickets;
    QVector<Agent> m_agents;
    QVector<AuditLog> m_previousLogs;
    QVector<AuditLog> m_auditLogs;
    QVector<HistoryEntry> m_historyEntries;
    SystemStatus m_status;
    QString m_selectedTicketId;
    QStringList m_errorNotes;
    QTimer* m_refreshTimer = nullptr;
};

} // namespace nexspace
