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

#include "i18nLoader.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QTranslator>

#include "common.h"
#include "settings.h"

static const char _i18nDirName[] = "i18n";

namespace NI18NLoader
{
    bool _init = false;
    QMap<QLocale::Language, QString> _translations;
    QTranslator _translator;
} // namespace NI18NLoader

void NI18NLoader::init()
{
    if (_init) {
        return;
    }
    _init = true;

    _translations[QLocale::English] = "";

    // find directories
    QStringList langDirList;
    langDirList << QCoreApplication::applicationDirPath() + "/" + _i18nDirName;
#ifndef Q_OS_WIN
    if (NCore::rcDir() != QCoreApplication::applicationDirPath()) {
        langDirList << NCore::rcDir() + "/" + _i18nDirName;
    }
    if (QDir(QCoreApplication::applicationDirPath()).dirName() == "bin") {
        QDir dir(QCoreApplication::applicationDirPath());
        dir.cd(QString() + "../share/nulloy/" + _i18nDirName);
        langDirList << dir.absolutePath();
    }
#endif

    // populate .qm files
    foreach (QString dirStr, langDirList) {
        QDir dir(dirStr);
        if (!dir.exists()) {
            continue;
        }
        QStringList fileNames = dir.entryList(QStringList("*.qm"), QDir::Files);
        foreach (QString fileName, fileNames) {
            QFileInfo fileInfo(fileName);
            QLocale locale(fileInfo.baseName());
            _translations[locale.language()] = dir.absoluteFilePath(fileName);
        }
    }

    // detect system locale and save to settings
    QString settingsLanguageTag = NSettings::instance()->value("Language", "").toString();
    QLocale locale = QLocale(settingsLanguageTag);
    if (settingsLanguageTag.isEmpty()) {
        locale = QLocale::system();
    }
    if (!_translations.contains(locale.language())) {
        locale = QLocale(QLocale::English);
    }
    NSettings::instance()->setValue("Language", locale.bcp47Name().split('-').first());

    // load translation
    if (locale.language() != QLocale::English) {
        QFileInfo fileInfo(_translations[locale.language()]);
        _translator.load(fileInfo.baseName(), fileInfo.absolutePath());
        QCoreApplication::instance()->installTranslator(&_translator);
    }
}

QList<QLocale::Language> NI18NLoader::translations()
{
    init();
    return _translations.keys();
}

QString NI18NLoader::translate(QLocale::Language language, const char *context,
                               const char *sourceText)
{
    init();

    if (language == QLocale::English || !_translations.contains(language)) {
        return sourceText;
    }

    QFileInfo fileInfo(_translations[language]);
    QTranslator translator;
    translator.load(fileInfo.baseName(), fileInfo.absolutePath());
    QCoreApplication::instance()->installTranslator(&translator);
    return QCoreApplication::translate(context, sourceText);
}
