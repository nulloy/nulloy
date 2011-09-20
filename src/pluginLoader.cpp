/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2011 Sergey Vlasov <sergey@vlasov.me>
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

#include "core.h"
#include "settings.h"
#include "pluginInterface.h"
#include "waveformBuilderInterface.h"
#include "playbackEngineInterface.h"

#include <QObject>
#include <QMessageBox>
#include <QPluginLoader>

#ifdef _N_GSTREAMER_PLUGINS_BUILTIN_
#include "playbackEngineGstreamer.h"
#include "waveformBuilderGstreamer.h"
#endif

namespace NPluginLoader
{
	bool _init = FALSE;
	QStringList _identifiers;
	NPlaybackEngineInterface *_playback = NULL;
	NWaveformBuilderInterface *_waveform = NULL;

	QString _playbackPrefer = "GStreamer";
	QString _wavefowmPrefer = "GStreamer";

	void _loadPlugins();
	QList<QPluginLoader *> _loaders;
}

void NPluginLoader::deinit()
{
	for (int i = 0; i < _loaders.size(); ++i) {
		if (_loaders[i]) {
			_loaders.at(i)->unload();
			delete _loaders[i];
			_loaders[i] = NULL;
		}
	}
}

void NPluginLoader::_loadPlugins()
{
	if (_init)
		return;
	_init = TRUE;

	QObjectList objects;
	QList<bool> usedFlags;

	QObjectList objectsStatic;
#ifdef _N_GSTREAMER_PLUGINS_BUILTIN_
	objectsStatic << new NPlaybackEngineGStreamer() << new NWaveformBuilderGstreamer();
#endif
	objectsStatic << QPluginLoader::staticInstances();

	foreach (QObject *obj, objectsStatic) {
		NPluginInterface *plugin = qobject_cast<NPluginInterface *>(obj);
		if (plugin) {
			objects << obj;
			qobject_cast<NPluginInterface *>(obj)->init();
			QString id = plugin->identifier();
			id.insert(id.lastIndexOf('/'), " (Built-in)");
			_identifiers << id;
			_loaders << NULL;
			usedFlags << TRUE;
		}
	}

	QStringList pluginsDirList;
	pluginsDirList << "plugins";
#ifndef Q_WS_WIN
	if (NCore::rcDir() != QCoreApplication::applicationDirPath())
		pluginsDirList << NCore::rcDir() + "/plugins";
	if (QDir(QCoreApplication::applicationDirPath()).dirName() == "bin")
		pluginsDirList << "../lib/nulloy/plugins";
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
				subDirsList.join(";") + ";" + getenv("PATH")).toAscii());
#endif

	foreach (QString dirStr, pluginsDirList) {
		QDir dir(dirStr);
		if (dir.exists()) {
			foreach (QString fileName, dir.entryList(QDir::Files)) {
				if (!QLibrary::isLibrary(fileName))
					continue;
				QPluginLoader *loader = new QPluginLoader(dir.absoluteFilePath(fileName));
				QObject *obj = loader->instance();
				NPluginInterface *plugin = qobject_cast<NPluginInterface *>(obj);
				if (plugin) {
					objects << obj;
					_identifiers << plugin->identifier();
					_loaders << loader;
					usedFlags << FALSE;
				} else {
					QMessageBox box(QMessageBox::Warning, QObject::tr("Plugin loading error"), QObject::tr("Failed to load plugin: ") +
									dir.absoluteFilePath(fileName) + "\n\n" + loader->errorString(), QMessageBox::Close);
					box.exec();
					delete loader;
				}
			}
		}
	}

	int index;

	QString playbackStr = NSettings::instance()->value("Playback").toString();
	index = _identifiers.indexOf("Nulloy/Playback/" + playbackStr);
	if (index == -1)
		index = _identifiers.indexOf(QRegExp("Nulloy/Playback/" + _playbackPrefer + ".*"));
	if (index == -1)
		index = _identifiers.indexOf(QRegExp("Nulloy/Playback.*"));
	if (index != -1) {
		QString interface = qobject_cast<NPluginInterface *>(objects.at(index))->interface();
		if (interface != NPlaybackEngineInterface::interface()) {
			QMessageBox::warning(NULL, QObject::tr("Plugin Interface Mismatch"),
				_identifiers.at(index).section('/', 2, 2) + " " +
				_identifiers.at(index).section('/', 1, 1) + " plugin has a different version of " +
				_identifiers.at(index).section('/', 1, 1) + " interface.\n" +
				"Internal version: " + NPlaybackEngineInterface::interface().section('/', 2, 2) + "\n" +
				"Plugin version: " + _identifiers.at(index).section('/', 3, 3),
				QMessageBox::Close);
		}

		_playback = qobject_cast<NPlaybackEngineInterface *>(objects.at(index));
		qobject_cast<NPluginInterface *>(objects.at(index))->init();
		usedFlags[index] = TRUE;
		NSettings::instance()->setValue("Playback", _identifiers.at(index).section('/', 2));
	}

	QString waveformStr = NSettings::instance()->value("Waveform").toString();
	index = _identifiers.indexOf("Nulloy/Waveform/" + waveformStr);
	if (index == -1)
		index = _identifiers.indexOf(QRegExp("Nulloy/Waveform/" + _wavefowmPrefer + ".*"));
	if (index == -1)
		index = _identifiers.indexOf(QRegExp("Nulloy/Waveform.*"));
	if (index != -1) {
		QString interface = qobject_cast<NPluginInterface *>(objects.at(index))->interface();
		if (interface != NWaveformBuilderInterface::interface()) {
			QMessageBox::warning(NULL, QObject::tr("Plugin Interface Mismatch"),
				_identifiers.at(index).section('/', 2, 2) + " " +
				_identifiers.at(index).section('/', 1, 1) + " plugin has a different version of " +
				_identifiers.at(index).section('/', 1, 1) + " interface.\n" +
				"Internal version: " + NWaveformBuilderInterface::interface().section('/', 2, 2) + "\n" +
				"Plugin version: " + _identifiers.at(index).section('/', 3, 3),
				QMessageBox::Close);
		}

		_waveform = qobject_cast<NWaveformBuilderInterface *>(objects.at(index));
		qobject_cast<NPluginInterface *>(objects.at(index))->init();
		usedFlags[index] = TRUE;
		NSettings::instance()->setValue("Waveform", _identifiers.at(index).section('/', 2));
	}

	for (int i = 0; i < _loaders.size(); ++i) {
		if (usedFlags.at(i) == FALSE) {
			_loaders.at(i)->unload();
			delete _loaders[i];
			_loaders[i] = NULL;
		}
	}

	if (!_waveform || !_playback) {
		QStringList message;
		if (!_waveform)
			message << QObject::tr("No Waveform plugin found.");
		if (!_playback)
			message << QObject::tr("No Playback plugin found.");
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

QStringList NPluginLoader::pluginIdentifiers()
{
	_loadPlugins();
	return _identifiers;
}

/* vim: set ts=4 sw=4: */
