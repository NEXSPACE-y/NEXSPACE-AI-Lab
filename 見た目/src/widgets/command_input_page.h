#pragma once

#include "../adapters/engine_adapter.h"
#include "command_draft_panel.h"
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

namespace nexspace {

class CommandInputPage : public QWidget {
    Q_OBJECT
public:
    explicit CommandInputPage(QWidget* parent = nullptr);

    void clearDraft();
    void setAdapterInfo(const EngineAdapterInfo& info);
    void setStatusMessage(const QString& message);

signals:
    void submitRequested(const CreateTicketInput& input);

private slots:
    void onSubmitClicked();
    void onClearClicked();

private:
    CreateTicketInput currentInput() const;
    void refreshModeHint(const EngineAdapterInfo& info);
    void setErrorMessage(const QString& message);

    void buildControls();
    TicketPriority selectedPriority() const;
    std::optional<AgentRole> selectedAgentRole() const;

    CommandDraftPanel* m_draft = nullptr;
    QLineEdit* m_typeInput = nullptr;
    QComboBox* m_priorityCombo = nullptr;
    QComboBox* m_agentRoleCombo = nullptr;
    QCheckBox* m_auditRequired = nullptr;
    QPushButton* m_submit = nullptr;
    QPushButton* m_clear = nullptr;
    QLabel* m_modeHint = nullptr;
    QLabel* m_status = nullptr;
};

} // namespace nexspace
