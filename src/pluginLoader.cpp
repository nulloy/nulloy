/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2022 Sergey Vlasov <sergey@vlasov.me>
**
**  This program can be distributed under the terms of the GNU
**  General Public License version 3.0 as published by the Free
**  Software Foundation and appearing in the file LICENSE.GPL3
**  included in the packaging of this file.  Please review the
**  following information to ensure the GNU General Public License
**  version 3.0 requirements will be met:
**
**  http://www.gnu.org/licenses/gpl-3.0.html
**
*********************************************************************/

#include "pluginLoader.h"

#include <QMessageBox>
#include <QPluginLoader>

#include "common.h"
#include "coverReaderInterface.h"
#include "playbackEngineInterface.h"
#include "pluginContainer.h"
#include "settings.h"
#include "tagReaderInterface.h"
#include "waveformBuilderInterface.h"

Q_DECLARE_METATYPE(NPlugin *)
Q_DECLARE_METATYPE(QPluginLoader *)

static const char _containerPrefer[] = "GStreamer";
static const char _pluginsDirName[] = "plugins";

namespace NPluginLoader
{
    bool _init = false;
    QList<Descriptor> _descriptors;
    QMap<N::PluginType, NPlugin *> _usedPlugins;
    QMap<QPluginLoader *, bool> _usedLoaders;

    NPlugin *_findPlugin(N::PluginType type);
} // namespace NPluginLoader

void NPluginLoader::deinit()
{
    if (!_init) {
        return;
    }
    _init = false;

    foreach (QPluginLoader *loader, _usedLoaders.keys()) {
        if (loader->isLoaded()) {
            loader->unload();
        }
    }
    _descriptors.clear();
    _usedPlugins.clear();
    _usedLoaders.clear();
}

NPlugin *NPluginLoader::_findPlugin(N::PluginType type)
{
    QString typeString = ENUM_TO_STR(N, PluginType, type);
    QString settingsContainer = NSettings::instance()->value("Plugins/" + typeString).toString();

    QList<int> indexesFilteredByType;
    for (int i = 0; i < _descriptors.count(); ++i) {
        if (_descriptors.at(i)[TypeRole] == type) {
            indexesFilteredByType << i;
        }
    }

    if (indexesFilteredByType.isEmpty()) {
        return NULL;
    }

    int index = -1;
    foreach (QString container, QStringList() << settingsContainer << _containerPrefer) {
        foreach (int i, indexesFilteredByType) {
            if (_descriptors.at(i)[ContainerNameRole] == container) {
                index = i;
                break;
            }
        }
        if (index != -1) {
            break;
        }
    }

    if (index == -1) {
        index = indexesFilteredByType.first();
    }

    NPlugin *plugin = _descriptors.at(index)[PluginObjectRole].value<NPlugin *>();
    plugin->init();

    QPluginLoader *loader = _descriptors.at(index)[LoaderObjectRole].value<QPluginLoader *>();
    _usedLoaders[loader] = true;

    QString containerName = _descriptors.at(index)[ContainerNameRole].toString();
    NSettings::instance()->setValue(QString() + "Plugins/" + typeString, containerName);

    return plugin;
}

