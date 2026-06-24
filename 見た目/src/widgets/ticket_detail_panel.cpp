#include "ticket_detail_panel.h"
#include "../domain.h"
#include <QVBoxLayout>
#include <QLabel>

namespace nexspace {

namespace {
QString auditCategoryLabel(const QString& category) {
    if (category == "requirement_missing") return "要件未達";
    if (category == "scope_deviation") return "仕様逸脱";
    if (category == "unsafe_action") return "危険操作";
    if (category == "tool_mismatch") return "ツール不整合";
    if (category == "format_error") return "形式不備";
    if (category == "incomplete_output") return "未完成出力";
    if (category == "unauthorized_change") return "未承認変更";
    if (category == "system_error") return "系統異常";
    return category.isEmpty() ? "-" : category;
}
}

TicketDetailPanel::TicketDetailPanel(QWidget* parent)
    : QWidget(parent) {
    setObjectName("DetailPanel");
    auto* layout = new QVBoxLayout(this);
    m_id = new QLabel("令札ID: -");
    m_title = new QLabel("タイトル: -");
    m_summary = new QLabel("要約: -");
    m_type = new QLabel("種別: -");
    m_priority = new QLabel("優先度: -");
    m_status = new QLabel("状態: -");
    m_agent = new QLabel("担当AI: -");
    m_auditResult = new QLabel("監察結果: -");
    m_auditCategory = new QLabel("指摘カテゴリ: -");
    m_recommend = new QLabel("推奨対応: -");
    m_created = new QLabel("作成: -");
    m_updated = new QLabel("更新: -");
    m_internal = new QLabel("内部ログ要約: -");
    m_internal->setWordWrap(true);

    layout->addWidget(new QLabel("令札詳細"));
    layout->addWidget(m_id);
    layout->addWidget(m_title);
    layout->addWidget(m_summary);
    layout->addWidget(m_type);
    layout->addWidget(m_priority);
    layout->addWidget(m_status);
    layout->addWidget(m_agent);
    layout->addWidget(m_auditResult);
    layout->addWidget(m_auditCategory);
    layout->addWidget(m_recommend);
    layout->addWidget(m_created);
    layout->addWidget(m_updated);
    layout->addWidget(new QLabel("内部ログ要約"));
    layout->addWidget(m_internal, 1);
}

void TicketDetailPanel::setTicket(const std::optional<Ticket>& ticketOpt, const QVector<AuditLog>& logs) {
    if (!ticketOpt.has_value()) {
        clear();
        return;
    }

    const Ticket& ticket = *ticketOpt;
    m_id->setText(QString("令札ID: %1").arg(ticket.id));
    m_title->setText(QString("タイトル: %1").arg(ticket.title));
    m_summary->setText(QString("要約: %1").arg(ticket.summary));
    m_type->setText(QString("種別: %1").arg(ticket.type));
    m_priority->setText(QString("優先度: %1").arg(toLabel(ticket.priority)));
    m_status->setText(QString("状態: %1").arg(toJapaneseStatus(ticket.status)));
    m_agent->setText(QString("担当AI: %1").arg(ticket.assignedAgentId.isEmpty() ? "未割当" : ticket.assignedAgentId));

    if (ticket.auditResult.has_value()) {
        m_auditResult->setText(QString("監察結果: %1").arg(toJapaneseAuditResult(ticket.auditResult.value())));
        m_auditCategory->setText(QString("指摘カテゴリ: %1").arg(auditCategoryLabel(ticket.auditCategory)));
    } else {
        m_auditResult->setText("監察結果: 未判定");
        m_auditCategory->setText("指摘カテゴリ: -");
    }
    m_recommend->setText("推奨対応: -");

    for (const auto& log : logs) {
        if (log.ticketId == ticket.id && !log.recommendedAction.isEmpty()) {
            m_recommend->setText(QString("推奨対応: %1").arg(log.recommendedAction));
            break;
        }
    }
    m_created->setText(QString("作成時刻: %1").arg(ticket.createdAt));
    m_updated->setText(QString("更新時刻: %1").arg(ticket.updatedAt));

    QStringList logSummary;
    for (const auto& log : logs) {
        if (log.ticketId == ticket.id) {
            logSummary.append(QString("[%1] %2").arg(log.createdAt, log.reasonSummary));
        }
    }
    m_internal->setText(logSummary.isEmpty() ? "内部ログ要約: なし" : logSummary.join(" / "));
}

void TicketDetailPanel::clear() {
    m_id->setText("令札ID: -");
    m_title->setText("タイトル: -");
    m_summary->setText("要約: -");
    m_type->setText("種別: -");
    m_priority->setText("優先度: -");
    m_status->setText("状態: -");
    m_agent->setText("担当AI: -");
    m_auditResult->setText("監察結果: -");
    m_auditCategory->setText("指摘カテゴリ: -");
    m_recommend->setText("推奨対応: -");
    m_created->setText("作成: -");
    m_updated->setText("更新: -");
    m_internal->setText("内部ログ要約: -");
}

} // namespace nexspace
