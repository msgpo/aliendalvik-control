#include "binderinterfaceabstract.h"
#include "intent.h"

#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(parcelCategory, "binder.parcel", QtDebugMsg)

void Intent::writeToParcel(Parcel *parcel)
{
    if (!parcel) {
        qCCritical(parcelCategory) << Q_FUNC_INFO << "Null Parcel!";
        return;
    }

    parcel->writeString(action);

    // Uri.writeToParcel
    if (data.isNull()) {
        parcel->writeInt(0); // NULL_TYPE_ID
    } else {
        parcel->writeInt(1); // TYPE_ID UriString = 1
        parcel->writeString(data);
    }

    parcel->writeString(type);

    parcel->writeInt(flags);

    parcel->writeString(package);

    // ComponentName.writeToParcel
    if (classPackage.isEmpty() && className.isEmpty()) {
        parcel->writeString(QString());
    } else {
        parcel->writeString(classPackage);
        parcel->writeString(className);
    }

    parcel->writeInt(0); // mSourceBounds disable
    parcel->writeInt(0); // mCategories disable
    parcel->writeInt(0); // mSelector disable
    parcel->writeInt(0); // mClipData disable

    parcel->writeInt(contentUserHint);

    if (extras.isEmpty()) {
        parcel->writeInt(-1);
    } else {
        parcel->writeBundle(extras);
    }
}
