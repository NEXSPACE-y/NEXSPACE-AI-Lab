#include "command_input_page.h"
#include <QFormLayout>
#include <QGroupBox>
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

    auto* title = new QLabel("軍令入力");
    QFont titleFont = title->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    title->setFont(titleFont);

    auto* metaGroup = new QGroupBox("依頼札作成");
    auto* metaLayout = new QVBoxLayout(metaGroup);
    m_draft = new CommandDraftPanel();

    m_submit = new QPushButton("札にする");
    m_clear = new QPushButton("下書きクリア");

    auto* buttonRow = new QVBoxLayout();
    buttonRow->addWidget(m_submit);
    buttonRow->addWidget(m_clear);

    m_modeHint = new QLabel("モード: 未設定");
    m_status = new QLabel("");

    metaLayout->addWidget(m_draft);
    metaLayout->addLayout(buttonRow);
    metaLayout->addWidget(m_modeHint);
    metaLayout->addWidget(m_status);

    layout->addWidget(title);
    layout->addWidget(metaGroup, 1);

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

