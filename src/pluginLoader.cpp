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
#include "pluginElementInterface.h"

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
	NTagReaderInterface *_tagReader = NULL;

	void _loadPlugins();
	QObject* _findPlugin(PluginType type, QObjectList &objects, QMap<QString, bool> &usedFlags);
	QMap<QString, QPluginLoader *> _loaders;
}

void NPluginLoader::deinit()
{
	foreach (QString key, _loaders.keys()) {
		if (_loaders[key]) {
			QStringList other_keys = _loaders.keys(_loaders[key]);
			_loaders[key]->unload();
			delete _loaders[key];
			_loaders[key] = NULL;

			foreach (QString other_key, other_keys)
				_loaders[other_key] = NULL;
		}
	}
	foreach (QString null_key, _loaders.keys(NULL))
		_loaders.remove(null_key);
}

QObject* NPluginLoader::_findPlugin(PluginType type, QObjectList &objects, QMap<QString, bool> &usedFlags) {
	QString base_interface;
	QString type_str;
	if (type == PlaybackEngine) {
		base_interface = NPlaybackEngineInterface::interface();
		type_str = "Playback";
	} else if (type == WaveformBuilder) {
		base_interface = NWaveformBuilderInterface::interface();
		type_str = "Waveform";
	} else if (type == TagReader) {
		base_interface = NTagReaderInterface::interface();
		type_str = "TagReader";
	}

	int index;
	QString type_num = QString::number(type);
	QString str = NSettings::instance()->value(type_str).toString();
	index = _identifiers.indexOf(QRegExp(type_num + "/" + str + "/.*"));
	if (index == -1)
		index = _identifiers.indexOf(QRegExp(type_num + "/GStreamer/.*"));
	if (index == -1)
		index = _identifiers.indexOf(QRegExp(type_num + "/.*"));
	if (index != -1) {
		NPluginElementInterface *el = qobject_cast<NPluginElementInterface *>(objects.at(index));

		QString identifier = _identifiers.at(index);
		QString plug_name = identifier.section('/', 1, 1);
		QString plug_ver = identifier.section('/', 2, 2);
		QString el_interface_ver = el->interface().section('/', 2, 2);

		QString base_interface_name = base_interface.section('/', 1, 1);
		QString base_interface_ver = base_interface.section('/', 2, 2);

		if (el_interface_ver != base_interface_ver) {
			QMessageBox::warning(NULL, QObject::tr("Plugin Interface Mismatch"),
								plug_name + " " + plug_ver + " plugin has a different version of " + base_interface_name +".\n" +
								"Internal version: " + base_interface_ver + "\n" +
								"Plugin version: " + el_interface_ver,
								QMessageBox::Close);
		}

		el->init();
		usedFlags[identifier] = TRUE;

		NSettings::instance()->setValue(type_str, plug_name + "/" + plug_ver);

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
		NPluginElementInterface *plugin = qobject_cast<NPluginElementInterface *>(obj);
		if (plugin) {
			objects << obj;
			qobject_cast<NPluginElementInterface *>(obj)->init();
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
					QObjectList elements = plugin->elements();
					objects << elements;
					foreach (QObject *obj, elements) {
						NPluginElementInterface *el = qobject_cast<NPluginElementInterface *>(obj);
						QString identifier = QString::number(el->type()) + "/" + plugin->name() + "/" + plugin->version() +
										((el->type() == Other) ? "" : "/" + el->name());
						_identifiers << identifier;
						_loaders[identifier] = loader;
						usedFlags[identifier] = FALSE;
					}
				} else {
					QMessageBox box(QMessageBox::Warning, QObject::tr("Plugin loading error"), QObject::tr("Failed to load plugin: ") +
									dir.absoluteFilePath(fileName) + "\n\n" + loader->errorString(), QMessageBox::Close);
					box.exec();
					delete loader;
				}
			}
		}
	}

	_playback = qobject_cast<NPlaybackEngineInterface *>(_findPlugin(PlaybackEngine, objects, usedFlags));
	_waveform = qobject_cast<NWaveformBuilderInterface *>(_findPlugin(WaveformBuilder, objects, usedFlags));
	_tagReader = qobject_cast<NTagReaderInterface *>(_findPlugin(TagReader, objects, usedFlags));

	QStringList used_keys;
	foreach (QString key, usedFlags.keys(TRUE))
		used_keys << key <<_loaders.keys(_loaders[key]);
	used_keys.removeDuplicates();

	foreach (QString key, _loaders.keys()) {
		if (!used_keys.contains(key) && _loaders[key]) {
			QStringList unused_keys = _loaders.keys(_loaders[key]);
			_loaders[key]->unload();
			delete _loaders[key];
			_loaders[key] = NULL;

			foreach (QString unused_key, unused_keys)
				_loaders[unused_key] = NULL;
		}
	}

	foreach (QString null_key, _loaders.keys(NULL))
		_loaders.remove(null_key);

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

QStringList NPluginLoader::pluginIdentifiers()
{
	_loadPlugins();
	return _identifiers;
}

/* vim: set ts=4 sw=4: */
