#include "command_input_bar.h"
#include "../domain.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace nexspace {

namespace {
QString modeHintText(const EngineAdapterInfo& info) {
    if (!info.connected) {
        if (info.type == EngineAdapterType::Noop) {
            return "未接続: Noop/Mock未接続（mock投入不可）";
        }
        if (info.type == EngineAdapterType::Real) {
            return "実接続未実装: この画面では未対応のため令札化は保留";
        }
        return "未接続: mock/noop";
    }
    if (info.type == EngineAdapterType::Mock) {
        return "Mock接続: 画面投入は「mock投入」です";
    }
    if (info.type == EngineAdapterType::Real) {
        return "実接続モード: 現状、令札作成API未実装";
    }
    return "接続状態不明";
}
}

CommandInputBar::CommandInputBar(QWidget* parent)
    : QWidget(parent) {
    setObjectName("CommandInputBar");
    setFixedHeight(104);

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(12, 8, 12, 8);
    root->setSpacing(10);

    auto* title = new QLabel("軍令入力");
    m_modeHint = new QLabel("接続状態: 不明");
    m_modeHint->setWordWrap(true);
    m_modeHint->setMaximumWidth(260);

    m_input = new QTextEdit();
    m_input->setPlaceholderText("軍令を入力...");
    m_input->setAcceptRichText(false);
    m_input->setMinimumHeight(48);
    m_input->setMaximumHeight(68);
    m_input->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_submitButton = new QPushButton("札にする");
    m_submitButton->setFixedWidth(96);

    m_lengthHint = new QLabel();
    m_status = new QLabel();
    m_status->setStyleSheet("color: #c9b67b;");
    m_status->setWordWrap(true);
    m_status->setMaximumWidth(240);

    auto* labelColumn = new QWidget();
    auto* labelLayout = new QVBoxLayout(labelColumn);
    labelLayout->setContentsMargins(0, 0, 0, 0);
    labelLayout->setSpacing(3);
    labelLayout->addWidget(title);
    labelLayout->addWidget(m_modeHint);
    labelLayout->addStretch();
    labelColumn->setFixedWidth(270);

    auto* actionColumn = new QWidget();
    auto* actionLayout = new QVBoxLayout(actionColumn);
    actionLayout->setContentsMargins(0, 0, 0, 0);
    actionLayout->setSpacing(4);
    actionLayout->addWidget(m_submitButton);
    actionLayout->addWidget(m_lengthHint);
    actionLayout->addWidget(m_status);
    actionLayout->addStretch();
    actionColumn->setFixedWidth(250);

    root->addWidget(labelColumn);
    root->addWidget(m_input, 1);
    root->addWidget(actionColumn);

    connect(m_input, &QTextEdit::textChanged, this, &CommandInputBar::refreshLengthHint);
    connect(m_submitButton, &QPushButton::clicked, this, &CommandInputBar::onSubmitClicked);
    refreshLengthHint();
}

void CommandInputBar::clearInput() {
    m_input->clear();
    m_status->clear();
    m_submitButton->setEnabled(true);
    refreshLengthHint();
}

void CommandInputBar::setAdapterInfo(const EngineAdapterInfo& info) {
    setHintText(modeHintText(info));
}

void CommandInputBar::setStatusMessage(const QString& message) {
    m_status->setText(message);
}

void CommandInputBar::setHintText(const QString& text) {
    m_modeHint->setText(text);
}

void CommandInputBar::refreshLengthHint() {
    const int length = m_input->toPlainText().size();
    const bool tooLong = length > kMaxCommandTextLength;
    m_lengthHint->setText(QString("文字数: %1 / %2").arg(length).arg(kMaxCommandTextLength));
    m_lengthHint->setStyleSheet(tooLong ? "color: #f5a55a;" : "color: #cfc09f;");
    m_submitButton->setEnabled(!tooLong);
}

QString CommandInputBar::trimmedCommandText() const {
    return m_input->toPlainText().trimmed();
}

CreateTicketInput CommandInputBar::buildInput() const {
    CreateTicketInput input;
    input.commandText = trimmedCommandText();
    input.title = std::nullopt;
    input.type = "依頼札";
    input.priority = TicketPriority::normal;
    input.auditRequired = false;
    input.requestedAgentRole = std::nullopt;
    return input;
}

void CommandInputBar::onSubmitClicked() {
    m_status->clear();
    const QString command = trimmedCommandText();
    if (command.isEmpty()) {
        m_status->setText("依頼文が空です。");
        return;
    }
    if (command.size() > kMaxCommandTextLength) {
        m_status->setText(QString("依頼文が長すぎます（%1字上限）。").arg(kMaxCommandTextLength));
        return;
    }
    emit submitRequested(buildInput());
}

} // namespace nexspace
