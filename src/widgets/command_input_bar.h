#pragma once

#include "../adapters/engine_adapter.h"
#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>

namespace nexspace {

class CommandInputBar : public QWidget {
    Q_OBJECT
public:
    explicit CommandInputBar(QWidget* parent = nullptr);

    void setAdapterInfo(const EngineAdapterInfo& info);
    void setStatusMessage(const QString& message);
    void clearInput();
    void setModeHint(const QString& text);
    QString trimmedCommandText() const;

signals:
    void submitRequested(const CreateTicketInput& input);

private slots:
    void onSubmitClicked();
    void refreshLengthHint();

private:
    CreateTicketInput buildInput() const;

    QTextEdit* m_input = nullptr;
    QPushButton* m_submitButton = nullptr;
    QLabel* m_modeHint = nullptr;
    QLabel* m_status = nullptr;
    QLabel* m_lengthHint = nullptr;
};

} // namespace nexspace

