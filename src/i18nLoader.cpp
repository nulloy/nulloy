/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2014 Sergey Vlasov <sergey@vlasov.me>
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

#include "i18nLoader.h"

#include "common.h"
#include "settings.h"

#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QTranslator>

namespace NI18NLoader
{
	bool __init = FALSE;
	QMap<QLocale::Language, QString> _translations;
	QTranslator _translator;
	void _init();
}

void NI18NLoader::_init()
{
	if (__init)
		return;
	__init = TRUE;

	_translations[QLocale::English] = "";

	QStringList langDirList;
	langDirList << QCoreApplication::applicationDirPath() + "/i18n";
#ifndef Q_WS_WIN
	if (NCore::rcDir() != QCoreApplication::applicationDirPath())
		langDirList << NCore::rcDir() + "/i18n";
	if (QDir(QCoreApplication::applicationDirPath()).dirName() == "bin") {
		QDir dir(QCoreApplication::applicationDirPath());
		dir.cd("../share/nulloy/i18n");
		langDirList << dir.absolutePath();
	}
#endif

	foreach (QString dirStr, langDirList) {
		QDir dir(dirStr);
		if (!dir.exists())
			continue;
		foreach (QString fileName, dir.entryList(QStringList("*.qm"), QDir::Files)) {
			QFileInfo fileInfo(fileName);
			QLocale locale(fileInfo.baseName());
			_translations[locale.language()] = dir.absoluteFilePath(fileName);
		}
	}

	QString settingsLanguageTag = NSettings::instance()->value("Language", "").toString();
	QLocale locale = QLocale(settingsLanguageTag);
	if (settingsLanguageTag.isEmpty())
		locale = QLocale::system();
	if (!_translations.contains(locale.language()))
		locale = QLocale(QLocale::English);
	NSettings::instance()->setValue("Language", locale.bcp47Name().split('-').first());
}

QList<QLocale::Language> NI18NLoader::translations()
{
	_init();
	return _translations.keys();
}

void NI18NLoader::loadTranslation(QLocale::Language language)
{
	_init();

	if (language == QLocale::AnyLanguage) {
		QString settingsLanguageTag = NSettings::instance()->value("Language", "").toString();
		QLocale locale = QLocale(settingsLanguageTag);
		language = locale.language();
	}

	if (language != QLocale::English) {
		QFileInfo fileInfo(_translations[language]);
		_translator.load(fileInfo.baseName(), fileInfo.absolutePath());
		QCoreApplication::instance()->installTranslator(&_translator);
	} else {
		QCoreApplication::instance()->removeTranslator(&_translator);
	}
}

