#include "binderinterfaceabstract.h"
#include "intent.h"
#include "parcel.h"
#include "parcelable.h"

#include <QEventLoop>
#include <QTimer>

void BinderInterfaceAbstract::registrationHandler(GBinderServiceManager *sm, const char *name, void *user_data)
{
    BinderInterfaceAbstract* instance = static_cast<BinderInterfaceAbstract *>(user_data);
    qCDebug(instance->logging) << Q_FUNC_INFO << "Service registered" << sm << name << user_data;
    instance->registerManager();
}

void BinderInterfaceAbstract::deathHandler(GBinderRemoteObject *obj, void *user_data)
{
    BinderInterfaceAbstract* instance = static_cast<BinderInterfaceAbstract *>(user_data);
    qCWarning(instance->logging) << Q_FUNC_INFO << "Binder died" << obj << user_data;
    instance->binderDisconnect();
}

BinderInterfaceAbstract::BinderInterfaceAbstract(const char *serviceName,
                                                 const char *interfaceName,
                                                 QObject *parent,
                                                 const char *loggingCategoryName)
    : AliendalvikController(parent)
    , LoggingClassWrapper(loggingCategoryName)
    , m_serviceName(serviceName)
    , m_interfaceName(interfaceName)
{
}

BinderInterfaceAbstract::~BinderInterfaceAbstract()
{
    binderDisconnect();
}

bool BinderInterfaceAbstract::isBinderReady() const
{
    return m_client;
}

bool BinderInterfaceAbstract::wait(quint64 timeout)
{
    if (isBinderReady()) {
        return true;
    }
    QTimer timer;
    QEventLoop loop;
    connect(this, &BinderInterfaceAbstract::binderConnected, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeout);
    loop.exec();

    return isBinderReady();
}

QSharedPointer<Parcel> BinderInterfaceAbstract::createTransaction()
{
    QSharedPointer<Parcel> dataParcel;
    qCDebug(logging) << Q_FUNC_INFO;

    if (!wait()) {
        qCCritical(logging) << Q_FUNC_INFO << "No client!";
        return dataParcel;
    }

    GBinderLocalRequest* req = gbinder_client_new_request(m_client);
    dataParcel.reset(new Parcel(req));
    return dataParcel;
}

QSharedPointer<Parcel> BinderInterfaceAbstract::sendTransaction(int code, QSharedPointer<Parcel> parcel, int *status)
{
    qCDebug(logging) << Q_FUNC_INFO << code;
    QSharedPointer<Parcel> outParcel;

    if (!m_client) {
        qCCritical(logging) << Q_FUNC_INFO << "No client!";
        return outParcel;
    }

    GBinderRemoteReply *reply = gbinder_client_transact_sync_reply
            (m_client, code, parcel->request(), status);

    outParcel.reset(new Parcel(reply));
    return outParcel;
}

GBinderServiceManager *BinderInterfaceAbstract::manager()
{
    return m_serviceManager;
}

GBinderLocalReply *BinderInterfaceAbstract::onTransact(GBinderLocalObject *obj, GBinderRemoteRequest *req, guint code, guint flags, int *status, void *user_data)
{
    BinderInterfaceAbstract* instance = static_cast<BinderInterfaceAbstract *>(user_data);
    const char *iface = gbinder_remote_request_interface(req);
    qCWarning(instance->logging) << Q_FUNC_INFO;
    qCWarning(instance->logging) << "Interface:" << iface;
    qCWarning(instance->logging) << "GBinderLocalObject:" << obj;
    qCWarning(instance->logging) << "GBinderRemoteRequest:" << req;
    qCWarning(instance->logging) << "Code:" << code;
    qCWarning(instance->logging) << "Flags:" << flags;
    GBinderLocalReply *reply = NULL;
    *status = GBINDER_STATUS_OK;
    return reply;
}

void BinderInterfaceAbstract::serviceStopped()
{
    qCDebug(logging) << Q_FUNC_INFO;
    binderDisconnect();
}

void BinderInterfaceAbstract::serviceStarted()
{
    qCDebug(logging) << Q_FUNC_INFO;
    binderConnect();
}

void BinderInterfaceAbstract::binderConnect()
{
    qCDebug(logging) << Q_FUNC_INFO << "Binder connect" << m_serviceName << m_interfaceName;

    if (!m_serviceManager) {
        qCDebug(logging) << Q_FUNC_INFO << "Creating service manager for" << binderDevice();
        m_serviceManager = gbinder_servicemanager_new(binderDevice());
    }

    if (!m_serviceManager) {
        qCCritical(logging) << Q_FUNC_INFO << "Can't create service manager!";
        return;
    }


    qCDebug(logging) << Q_FUNC_INFO << "Add registration handler";
    m_registrationHandler = gbinder_servicemanager_add_registration_handler(m_serviceManager,
                                                                            m_serviceName,
                                                                            &BinderInterfaceAbstract::registrationHandler,
                                                                            this);
}

void BinderInterfaceAbstract::binderDisconnect()
{
    qCWarning(logging) << Q_FUNC_INFO << "Binder disconnect" << m_serviceName << m_interfaceName;

    if (m_client) {
        qCWarning(logging) << Q_FUNC_INFO << "Removing client";
        gbinder_client_unref(m_client);
        m_client = nullptr;
    }

    if (m_deathHandler && m_remote) {
        qCWarning(logging) << Q_FUNC_INFO << "Removing death handler";
        gbinder_remote_object_remove_handler(m_remote, m_deathHandler);
        m_deathHandler = 0;
    }

    if (m_remote) {
        qCWarning(logging) << Q_FUNC_INFO << "Removing remote";
        gbinder_remote_object_unref(m_remote);
        m_remote = nullptr;
    }

    if (m_registrationHandler && m_serviceManager) {
        qCWarning(logging) << Q_FUNC_INFO << "Removing registration handler";
        gbinder_servicemanager_remove_handler(m_serviceManager, m_registrationHandler);
        m_registrationHandler = 0;
    }

    if (m_serviceManager) {
        qCWarning(logging) << Q_FUNC_INFO << "Removing service manager";
        gbinder_servicemanager_unref(m_serviceManager);
        m_serviceManager = nullptr;
    }

    emit binderDisconnected();
}

void BinderInterfaceAbstract::registerManager()
{
    qCDebug(logging) << Q_FUNC_INFO << "Register manager" << m_serviceName;

    int status;
    m_remote = gbinder_remote_object_ref(
                gbinder_servicemanager_get_service_sync(m_serviceManager,
                                                        m_serviceName,
                                                        &status));

    qCDebug(logging) << Q_FUNC_INFO << "Service status:" << status;

    if (!m_remote) {
        qCCritical(logging) << Q_FUNC_INFO << "No remote!";
        binderDisconnect();
        return;
    }

    m_deathHandler = gbinder_remote_object_add_death_handler(m_remote,
                                                             &BinderInterfaceAbstract::deathHandler,
                                                             this);

    qCDebug(logging) << Q_FUNC_INFO << "Register client" << m_interfaceName;

    m_client = gbinder_client_new(m_remote, m_interfaceName);

    if (!m_client) {
        qCCritical(logging) << Q_FUNC_INFO << "No client!";
        binderDisconnect();
        return;
    }

    qCDebug(logging) << Q_FUNC_INFO << "Removing registration handler";

    gbinder_servicemanager_remove_handler(m_serviceManager, m_registrationHandler);
    m_registrationHandler = 0;

    emit binderConnected();
}
