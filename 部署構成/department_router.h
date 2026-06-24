#pragma once

#include <QString>
#include <QStringList>
#include "domain.h"

namespace nexspace {

class DepartmentRouter {
public:
    static QStringList routeForTicket(const Ticket& ticket) {
        if (isTechnicalTicket(ticket)) {
            return routeBaseForBranch("07_技術部");
        }
        if (isSalesTicket(ticket)) {
            return routeBaseForBranch("06_営業部");
        }
        return routeBaseForBranch("05_制作室");
    }

    static QString initialDepartmentForTicket(const Ticket& ticket) {
        const QStringList route = routeForTicket(ticket);
        return route.isEmpty() ? QString("02_秘書室") : route.first();
    }

    static bool isValidTransition(const QString& fromDepartmentId, const QString& toDepartmentId, const QStringList& route) {
        if (toDepartmentId.isEmpty()) {
            return false;
        }
        if (fromDepartmentId.isEmpty()) {
            return toDepartmentId == "02_秘書室";
        }
        if (route.isEmpty()) {
            return false;
        }

        const int fromIndex = route.indexOf(fromDepartmentId);
        const int toIndex = route.indexOf(toDepartmentId);
        if (fromIndex < 0) {
            return false;
        }

        if (toIndex == fromIndex - 1) {
            return true;
        }
        if (toIndex < 0) {
            return false;
        }

        return toIndex > fromIndex && (toIndex - fromIndex) == 1;
    }

private:
    static bool isTechnicalTicket(const Ticket& ticket) {
        return ticket.type.contains("技術") || ticket.title.contains("技術") ||
               ticket.type.contains("実装") || ticket.summary.contains("実装") ||
               ticket.summary.contains("API") || ticket.title.contains("API");
    }

    static bool isSalesTicket(const Ticket& ticket) {
        return ticket.type.contains("営業") || ticket.type.contains("販売") || ticket.type.contains("告知") ||
               ticket.title.contains("営業") || ticket.summary.contains("顧客") ||
               ticket.title.contains("顧客") || ticket.summary.contains("提案");
    }

    static QStringList routeBaseForBranch(const QString& branchDepartmentId) {
        if (branchDepartmentId == "07_技術部") {
            return {"02_秘書室", "01_社長室", "04_戦略室", "05_制作室", "07_技術部", "08_検査部", "03_監査室", "10_庶務課"};
        }
        if (branchDepartmentId == "06_営業部") {
            return {"02_秘書室", "01_社長室", "04_戦略室", "06_営業部", "08_検査部", "03_監査室", "10_庶務課"};
        }
        return {"02_秘書室", "01_社長室", "04_戦略室", "05_制作室", "08_検査部", "03_監査室", "10_庶務課"};
    }
};

} // namespace nexpace
