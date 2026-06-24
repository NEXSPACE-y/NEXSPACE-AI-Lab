#pragma once

#include "../adapters/engine_adapter.h"
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

namespace nexspace {

class CommandDraftPanel : public QWidget {
    Q_OBJECT
public:
    explicit CommandDraftPanel(QWidget* parent = nullptr);

    void clear();
    QString titleText() const;
    QString commandText() const;
    bool hasCommandText() const;
    int commandLength() const;
    bool isOverLimit(QString* reason = nullptr) const;
    CreateTicketInput buildInput() const;
    void setModeHint(const QString& text);

private:
    void refreshLengthHint();
    void onTextChanged();

    QTextEdit* m_commandInput = nullptr;
    QLabel* m_commandLabel = nullptr;
    QLabel* m_lengthHint = nullptr;
    QLabel* m_errorHint = nullptr;
};

} // namespace nexspace
