#pragma once

#include "../sound/soundCueTypes.h"
#include <QVariantMap>
#include <QWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace nexspace {

class SettingsPage : public QWidget {
    Q_OBJECT
public:
    explicit SettingsPage(QWidget* parent = nullptr);

    SoundCueSettings soundSettings() const;
    void setSoundSettings(const SoundCueSettings& settings);
    int autoRefreshSeconds() const;

signals:
    void settingsApplied(const QVariantMap& settings);

private:
    QVBoxLayout* m_rootLayout = nullptr;
    void setupEngineGroup();
    void setupAuditorGroup();
    void setupUiGroup();
    void setupStorageGroup();
    void setupSoundGroup();
    void emitCurrentSettings();
    QVariantMap buildSettingsMap() const;

    QCheckBox* m_engineEnabled = nullptr;
    QCheckBox* m_autoProcess = nullptr;
    QSpinBox* m_auditStrength = nullptr;
    QSpinBox* m_maxConcurrent = nullptr;
    QSpinBox* m_timeoutSec = nullptr;

    QCheckBox* m_auditorEnabled = nullptr;
    QCheckBox* m_warnStop = nullptr;
    QCheckBox* m_autoReprocessReject = nullptr;
    QSpinBox* m_auditLogRetention = nullptr;

    QCheckBox* m_uiDark = nullptr;
    QCheckBox* m_compact = nullptr;
    QSpinBox* m_refreshSec = nullptr;
    QCheckBox* m_notify = nullptr;

    QLineEdit* m_ticketStorage = nullptr;
    QLineEdit* m_logStorage = nullptr;
    QSpinBox* m_historyRetention = nullptr;
    QPushButton* m_exportButton = nullptr;

    QCheckBox* m_soundEnabled = nullptr;
    QComboBox* m_soundMode = nullptr;
    QDoubleSpinBox* m_masterVol = nullptr;
    QDoubleSpinBox* m_eventVol = nullptr;
    QDoubleSpinBox* m_errorVol = nullptr;
    QSpinBox* m_cooldown = nullptr;
    QCheckBox* m_nightMute = nullptr;
    QLineEdit* m_nightStart = nullptr;
    QLineEdit* m_nightEnd = nullptr;
};

} // namespace nexspace
