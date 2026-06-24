#include "settings_page.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>

namespace nexspace {

SettingsPage::SettingsPage(QWidget* parent)
    : QWidget(parent) {
    m_rootLayout = new QVBoxLayout(this);
    setupEngineGroup();
    setupAuditorGroup();
    setupUiGroup();
    setupStorageGroup();
    setupSoundGroup();

    auto* apply = new QPushButton("設定を反映");
    auto* note = new QLabel("SE音源は導線のみ。音源未接続で再生なし。");
    m_rootLayout->addWidget(apply);
    m_rootLayout->addWidget(note);
    m_rootLayout->addStretch();

    connect(apply, &QPushButton::clicked, this, &SettingsPage::emitCurrentSettings);
}

void SettingsPage::setupEngineGroup() {
    auto* group = new QGroupBox("軍令設定");
    auto* layout = new QFormLayout(group);

    m_engineEnabled = new QCheckBox("エンジン ON");
    m_autoProcess = new QCheckBox("自動処理 ON");
    m_auditStrength = new QSpinBox();
    m_auditStrength->setRange(1, 5);
    m_auditStrength->setValue(3);
    m_maxConcurrent = new QSpinBox();
    m_maxConcurrent->setRange(1, 16);
    m_maxConcurrent->setValue(4);
    m_timeoutSec = new QSpinBox();
    m_timeoutSec->setRange(1, 300);
    m_timeoutSec->setValue(30);

    layout->addRow("エンジン稼働", m_engineEnabled);
    layout->addRow("自動処理", m_autoProcess);
    layout->addRow("監察強度", m_auditStrength);
    layout->addRow("最大同時処理数", m_maxConcurrent);
    layout->addRow("タイムアウト秒数", m_timeoutSec);

    m_rootLayout->addWidget(group);
    m_engineEnabled->setChecked(true);
    m_autoProcess->setChecked(true);
}

void SettingsPage::setupAuditorGroup() {
    auto* group = new QGroupBox("監察設定");
    auto* layout = new QFormLayout(group);

    m_auditorEnabled = new QCheckBox("監察AI有効");
    m_warnStop = new QCheckBox("WARNで停止");
    m_autoReprocessReject = new QCheckBox("REJECT時に再処理");
    m_auditLogRetention = new QSpinBox();
    m_auditLogRetention->setRange(1, 365);
    m_auditLogRetention->setValue(30);

    layout->addRow("監察AI", m_auditorEnabled);
    layout->addRow("WARN停止", m_warnStop);
    layout->addRow("REJECT再処理", m_autoReprocessReject);
    layout->addRow("監査ログ保存日数", m_auditLogRetention);

    m_rootLayout->addWidget(group);
    m_auditorEnabled->setChecked(true);
    m_warnStop->setChecked(false);
    m_autoReprocessReject->setChecked(true);
}

void SettingsPage::setupUiGroup() {
    auto* group = new QGroupBox("UI設定");
    auto* layout = new QFormLayout(group);

    m_uiDark = new QCheckBox("ライト/ダーク(現在はダーク)");
    m_compact = new QCheckBox("コンパクト表示");
    m_refreshSec = new QSpinBox();
    m_refreshSec->setRange(1, 60);
    m_refreshSec->setValue(3);
    m_notify = new QCheckBox("通知 ON");

    layout->addRow("テーマ", m_uiDark);
    layout->addRow("コンパクト", m_compact);
    layout->addRow("自動更新間隔(秒)", m_refreshSec);
    layout->addRow("通知", m_notify);
    m_rootLayout->addWidget(group);
    m_uiDark->setChecked(true);
    m_compact->setChecked(false);
    m_notify->setChecked(true);
}

void SettingsPage::setupStorageGroup() {
    auto* group = new QGroupBox("保存先設定");
    auto* layout = new QFormLayout(group);
    m_ticketStorage = new QLineEdit("mock://tickets");
    m_logStorage = new QLineEdit("mock://audit_log");
    m_historyRetention = new QSpinBox();
    m_historyRetention->setRange(1, 365);
    m_historyRetention->setValue(90);
    m_exportButton = new QPushButton("エクスポート");

    layout->addRow("札保存先", m_ticketStorage);
    layout->addRow("ログ保存先", m_logStorage);
    layout->addRow("履歴保存日数", m_historyRetention);
    layout->addRow("監査履歴エクスポート", m_exportButton);
    connect(m_exportButton, &QPushButton::clicked, this, []() {
        QMessageBox::information(nullptr, "エクスポート", "監査履歴のエクスポートは未接続モードでは未実装です。");
    });
    m_rootLayout->addWidget(group);
}

void SettingsPage::setupSoundGroup() {
    auto* group = new QGroupBox("軍議演出SE");
    auto* layout = new QFormLayout(group);

    m_soundEnabled = new QCheckBox("有効/無効");
    m_soundMode = new QComboBox();
    m_soundMode->addItem("OFF", "off");
    m_soundMode->addItem("Minimal", "minimal");
    m_soundMode->addItem("Normal", "normal");
    m_soundMode->addItem("Gunji", "gunji");
    m_masterVol = new QDoubleSpinBox();
    m_eventVol = new QDoubleSpinBox();
    m_errorVol = new QDoubleSpinBox();
    m_masterVol->setRange(0.0, 1.0);
    m_eventVol->setRange(0.0, 1.0);
    m_errorVol->setRange(0.0, 1.0);
    m_cooldown = new QSpinBox();
    m_cooldown->setRange(0, 30000);
    m_cooldown->setValue(5000);
    m_nightMute = new QCheckBox("夜間ミュート");
    m_nightStart = new QLineEdit("22:00");
    m_nightEnd = new QLineEdit("08:00");

    m_soundEnabled->setChecked(false);
    m_soundMode->setCurrentIndex(0);
    m_masterVol->setValue(0.5);
    m_eventVol->setValue(0.4);
    m_errorVol->setValue(0.5);
    m_nightMute->setChecked(true);

    layout->addRow("有効 / 無効", m_soundEnabled);
    layout->addRow("モード", m_soundMode);
    layout->addRow("Master Volume", m_masterVol);
    layout->addRow("Event Volume", m_eventVol);
    layout->addRow("Error Volume", m_errorVol);
    layout->addRow("Cooldown", m_cooldown);
    layout->addRow("夜間ミュート", m_nightMute);
    layout->addRow("夜間ミュート開始", m_nightStart);
    layout->addRow("夜間ミュート終了", m_nightEnd);

    auto* label = new QLabel("現在は導線のみ。音源再生は未接続。");
    layout->addWidget(label);
    m_rootLayout->addWidget(group);
}

void SettingsPage::emitCurrentSettings() {
    emit settingsApplied(buildSettingsMap());
    QMessageBox::information(this, "設定反映", "設定を反映しました（SEはNo-op）。");
}

QVariantMap SettingsPage::buildSettingsMap() const {
    QVariantMap result;
    result["engine.enabled"] = m_engineEnabled->isChecked();
    result["engine.auto_process"] = m_autoProcess->isChecked();
    result["engine.audit_strength"] = m_auditStrength->value();
    result["engine.max_concurrency"] = m_maxConcurrent->value();
    result["engine.timeout_sec"] = m_timeoutSec->value();

    result["auditor.enabled"] = m_auditorEnabled->isChecked();
    result["auditor.warn_stop"] = m_warnStop->isChecked();
    result["auditor.auto_reprocess_reject"] = m_autoReprocessReject->isChecked();
    result["auditor.log_retention_days"] = m_auditLogRetention->value();

    result["ui.dark_mode"] = m_uiDark->isChecked();
    result["ui.compact"] = m_compact->isChecked();
    result["ui.refresh_seconds"] = m_refreshSec->value();
    result["ui.notify"] = m_notify->isChecked();

    result["storage.ticket_path"] = m_ticketStorage->text();
    result["storage.log_path"] = m_logStorage->text();
    result["storage.history_retention_days"] = m_historyRetention->value();

    SoundCueSettings sound = soundSettings();
    result["sound.enabled"] = sound.enabled;
    result["sound.mode"] = toString(sound.mode);
    result["sound.master_volume"] = sound.masterVolume;
    result["sound.event_volume"] = sound.eventVolume;
    result["sound.error_volume"] = sound.errorVolume;
    result["sound.cooldown_ms"] = sound.cooldownMs;
    result["sound.night_mute_enabled"] = sound.nightMuteEnabled;
    result["sound.night_mute_start"] = sound.nightMuteStart;
    result["sound.night_mute_end"] = sound.nightMuteEnd;
    return result;
}

SoundCueSettings SettingsPage::soundSettings() const {
    SoundCueSettings settings;
    settings.enabled = m_soundEnabled->isChecked();
    const QString mode = m_soundMode->currentData().toString();
    if (mode == "minimal") settings.mode = SoundCueMode::Minimal;
    else if (mode == "normal") settings.mode = SoundCueMode::Normal;
    else if (mode == "gunji") settings.mode = SoundCueMode::Gunji;
    else settings.mode = SoundCueMode::Off;
    settings.masterVolume = m_masterVol->value();
    settings.eventVolume = m_eventVol->value();
    settings.errorVolume = m_errorVol->value();
    settings.cooldownMs = m_cooldown->value();
    settings.nightMuteEnabled = m_nightMute->isChecked();
    settings.nightMuteStart = m_nightStart->text();
    settings.nightMuteEnd = m_nightEnd->text();
    return settings;
}

void SettingsPage::setSoundSettings(const SoundCueSettings& settings) {
    m_soundEnabled->setChecked(settings.enabled);
    if (settings.mode == SoundCueMode::Minimal) {
        m_soundMode->setCurrentText("Minimal");
    } else if (settings.mode == SoundCueMode::Normal) {
        m_soundMode->setCurrentText("Normal");
    } else if (settings.mode == SoundCueMode::Gunji) {
        m_soundMode->setCurrentText("Gunji");
    } else {
        m_soundMode->setCurrentText("OFF");
    }
    m_masterVol->setValue(settings.masterVolume);
    m_eventVol->setValue(settings.eventVolume);
    m_errorVol->setValue(settings.errorVolume);
    m_cooldown->setValue(settings.cooldownMs);
    m_nightMute->setChecked(settings.nightMuteEnabled);
    m_nightStart->setText(settings.nightMuteStart);
    m_nightEnd->setText(settings.nightMuteEnd);
}

int SettingsPage::autoRefreshSeconds() const {
    return m_refreshSec->value();
}

} // namespace nexspace
