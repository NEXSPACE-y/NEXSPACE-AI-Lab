#include "command_input_bar.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace nexspace {

CommandInputBar::CommandInputBar(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    auto* row = new QHBoxLayout();
    m_input = new QTextEdit();
    m_input->setPlaceholderText("軍令を入力...");
    m_input->setMinimumHeight(60);
    m_input->setMaximumHeight(120);
    m_submitButton = new QPushButton("札にする");
    m_modeHint = new QLabel("令札作成モード: 未設定");
    m_status = new QLabel("");
    m_lengthHint = new QLabel(QString("文字数: 0 / %1").arg(kMaxCommandTextLength));

    row->addWidget(m_input, 1);
    row->addWidget(m_submitButton);

    layout->addWidget(new QLabel("下部入力（簡易軍令入力）"));
    layout->addLayout(row);
    layout->addWidget(m_modeHint);
    layout->addWidget(m_lengthHint);
    layout->addWidget(m_status);
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
    m_lengthHint->setText(QString("文字数: %1 / %2").arg(length).arg(kMaxCommandTextLength));
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

