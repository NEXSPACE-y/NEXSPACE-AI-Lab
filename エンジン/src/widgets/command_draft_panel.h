#pragma once

#include "../adapters/engine_adapter.h"
#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>

namespace nexspace {

class CommandDraftPanel : public QWidget {
    Q_OBJECT
public:
    explicit CommandDraftPanel(QWidget* parent = nullptr);

    void clear();
    void setModeHint(const QString& text);
    bool isValid(QString* reason = nullptr) const;
    QString titleText() const;
    QString commandText() const;
    TicketPriority selectedPriority() const;
    std::optional<AgentRole> selectedAgentRole() const;
    bool isAuditRequired() const;
    void setErrorMessage(const QString& message) const;
    int commandLength() const;
    CreateTicketInput buildInput() const;

    void clearErrorMessage();

private:
    void refreshLengthHint();

    QLineEdit* m_titleInput = nullptr;
    QTextEdit* m_commandInput = nullptr;
    QComboBox* m_typeInput = nullptr;
    QComboBox* m_priorityInput = nullptr;
    QComboBox* m_agentInput = nullptr;
    QCheckBox* m_auditRequired = nullptr;
    QLabel* m_lengthHint = nullptr;
    QLabel* m_errorHint = nullptr;
};

} // namespace nexspace

