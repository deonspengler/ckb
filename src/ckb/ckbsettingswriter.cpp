#include "ckbsettingswriter.h"
#include <QMutex>
#include <cstdio>

// Mirror ckbsettings.cpp
extern QAtomicInt cacheWritesInProgress;
extern QMutex settingsMutex, settingsCacheMutex;
#define lockMutex           QMutexLocker locker(backing == _globalSettings ? &mutex : 0)
#define lockMutexStatic     QMutexLocker locker(&settingsMutex)
#define lockMutexStatic2    QMutexLocker locker2(&settingsMutex)
#define lockMutexCache      QMutexLocker locker(&settingsCacheMutex)

CkbSettingsWriter::CkbSettingsWriter(QSettings* backing, const QStringList& removals, const QMap<QString, QVariant>& updates) :
    _backing(backing), _removals(removals), _updates(updates) {
    cacheWritesInProgress.ref();
}

CkbSettingsWriter::~CkbSettingsWriter(){
    cacheWritesInProgress.deref();
}

void CkbSettingsWriter::run(){
    lockMutexStatic;
    // Process key removals
    foreach(const QString& rm, _removals){
        _backing->remove(rm);
        QByteArray keyC = rm.toUtf8();
        printf("Settings: deleting %s\n", keyC.constData());
    }
    // Process writes
    QMapIterator<QString, QVariant> i(_updates);
    while(i.hasNext()){
        i.next();
        _backing->setValue(i.key(), i.value());
        QByteArray keyC = i.key().toUtf8();
        QByteArray valC = i.value().toString().toUtf8();
        printf("Settings: writing %s <- %s\n", keyC.constData(), valC.constData());
        // Updating the global cache was done above
    }
    deleteLater();
}
