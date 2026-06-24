#include "command_draft_panel.h"
#include "../domain.h"
#include <QSizePolicy>
#include <QVBoxLayout>

namespace nexspace {

CommandDraftPanel::CommandDraftPanel(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    m_commandLabel = new QLabel("草案");
    m_commandInput = new QTextEdit();
    m_commandInput->setPlaceholderText("依頼本文を入力...");
    m_commandInput->setAcceptRichText(false);
    m_commandInput->setMinimumHeight(260);
    m_commandInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_lengthHint = new QLabel();
    m_errorHint = new QLabel();
    m_errorHint->setStyleSheet("color: #f5a55a;");
    layout->addWidget(m_commandLabel);
    layout->addWidget(m_commandInput);
    layout->addWidget(m_lengthHint);
    layout->addWidget(m_errorHint);
    connect(m_commandInput, &QTextEdit::textChanged, this, &CommandDraftPanel::onTextChanged);
    refreshLengthHint();
}

void CommandDraftPanel::clear() {
    m_commandInput->clear();
    m_errorHint->clear();
    refreshLengthHint();
}

QString CommandDraftPanel::titleText() const {
    return m_commandInput->toPlainText().trimmed();
}

QString CommandDraftPanel::commandText() const {
    return m_commandInput->toPlainText().trimmed();
}

bool CommandDraftPanel::hasCommandText() const {
    return !m_commandInput->toPlainText().trimmed().isEmpty();
}

int CommandDraftPanel::commandLength() const {
    return m_commandInput->toPlainText().size();
}

bool CommandDraftPanel::isOverLimit(QString* reason) const {
    const int length = commandLength();
    if (length <= kMaxCommandTextLength) {
        return false;
    }
    if (reason != nullptr) {
        *reason = QString("本文が長すぎます（%1字上限）").arg(kMaxCommandTextLength);
    }
    return true;
}

void CommandDraftPanel::onTextChanged() {
    refreshLengthHint();
}

void CommandDraftPanel::refreshLengthHint() {
    const int length = commandLength();
    const bool tooLong = length > kMaxCommandTextLength;
    m_lengthHint->setText(QString("文字数: %1 / %2").arg(length).arg(kMaxCommandTextLength));
    m_lengthHint->setStyleSheet(tooLong ? "color: #f5a55a;" : "color: #cfc09f;");
    m_errorHint->setText(tooLong ? QString("最大文字数を超えています（%1字）。").arg(kMaxCommandTextLength) : "");
}

CreateTicketInput CommandDraftPanel::buildInput() const {
    CreateTicketInput input;
    const QString body = commandText();
    input.commandText = body;
    input.title = std::nullopt;
    input.type = "依頼札";
    input.priority = TicketPriority::normal;
    input.auditRequired = false;
    input.requestedAgentRole = std::nullopt;
    return input;
}

void CommandDraftPanel::setModeHint(const QString& text) {
    m_commandLabel->setText(QString("草案: %1").arg(text));
}

} // namespace nexpace
