#pragma once

#include "domain.h"
#include "department_router.h"
#include <QHash>
#include <QVector>

namespace nexspace {

class DepartmentRegistry {
public:
    static const QVector<Department>& allDepartments();
    static QStringList allDepartmentIds();
    static const Department* findDepartment(const QString& departmentId);
    static QStringList routeForTicket(const Ticket& ticket);
    static QString initialDepartmentForTicket(const Ticket& ticket);
    static bool canAuditDepartment(const QString& targetDepartmentId);
    static bool isValidTransition(const QString& fromDepartmentId, const QString& toDepartmentId, const QStringList& route);
    static bool hasParent(const QString& departmentId);

private:
    static QVector<Department> buildDepartments();
};

} // namespace nexspace