void NPluginLoader::init()
{
    if (_init) {
        return;
    }
    _init = true;

    QStringList pluginsDirList;
    pluginsDirList << QCoreApplication::applicationDirPath() + "/" + _pluginsDirName;
#ifndef Q_OS_WIN
    if (NCore::rcDir() != QCoreApplication::applicationDirPath()) {
        pluginsDirList << NCore::rcDir() + "/" + _pluginsDirName;
    }
    if (QDir(QCoreApplication::applicationDirPath()).dirName() == "bin") {
        QDir dir(QCoreApplication::applicationDirPath());
        dir.cd(QString() + "../" + N_LIBDIR + "/nulloy/" + _pluginsDirName);
        pluginsDirList << dir.absolutePath();
    }
#endif

#ifdef Q_OS_WIN
    QStringList subDirsList;
    foreach (QString dirStr, pluginsDirList) {
        QDir dir(dirStr);
        if (dir.exists()) {
            foreach (QString subDir, dir.entryList(QDir::Dirs))
                subDirsList << dirStr + "/" + subDir;
        }
    }
    _wputenv(reinterpret_cast<const wchar_t *>(QString("PATH=" + pluginsDirList.join(";") + ";" +
                                                       subDirsList.join(";") + ";" + getenv("PATH"))
                                                   .replace('/', '\\')
                                                   .utf16()));
#endif
    foreach (QString dirStr, pluginsDirList) {
        QDir dir(dirStr);
        if (!dir.exists()) {
            continue;
        }
        foreach (QString fileName, dir.entryList(QDir::Files)) {
            QString fileFullPath = dir.absoluteFilePath(fileName);
#ifdef Q_OS_WIN
            // skip non plugin files
            if (!fileName.startsWith("plugin", Qt::CaseInsensitive) ||
                !fileName.endsWith("dll", Qt::CaseInsensitive))
                continue;
#endif
            if (!QLibrary::isLibrary(fileFullPath)) {
                continue;
            }
            QPluginLoader *loader = new QPluginLoader(fileFullPath);
            _usedLoaders[loader] = false;
            QObject *instance = loader->instance();
            NPluginContainer *container = qobject_cast<NPluginContainer *>(instance);
            if (container) {
                qDebug() << "found container" << container->name() << ":";
                QList<NPlugin *> _plugins = container->plugins();
                foreach (NPlugin *plugin, _plugins) {
                    qDebug() << "*" << ENUM_TO_STR(N, PluginType, plugin->type());
                    Descriptor d;
                    d[TypeRole] = plugin->type();
                    d[ContainerNameRole] = container->name();
                    d[PluginObjectRole] = QVariant::fromValue<NPlugin *>(plugin);
                    d[LoaderObjectRole] = QVariant::fromValue<QPluginLoader *>(loader);
                    _descriptors << d;
                }
            } else {
                QMessageBox::warning(NULL, QObject::tr("Plugin loading error"),
                                     QObject::tr("Failed to load plugin: ") + fileFullPath +
                                         "\n\n" + loader->errorString(),
                                     QMessageBox::Close);
                delete loader;
            }
        }
    }

    NFlagIterator<N::PluginType> iter(N::MaxPlugin);
    while (iter.hasNext()) {
        iter.next();
        N::PluginType type = iter.value();
        _usedPlugins[type] = _findPlugin(type);
        if (_usedPlugins[type]) {
            qDebug() << "registering plugin:" << _usedPlugins[type]->name();
        }
    }

    // unload non-used
    foreach (QPluginLoader *loader, _usedLoaders.keys(false)) {
        qDebug() << "unloading non-used container:"
                 << (qobject_cast<NPluginContainer *>(loader->instance()))->name();
        loader->unload();
    }

    if (!_usedPlugins[N::WaveformBuilder] || !_usedPlugins[N::PlaybackEngine] ||
        !_usedPlugins[N::TagReader]) {
        QStringList message;
        if (!_usedPlugins[N::WaveformBuilder]) {
            message << QObject::tr("No Waveform plugin found.");
        }
        if (!_usedPlugins[N::PlaybackEngine]) {
            message << QObject::tr("No Playback plugin found.");
        }
        if (!_usedPlugins[N::TagReader]) {
            message << QObject::tr("No TagReader plugin found.");
        }
        QMessageBox::critical(NULL, QObject::tr("Plugin loading error"), message.join("\n"),
                              QMessageBox::Close);
        exit(1);
    }
}

NPlugin *NPluginLoader::getPlugin(N::PluginType type)
{
    if (!_init) {
        return NULL;
    }
    return _usedPlugins[type];
}

QList<NPluginLoader::Descriptor> NPluginLoader::descriptors()
{
    return _descriptors;
}
