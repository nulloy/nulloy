/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2013 Sergey Vlasov <sergey@vlasov.me>
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

#include "global.h"
#include "common.h"
#include "settings.h"

#include "pluginContainer.h"
#include "plugin.h"

#include "waveformBuilderInterface.h"
#include "playbackEngineInterface.h"
#include "tagReaderInterface.h"
#include "coverReaderInterface.h"

#include <QMessageBox>
#include <QObject>
#include <QPluginLoader>
#include <QStringList>

#ifdef _N_GSTREAMER_PLUGINS_BUILTIN_
#include "playbackEngineGstreamer.h"
#include "waveformBuilderGstreamer.h"
#endif

namespace NPluginLoader
{
	static bool _init = FALSE;
	static QStringList _identifiers;
	static NPlaybackEngineInterface *_playback = NULL;
	static NWaveformBuilderInterface *_waveform = NULL;
	static NTagReaderInterface *_tagReader = NULL;
	static NCoverReaderInterface *_coverReader = NULL;

	void _loadPlugins();
	QObject* _findPlugin(N::PluginType type, QObjectList &objects, QMap<QString, bool> &usedFlags);
	static QMap<QString, QPluginLoader *> _loaders;
}

void NPluginLoader::deinit()
{
	foreach (QString key, _loaders.keys()) {
		if (_loaders[key])
			_loaders[key]->unload();
		_loaders.remove(key);
	}
}

QObject* NPluginLoader::_findPlugin(N::PluginType type, QObjectList &objects, QMap<QString, bool> &usedFlags)
{
	QString typeString = ENUM_NAME(N, PluginType, type);
	QString savedContainerName = NSettings::instance()->value("Plugins/" + typeString).toString();

	int index;
	index = _identifiers.indexOf(QRegExp(typeString + "/" + savedContainerName + "/.*"));
	if (index == -1)
		index = _identifiers.indexOf(QRegExp(typeString + "/GStreamer/.*"));
	if (index == -1)
		index = _identifiers.indexOf(QRegExp(typeString + "/.*"));
	if (index != -1) {
		NPlugin *plugin = qobject_cast<NPlugin *>(objects.at(index));

		QString identifier = _identifiers.at(index);

		usedFlags[identifier] = TRUE;

		QString containerName = identifier.section('/', 1, 1);
		NSettings::instance()->setValue(QString() + "Plugins/" + typeString, containerName);

		plugin->init();

		return objects.at(index);
	} else {
		return NULL;
	}
}

