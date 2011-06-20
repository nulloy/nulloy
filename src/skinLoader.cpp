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

#include "skinLoader.h"

#include "skinFileSystem.h"
#include "rcDir.h"
#include <qtiocompressor.h>

#include <QBuffer>
#include <QDir>
#include <QMessageBox>
#include <QCoreApplication>

#include <QDebug>

static bool _init = FALSE;
static QMap<int, QString> _identifiers;
static QString _uiFormFile;
static QString _scriptFile;

static QString _skinPrefer = "Silver";
static QString _skinSuffix= "nzs";

static bool _nextFile(QFile &zipFile, QString &fileName, QByteArray &data)
{
	quint32 signature, crc, compSize, unCompSize;
	quint16 extractVersion, bitFlag, compMethod, modTime, modDate;
	quint16 nameLen, extraLen;

	QDataStream in(&zipFile);
	in.setByteOrder(QDataStream::LittleEndian);

	in >> signature;
	if (signature != 0x04034b50)
		return FALSE;

	in >> extractVersion >> bitFlag >> compMethod;
	in >> modTime >> modDate >> crc >> compSize >> unCompSize;
	in >> nameLen >> extraLen;

	fileName = QString(zipFile.read(nameLen));
	zipFile.read(extraLen);

	QByteArray compData = zipFile.read(compSize);
	QByteArray unCompData;
	if (compMethod == 0) {
		unCompData = compData;
	} else {
		QBuffer compBuf(&compData);
		QtIOCompressor compressor(&compBuf);
		compressor.setStreamFormat(QtIOCompressor::RawZipFormat);
		compressor.open(QIODevice::ReadOnly);
		unCompData = compressor.readAll();
	}

	data = unCompData;

	return TRUE;
}

static void _loadSkins(QSettings *settings)
{
	if (_init)
		return;
	_init = TRUE;

	QStringList skinsDirList;
	skinsDirList << ":skins" << "skins";
#if !defined WIN32 && !defined _WINDOWS && !defined Q_WS_WIN
	if (rcDir() != QCoreApplication::applicationDirPath())
		skinsDirList << rcDir() + "/skins";
	if (QDir(QCoreApplication::applicationDirPath()).dirName() == "bin")
		skinsDirList << "../share/nulloy/skins";
#endif

	QFileInfoList containersInfoList;
	foreach (QString dirStr, skinsDirList) {
		QDir dir(dirStr);
		if (dir.exists())
			containersInfoList << dir.entryInfoList(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
	}

	for (int i = 0; i < containersInfoList.size(); ++i) {
		QFileInfo skinContainer = containersInfoList.at(i);
		QString id;
		if (!skinContainer.isDir()) {
			if (skinContainer.suffix() == _skinSuffix) {
				QFile zipFile(skinContainer.absoluteFilePath());
				zipFile.open(QIODevice::ReadOnly);
				QString fileName;
				QByteArray data;
				while (_nextFile(zipFile, fileName, data)) {
					if (fileName == "id.txt") {
						id = data.mid(0, data.indexOf('\n')).replace('\r', "");
						break;
					}
				}
				zipFile.close();
			} else {
				containersInfoList.removeAt(i);
				--i;
				continue;
			}
		} else if (skinContainer.isDir()) {
			QFileInfoList infoList = QDir(skinContainer.absoluteFilePath()).entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
			bool isSkin = FALSE;
			foreach (QFileInfo fileInfo, infoList) {
				if (fileInfo.fileName() == "id.txt") {
					isSkin = TRUE;
					QFile file(fileInfo.absoluteFilePath());
					file.open(QIODevice::ReadOnly);
					QByteArray data = file.readAll();
					id = data.mid(0, data.indexOf('\n')).replace('\r', "");
					file.close();
					break;
				}
			}
			if (!isSkin) {
				containersInfoList.removeAt(i);
				--i;
				continue;
			}
		}
		if (skinContainer.absoluteFilePath().startsWith(":skins"))
			id.insert(id.lastIndexOf('/'), " (Built-in)");
		_identifiers.insert(i, id);
	}

	QString skinStr = settings->value("GUI/Skin").toString();
	QStringList values = _identifiers.values();
	int index;
	index = values.indexOf(QRegExp("Nulloy/Skin/" + skinStr));
	if (index == -1)
		index = values.indexOf(QRegExp("Nulloy/Skin/" + _skinPrefer + ".*"));
	if (index == -1)
		index = 0;

	if (_identifiers.count() == 0) {
		QMessageBox box(QMessageBox::Critical, "Skin loading error", "No skins found.", QMessageBox::Close);
		box.exec();
		return;
	}

	QFileInfo skinContainer = containersInfoList.at(index);
	if (!skinContainer.isDir() && skinContainer.suffix() == _skinSuffix) {
		QFile zipFile(skinContainer.absoluteFilePath());
		zipFile.open(QIODevice::ReadOnly);
		QString fileName;
		QByteArray data;
		while (_nextFile(zipFile, fileName, data)) {
			if (fileName == "script.js" || fileName == "form.ui") {
				QString str(data);
				QRegExp rx("(url\\()([^:])");
				str.replace(rx, "\\1" + NSkinFileSystem::prefix() + "\\2");
				NSkinFileSystem::addFile(fileName, str.toUtf8());

				if (fileName == "form.ui")
					_uiFormFile = NSkinFileSystem::prefix() + fileName;
				else if (fileName == "script.js")
					_scriptFile = NSkinFileSystem::prefix() + fileName;
			} else {
				NSkinFileSystem::addFile(fileName, data);
			}
		}
		zipFile.close();
	} else if (skinContainer.isDir()) {
		QFileInfoList infoList = QDir(skinContainer.absoluteFilePath()).entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
		foreach (QFileInfo fileInfo, infoList) {
			if (fileInfo.fileName() == "script.js" || fileInfo.fileName() == "form.ui") {
				QFile file(fileInfo.absoluteFilePath());
				file.open(QIODevice::ReadOnly);
				QByteArray data = file.readAll();
				QString str(data);
				QRegExp rx("(url\\()([^:])");
				str.replace(rx, "\\1" + fileInfo.absolutePath() + "/\\2");
				str.replace("<iconset resource=\"resources.qrc\">", "<iconset>");
				NSkinFileSystem::addFile(fileInfo.fileName(), str.toUtf8());

				if (fileInfo.fileName() == "form.ui")
					_uiFormFile = NSkinFileSystem::prefix() + fileInfo.fileName();
				else if (fileInfo.fileName() == "script.js")
					_scriptFile = NSkinFileSystem::prefix() + fileInfo.fileName();
			}
		}
	}

	settings->setValue("GUI/Skin", _identifiers.value(index).section('/', 2));
}

QStringList skinIdentifiers(QSettings *settings)
{
	_loadSkins(settings);
	return _identifiers.values();
}

QString skinUiFormFile(QSettings *settings)
{
	_loadSkins(settings);
	return _uiFormFile;
}

QString skinScriptFile(QSettings *settings)
{
	_loadSkins(settings);
	return _scriptFile;
}

/* vim: set ts=4 sw=4: */
