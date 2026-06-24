#include "command_input_page.h"
#include "../domain.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace nexspace {
namespace {
QString modeHintText(const EngineAdapterInfo& info) {
    if (!info.connected) {
        if (info.type == EngineAdapterType::Mock) {
            return "未接続: Mock未接続（投入不可）";
        }
        if (info.type == EngineAdapterType::Noop) {
            return "未接続: Noop接続（投入不可）";
        }
        if (info.type == EngineAdapterType::Real) {
            return "実接続未接続: RealEngineAdapterは令札作成未実装";
        }
    }
    if (info.type == EngineAdapterType::Mock && info.connected) {
        return "mock接続: 令札作成は画面上でmock投入として反映";
    }
    if (info.type == EngineAdapterType::Real) {
        return "実接続: 令札作成APIは本実装未接続（mock投入不可）";
    }
    return "接続状態を確認してください";
}
}

CommandInputPage::CommandInputPage(QWidget* parent)
    : QWidget(parent) {
    buildControls();
}

void CommandInputPage::buildControls() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);

    m_modeHint = new QLabel("接続状態: -");
    m_modeHint->setWordWrap(true);

    auto* content = new QWidget();
    auto* contentLayout = new QHBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(12);

    m_draft = new CommandDraftPanel();
    m_draft->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* metaGroup = new QGroupBox("依頼札作成");
    metaGroup->setFixedWidth(330);
    metaGroup->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    auto* metaLayout = new QFormLayout(metaGroup);
    metaLayout->setContentsMargins(12, 18, 12, 12);
    metaLayout->setSpacing(8);
    metaLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    m_typeInput = new QLineEdit("依頼札");
    m_priorityCombo = new QComboBox();
    m_priorityCombo->addItem("低", "low");
    m_priorityCombo->addItem("中", "normal");
    m_priorityCombo->addItem("高", "high");
    m_priorityCombo->addItem("最優先", "critical");
    m_priorityCombo->setCurrentText("中");

    m_agentRoleCombo = new QComboBox();
    m_agentRoleCombo->addItem("未割当");
    m_agentRoleCombo->addItem("上位役:軍師");
    m_agentRoleCombo->addItem("上位役:実行隊");
    m_agentRoleCombo->addItem("上位役:監察官");
    m_agentRoleCombo->addItem("上位役:物見");
    m_agentRoleCombo->addItem("上位役:本陣");

    m_auditRequired = new QCheckBox("監察必須");
    m_auditRequired->setChecked(true);
    m_submit = new QPushButton("札にする");
    m_clear = new QPushButton("下書きクリア");

    metaLayout->addRow("種別", m_typeInput);
    metaLayout->addRow("優先度", m_priorityCombo);
    metaLayout->addRow("担当候補", m_agentRoleCombo);
    metaLayout->addRow("監察必須", m_auditRequired);

    auto* buttonRow = new QWidget();
    auto* buttonLayout = new QHBoxLayout(buttonRow);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(8);
    buttonLayout->addWidget(m_submit);
    buttonLayout->addWidget(m_clear);
    buttonLayout->addStretch();

    metaLayout->addRow("アクション", buttonRow);

    m_status = new QLabel();
    m_status->setStyleSheet("color: #c9b67b;");
    m_status->setWordWrap(true);

    contentLayout->addWidget(m_draft, 1);
    contentLayout->addWidget(metaGroup);
    root->addWidget(m_modeHint);
    root->addWidget(content, 1);
    root->addWidget(m_status);

    connect(m_submit, &QPushButton::clicked, this, &CommandInputPage::onSubmitClicked);
    connect(m_clear, &QPushButton::clicked, this, &CommandInputPage::onClearClicked);
}

void CommandInputPage::onSubmitClicked() {
    m_status->clear();
    const auto input = currentInput();
    if (input.commandText.isEmpty()) {
        setErrorMessage("依頼本文が空です。");
        return;
    }
    QString reason;
    if (m_draft->isOverLimit(&reason)) {
        setErrorMessage(reason);
        return;
    }
    emit submitRequested(input);
}

void CommandInputPage::onClearClicked() {
    clearDraft();
    m_status->setText("下書きをクリアしました。");
}

void CommandInputPage::clearDraft() {
    m_typeInput->setText("依頼札");
    m_priorityCombo->setCurrentText("中");
    m_agentRoleCombo->setCurrentIndex(0);
    m_auditRequired->setChecked(true);
    m_draft->clear();
    m_status->clear();
    m_draft->setModeHint("草案");
}

void CommandInputPage::setAdapterInfo(const EngineAdapterInfo& info) {
    refreshModeHint(info);
}

void CommandInputPage::refreshModeHint(const EngineAdapterInfo& info) {
    m_modeHint->setText(modeHintText(info));
    m_draft->setModeHint(info.type == EngineAdapterType::Mock && info.connected ? "mock投入" : "待機中");
}

void CommandInputPage::setStatusMessage(const QString& message) {
    m_status->setText(message);
}

void CommandInputPage::setErrorMessage(const QString& message) {
    m_status->setText(message);
}

CreateTicketInput CommandInputPage::currentInput() const {
    CreateTicketInput input;
    const QString rawType = m_typeInput->text().trimmed();
    input.type = rawType.isEmpty() ? "依頼札" : rawType;
    input.commandText = m_draft->commandText();
    input.title = std::nullopt;
    input.priority = selectedPriority();
    input.requestedAgentRole = selectedAgentRole();
    input.auditRequired = m_auditRequired->isChecked();
    return input;
}

TicketPriority CommandInputPage::selectedPriority() const {
    const QString value = m_priorityCombo->currentData().toString();
    if (value == "low") return TicketPriority::low;
    if (value == "high") return TicketPriority::high;
    if (value == "critical") return TicketPriority::critical;
    return TicketPriority::normal;
}

std::optional<AgentRole> CommandInputPage::selectedAgentRole() const {
    const int index = m_agentRoleCombo->currentIndex();
    if (index <= 0) {
        return std::nullopt;
    }
    switch (index) {
        case 1: return AgentRole::Orchestrator;
        case 2: return AgentRole::Worker;
        case 3: return AgentRole::Auditor;
        case 4: return AgentRole::Observer;
        case 5: return AgentRole::System;
        default: return std::nullopt;
    }
}

} // namespace nexpace
