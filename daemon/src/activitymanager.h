#ifndef ACTIVITYMANAGER_H
#define ACTIVITYMANAGER_H

#include "binderinterfaceabstract.h"

#include <QVariantHash>

enum {
    START_ACTIVITY_TRANSACTION = GBINDER_FIRST_CALL_TRANSACTION + 2,
    FORCE_STOP_PACKAGE_TRANSACTION = GBINDER_FIRST_CALL_TRANSACTION + 70,
};

enum {
    USER_NULL = -10000,
    USER_CURRENT_OR_SELF = -3,
    USER_CURRENT = -2,
    USER_ALL = -1,
    USER_OWNER = 0
};

class Intent
{
public:
    void writeToParcel(Parcel *parcel);

    QString action;
    QString data;
    QString type;

    int flags = 0;

    QString package;
    QString classPackage;
    QString className;

    int contentUserHint = USER_CURRENT;

    QVariantHash extras;
};

class ActivityManager : public BinderInterfaceAbstract
{
    Q_OBJECT
public:
    explicit ActivityManager(QObject *parent = nullptr);

    void startActivity(Intent intent);
    void forceStopPackage(const QString &package);

};

#endif // ACTIVITYMANAGER_H
