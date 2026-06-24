#include "command_input_bar.h"
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace nexspace {

CommandInputBar::CommandInputBar(QWidget* parent)
    : QWidget(parent) {
    setObjectName("CommandInputBar");
    setFixedHeight(104);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(10);

    m_input = new QTextEdit();
    m_input->setPlaceholderText("軍令を入力...");
    m_input->setAcceptRichText(false);
    m_input->setMinimumHeight(48);
    m_input->setMaximumHeight(68);
    m_input->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_submitButton = new QPushButton("札にする");
    m_submitButton->setFixedWidth(96);

    m_modeHint = new QLabel("令札作成モード: 未設定");
    m_modeHint->setWordWrap(true);
    m_modeHint->setMaximumWidth(260);

    m_status = new QLabel("");
    m_status->setWordWrap(true);
    m_status->setMaximumWidth(240);

    m_lengthHint = new QLabel(QString("文字数: 0 / %1").arg(kMaxCommandTextLength));

    auto* labelColumn = new QWidget();
    auto* labelLayout = new QVBoxLayout(labelColumn);
    labelLayout->setContentsMargins(0, 0, 0, 0);
    labelLayout->setSpacing(3);
    labelLayout->addWidget(new QLabel("軍令入力"));
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

    layout->addWidget(labelColumn);
    layout->addWidget(m_input, 1);
    layout->addWidget(actionColumn);
    setStatusMessage("");

    connect(m_submitButton, &QPushButton::clicked, this, &CommandInputBar::onSubmitClicked);
    connect(m_input, &QTextEdit::textChanged, this, &CommandInputBar::refreshLengthHint);
}

void CommandInputBar::setAdapterInfo(const EngineAdapterInfo& info) {
    if (info.type == EngineAdapterType::Mock) {
        if (info.connected) {
            setModeHint("mock投入モード: 令札は画面確認用としてmockに投入されます");
        } else {
            setModeHint("mock未接続: mock投入は保留");
        }
    } else if (info.type == EngineAdapterType::Real) {
        setModeHint("実接続: 令札作成APIは本接続未対応（未完成）");
    } else {
        setModeHint("noop/未接続: 令札投入は実処理されません");
    }
}

void CommandInputBar::setStatusMessage(const QString& message) {
    m_status->setText(message);
}

void CommandInputBar::setModeHint(const QString& text) {
    m_modeHint->setText(QString("モード: %1").arg(text));
}

QString CommandInputBar::trimmedCommandText() const {
    return m_input->toPlainText().trimmed();
}

void CommandInputBar::clearInput() {
    m_input->clear();
    m_status->clear();
    refreshLengthHint();
}

void CommandInputBar::refreshLengthHint() {
    const int length = m_input->toPlainText().size();
    const bool tooLong = length > kMaxCommandTextLength;
    m_lengthHint->setText(QString("文字数: %1 / %2").arg(length).arg(kMaxCommandTextLength));
    m_lengthHint->setStyleSheet(tooLong ? "color: #f5a55a;" : "color: #cfc09f;");
    m_submitButton->setEnabled(!tooLong);
}

CreateTicketInput CommandInputBar::buildInput() const {
    CreateTicketInput input;
    input.commandText = m_input->toPlainText().trimmed();
    input.type = "依頼札";
    input.priority = TicketPriority::normal;
    input.auditRequired = true;
    return input;
}

void CommandInputBar::onSubmitClicked() {
    const QString command = trimmedCommandText();
    if (command.isEmpty()) {
        setStatusMessage("依頼文が空です。");
        return;
    }
    if (command.size() > kMaxCommandTextLength) {
        setStatusMessage(QString("依頼文が長すぎます（上限%1字）").arg(kMaxCommandTextLength));
        return;
    }
    emit submitRequested(buildInput());
}

} // namespace nexspace