void NPluginLoader::_loadPlugins()
{
	if (_init)
		return;
	_init = TRUE;

	QObjectList objects;
	QMap<QString, bool> usedFlags;

#if 0
	QObjectList objectsStatic;
#ifdef _N_GSTREAMER_PLUGINS_BUILTIN_
	objectsStatic << new NPlaybackEngineGStreamer() << new NWaveformBuilderGstreamer();
#endif
	objectsStatic << QPluginLoader::staticInstances();

	foreach (QObject *obj, objectsStatic) {
		NPlugin *plugin = qobject_cast<NPlugin *>(obj);
		if (plugin) {
			objects << obj;
			qobject_cast<NPlugin *>(obj)->init();
			QString id = plugin->identifier();
			id.insert(id.lastIndexOf('/'), " (Built-in)");
			_identifiers << id;
			_loaders << NULL;
			usedFlags << TRUE;
		}
	}
#endif

	QStringList pluginsDirList;
	pluginsDirList << QCoreApplication::applicationDirPath() + "/plugins";
#ifndef Q_WS_WIN
	if (NCore::rcDir() != QCoreApplication::applicationDirPath())
		pluginsDirList << NCore::rcDir() + "/plugins";
	if (QDir(QCoreApplication::applicationDirPath()).dirName() == "bin") {
		QDir dir(QCoreApplication::applicationDirPath());
		dir.cd("../lib/nulloy/plugins");
		pluginsDirList << dir.absolutePath();
	}
#endif

#ifdef Q_WS_WIN
		QStringList subDirsList;
		foreach (QString dirStr, pluginsDirList) {
			QDir dir(dirStr);
			if (dir.exists()) {
				foreach (QString subDir, dir.entryList(QDir::Dirs))
					subDirsList << dirStr + "/" + subDir;
			}
		}
		_putenv(QString("PATH=" + pluginsDirList.join(";") + ";" +
			subDirsList.join(";") + ";" + getenv("PATH")).replace('/', '\\').toUtf8());
#endif
	foreach (QString dirStr, pluginsDirList) {
		QDir dir(dirStr);
		if (!dir.exists())
			continue;
		foreach (QString fileName, dir.entryList(QDir::Files)) {
			QString fileFullPath = dir.absoluteFilePath(fileName);
#ifdef Q_WS_WIN
			// skip non plugin files
			if (!fileName.startsWith("plugin", Qt::CaseInsensitive) || !fileName.endsWith("dll", Qt::CaseInsensitive))
				continue;
#endif
			if (!QLibrary::isLibrary(fileFullPath))
				continue;
			QPluginLoader *loader = new QPluginLoader(fileFullPath);
			QObject *instance = loader->instance();
			NPluginContainer *container = qobject_cast<NPluginContainer *>(instance);
			if (container) {
				QObjectList plugins = container->plugins();
				objects << plugins;
				foreach (QObject *obj, plugins) {
					NPlugin *plugin = qobject_cast<NPlugin *>(obj);
					QString typeString = ENUM_NAME(N, PluginType, plugin->type());
					QString identifier = typeString + "/" + container->name() + "/" + fileFullPath.replace("/", "\\");
					_identifiers << identifier;
					_loaders[identifier] = loader;
					usedFlags[identifier] = FALSE;
				}
			} else {
				QMessageBox box(QMessageBox::Warning, QObject::tr("Plugin loading error"), QObject::tr("Failed to load plugin: ") +
				                fileFullPath + "\n\n" + loader->errorString(), QMessageBox::Close);
				box.exec();
				delete loader;
			}
		}
	}

	_playback = qobject_cast<NPlaybackEngineInterface *>(_findPlugin(N::PlaybackEngineType, objects, usedFlags));
	_waveform = qobject_cast<NWaveformBuilderInterface *>(_findPlugin(N::WaveformBuilderType, objects, usedFlags));
	_tagReader = qobject_cast<NTagReaderInterface *>(_findPlugin(N::TagReaderType, objects, usedFlags));
	_coverReader = qobject_cast<NCoverReaderInterface *>(_findPlugin(N::CoverReaderType, objects, usedFlags));

	// remove not used plugins
	foreach (QString identifier, usedFlags.keys(FALSE)) {
		QStringList boundIdentifiers = _loaders.keys(_loaders[identifier]);
		bool safeToUnload = TRUE;
		foreach (QString boundIdentifier, boundIdentifiers) {
			if (usedFlags[boundIdentifier] == TRUE) {
				safeToUnload = FALSE;
				break;
			}
		}
		if (safeToUnload) {
			_loaders[identifier]->unload();
			_loaders.remove(identifier);
		}
	}

	if (!_waveform || !_playback || !_tagReader) {
		QStringList message;
		if (!_waveform)
			message << QObject::tr("No Waveform plugin found.");
		if (!_playback)
			message << QObject::tr("No Playback plugin found.");
		if (!_tagReader)
			message << QObject::tr("No TagReader plugin found.");
		QMessageBox::critical(NULL, QObject::tr("Plugin loading error"), message.join("\n"), QMessageBox::Close);
		exit(1);
	}
}

NPlaybackEngineInterface* NPluginLoader::playbackPlugin()
{
	_loadPlugins();
	return _playback;
}

NWaveformBuilderInterface* NPluginLoader::waveformPlugin()
{
	_loadPlugins();
	return _waveform;
}

NTagReaderInterface* NPluginLoader::tagReaderPlugin()
{
	_loadPlugins();
	return _tagReader;
}

NCoverReaderInterface* NPluginLoader::coverReaderPlugin()
{
	_loadPlugins();
	return _coverReader;
}

QStringList NPluginLoader::pluginIdentifiers()
{
	_loadPlugins();
	return _identifiers;
}

