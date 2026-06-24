#pragma once

#include "../adapters/engine_adapter.h"
#include "command_draft_panel.h"
#include <QWidget>
#include <QLabel>
#include <QPushButton>

namespace nexspace {

class CommandInputPage : public QWidget {
    Q_OBJECT
public:
    explicit CommandInputPage(QWidget* parent = nullptr);

    void setAdapterInfo(const EngineAdapterInfo& info);
    void clearDraft();
    CreateTicketInput currentInput() const;

signals:
    void submitRequested(const CreateTicketInput& input);

public slots:
    void setStatusMessage(const QString& message);

private slots:
    void onSubmitClicked();
    void onClearClicked();

private:
    void refreshModeHint(const EngineAdapterInfo& info);
    void setErrorMessage(const QString& message);
    void clearError();

    QLabel* m_modeHint = nullptr;
    QLabel* m_status = nullptr;
    CommandDraftPanel* m_draft = nullptr;
    QPushButton* m_submit = nullptr;
    QPushButton* m_clear = nullptr;
};

} // namespace nexspace

