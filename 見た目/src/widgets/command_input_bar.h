#pragma once

#include "../adapters/engine_adapter.h"
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

namespace nexspace {

class CommandInputBar : public QWidget {
    Q_OBJECT
public:
    explicit CommandInputBar(QWidget* parent = nullptr);

    void clearInput();
    void setAdapterInfo(const EngineAdapterInfo& info);
    void setStatusMessage(const QString& message);

signals:
    void submitRequested(const CreateTicketInput& input);

private:
    void onSubmitClicked();
    void refreshLengthHint();
    void setHintText(const QString& text);
    QString trimmedCommandText() const;
    CreateTicketInput buildInput() const;

    QTextEdit* m_input = nullptr;
    QPushButton* m_submitButton = nullptr;
    QLabel* m_modeHint = nullptr;
    QLabel* m_lengthHint = nullptr;
    QLabel* m_status = nullptr;
};

} // namespace nexspace
