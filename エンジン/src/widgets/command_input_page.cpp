#include "command_input_page.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace nexspace {

namespace {

QString currentModeLabel(const EngineAdapterInfo& info) {
    if (info.type == EngineAdapterType::Mock) {
        return info.connected ? "mock接続: 令札はmock投入扱い（画面確認用）" : "mock未接続: mock投入不可";
    }
    if (info.type == EngineAdapterType::Real) {
        return "実接続: 令札作成APIは未接続（未対応）";
    }
    return "noop/未接続: 令札作成は未対応";
}

} // namespace

CommandInputPage::CommandInputPage(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    auto* title = new QLabel("軍令入力");
    QFont titleFont = title->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    title->setFont(titleFont);

    m_modeHint = new QLabel("モード: 未設定");
    m_modeHint->setWordWrap(true);
    m_status = new QLabel("");
    m_status->setWordWrap(true);
    m_status->setStyleSheet("color: #c9b67b;");

    auto* content = new QWidget();
    auto* contentLayout = new QHBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(12);

    auto* metaGroup = new QGroupBox("依頼札作成");
    auto* metaLayout = new QVBoxLayout(metaGroup);
    metaLayout->setContentsMargins(12, 18, 12, 12);
    metaLayout->setSpacing(8);

    m_draft = new CommandDraftPanel();
    m_draft->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_submit = new QPushButton("札にする");
    m_clear = new QPushButton("下書きクリア");

    auto* actionGroup = new QGroupBox("札化操作");
    actionGroup->setFixedWidth(300);
    actionGroup->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    auto* actionLayout = new QVBoxLayout(actionGroup);
    actionLayout->setContentsMargins(12, 18, 12, 12);
    actionLayout->setSpacing(8);

    auto* buttonRow = new QHBoxLayout();
    buttonRow->setSpacing(8);
    buttonRow->addWidget(m_submit);
    buttonRow->addWidget(m_clear);

    metaLayout->addWidget(m_draft);
    actionLayout->addLayout(buttonRow);
    actionLayout->addWidget(m_modeHint);
    actionLayout->addWidget(m_status);
    actionLayout->addStretch();

    layout->addWidget(title);
    contentLayout->addWidget(metaGroup, 1);
    contentLayout->addWidget(actionGroup);
    layout->addWidget(content, 1);

    connect(m_submit, &QPushButton::clicked, this, &CommandInputPage::onSubmitClicked);
    connect(m_clear, &QPushButton::clicked, this, &CommandInputPage::onClearClicked);
}

void CommandInputPage::setAdapterInfo(const EngineAdapterInfo& info) {
    refreshModeHint(info);
}

void CommandInputPage::setStatusMessage(const QString& message) {
    m_status->setText(message);
}

void CommandInputPage::setErrorMessage(const QString& message) {
    m_status->setText(message);
}

void CommandInputPage::clearError() {
    m_status->clear();
    m_draft->clearErrorMessage();
}

void CommandInputPage::clearDraft() {
    m_draft->clear();
    clearError();
}

CreateTicketInput CommandInputPage::currentInput() const {
    return m_draft->buildInput();
}

void CommandInputPage::refreshModeHint(const EngineAdapterInfo& info) {
    m_modeHint->setText(QString("モード: %1").arg(currentModeLabel(info)));
}

void CommandInputPage::onSubmitClicked() {
    QString reason;
    if (!m_draft->isValid(&reason)) {
        setErrorMessage(reason);
        return;
    }
    emit submitRequested(currentInput());
}

void CommandInputPage::onClearClicked() {
    clearDraft();
    setStatusMessage("草案をクリアしました。");
}

} // namespace nexspace
