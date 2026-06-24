#include "command_draft_panel.h"
#include <QVariant>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace nexspace {

QString roleText(AgentRole role) {
    switch (role) {
        case AgentRole::Orchestrator:
            return "軍師";
        case AgentRole::Worker:
            return "実行隊";
        case AgentRole::Auditor:
            return "監察官";
        case AgentRole::Observer:
            return "物見";
        case AgentRole::System:
            return "本陣";
        default:
            return "";
    }
}

CommandDraftPanel::CommandDraftPanel(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    auto* form = new QFormLayout();

    m_titleInput = new QLineEdit("依頼札");
    m_commandInput = new QTextEdit();
    m_typeInput = new QComboBox();
    m_priorityInput = new QComboBox();
    m_agentInput = new QComboBox();
    m_auditRequired = new QCheckBox();
    m_lengthHint = new QLabel();
    m_errorHint = new QLabel();

    m_commandInput->setPlaceholderText("依頼本文を入力...");
    m_commandInput->setMinimumHeight(110);
    m_commandInput->setMaximumHeight(160);

    m_typeInput->addItem("依頼札");
    m_typeInput->addItem("監査票");
    m_typeInput->addItem("割当札");
    m_typeInput->addItem("業務文面");

    m_priorityInput->addItem("低", QVariant::fromValue(static_cast<int>(TicketPriority::low)));
    m_priorityInput->addItem("中", QVariant::fromValue(static_cast<int>(TicketPriority::normal)));
    m_priorityInput->addItem("高", QVariant::fromValue(static_cast<int>(TicketPriority::high)));
    m_priorityInput->addItem("最優先", QVariant::fromValue(static_cast<int>(TicketPriority::critical)));
    m_priorityInput->setCurrentIndex(1);

    m_agentInput->addItem("未割当", QVariant());
    m_agentInput->addItem(QString("軍師 (%1)").arg(roleText(AgentRole::Orchestrator)),
                          QVariant::fromValue(static_cast<int>(AgentRole::Orchestrator)));
    m_agentInput->addItem(QString("実行隊 (%1)").arg(roleText(AgentRole::Worker)),
                          QVariant::fromValue(static_cast<int>(AgentRole::Worker)));
    m_agentInput->addItem(QString("監察官 (%1)").arg(roleText(AgentRole::Auditor)),
                          QVariant::fromValue(static_cast<int>(AgentRole::Auditor)));
    m_agentInput->addItem(QString("物見 (%1)").arg(roleText(AgentRole::Observer)),
                          QVariant::fromValue(static_cast<int>(AgentRole::Observer)));
    m_agentInput->addItem(QString("本陣 (%1)").arg(roleText(AgentRole::System)),
                          QVariant::fromValue(static_cast<int>(AgentRole::System)));
    m_auditRequired->setChecked(true);

    form->addRow("タイトル", m_titleInput);
    form->addRow("依頼本文", m_commandInput);
    form->addRow("種別", m_typeInput);
    form->addRow("優先度", m_priorityInput);
    form->addRow("担当候補", m_agentInput);
    form->addRow("監察必須", m_auditRequired);

    layout->addLayout(form);
    layout->addWidget(m_lengthHint);
    layout->addWidget(m_errorHint);
    refreshLengthHint();

    connect(m_commandInput, &QTextEdit::textChanged, this, &CommandDraftPanel::refreshLengthHint);
}

void CommandDraftPanel::clear() {
    m_titleInput->setText("依頼札");
    m_commandInput->clear();
    m_typeInput->setCurrentText("依頼札");
    m_priorityInput->setCurrentIndex(1);
    m_agentInput->setCurrentIndex(0);
    m_auditRequired->setChecked(true);
    m_errorHint->clear();
    refreshLengthHint();
}

void CommandDraftPanel::setModeHint(const QString& text) {
    Q_UNUSED(text);
}

bool CommandDraftPanel::isValid(QString* reason) const {
    const QString command = commandText();
    if (command.isEmpty()) {
        if (reason) {
            *reason = "依頼本文が空です。";
        }
        return false;
    }
    if (command.size() > kMaxCommandTextLength) {
        if (reason) {
            *reason = QString("依頼文が長すぎます（上限%1字）。").arg(kMaxCommandTextLength);
        }
        return false;
    }
    if (reason) {
        reason->clear();
    }
    return true;
}

QString CommandDraftPanel::titleText() const {
    return m_titleInput->text().trimmed();
}

QString CommandDraftPanel::commandText() const {
    return m_commandInput->toPlainText().trimmed();
}

TicketPriority CommandDraftPanel::selectedPriority() const {
    const int index = m_priorityInput->currentIndex();
    return static_cast<TicketPriority>(m_priorityInput->itemData(index).toInt());
}

std::optional<AgentRole> CommandDraftPanel::selectedAgentRole() const {
    const QVariant roleValue = m_agentInput->currentData();
    if (!roleValue.isValid()) {
        return std::nullopt;
    }
    const int roleId = roleValue.toInt();
    if (roleId < 0) {
        return std::nullopt;
    }
    return static_cast<AgentRole>(roleId);
}

bool CommandDraftPanel::isAuditRequired() const {
    return m_auditRequired->isChecked();
}

void CommandDraftPanel::setErrorMessage(const QString& message) const {
    m_errorHint->setText(message);
}

void CommandDraftPanel::clearErrorMessage() {
    m_errorHint->clear();
}

int CommandDraftPanel::commandLength() const {
    return commandText().size();
}

CreateTicketInput CommandDraftPanel::buildInput() const {
    CreateTicketInput input;
    input.title = titleText();
    input.commandText = commandText();
    input.type = m_typeInput->currentText();
    input.priority = selectedPriority();
    input.requestedAgentRole = selectedAgentRole();
    input.auditRequired = isAuditRequired();
    return input;
}

void CommandDraftPanel::refreshLengthHint() {
    const int length = m_commandInput->toPlainText().size();
    m_lengthHint->setText(QString("文字数: %1 / %2").arg(length).arg(kMaxCommandTextLength));
    m_errorHint->setText(length > kMaxCommandTextLength ? QString("最大文字数を超えています（%1字）。").arg(kMaxCommandTextLength) : "");
}

} // namespace nexspace
