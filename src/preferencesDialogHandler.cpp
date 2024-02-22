/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2024 Sergey Vlasov <sergey@vlasov.me>
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

#include "preferencesDialogHandler.h"

#include "global.h"
#include "i18nLoader.h"
#include "player.h"
#include "pluginLoader.h"
#include "settings.h"
#include "shortcutEditorModel.h"

#ifndef _N_NO_SKINS_
#include "skinLoader.h"
#endif

#include <QStyleFactory>
#include <QTextCodec>

using namespace NPluginLoader;

static const char *LANGUAGE = QT_TRANSLATE_NOOP("PreferencesDialog", "English");

NPreferencesDialogHandler::NPreferencesDialogHandler(NPlayer *player, QObject *parentWindow)
    : NDialogHandler(QUrl::fromLocalFile(":src/preferencesDialog.qml"), parentWindow)
{
    m_player = player;

    connect(this, &NDialogHandler::beforeShown, [this](QQmlContext *context) {
        { // translations >>
            QList<QVariantMap> data;
            foreach (QLocale::Language language, NI18NLoader::translations()) {
                QString languageString = QLocale::languageToString(language);
                QString localizedString = NI18NLoader::translate(language, "preferencesDialog",
                                                                 LANGUAGE);
                QVariantMap item;
                item["text"] = QString("%1 (%2)").arg(localizedString).arg(languageString);
                item["value"] = QLocale(language).bcp47Name().split('-').first();
                data << item;
            }
            context->setContextProperty("NLanguagesModel", QVariant::fromValue(data));
        }

#ifndef _N_NO_SKINS_
        { // skins
            QList<QVariantMap> data;
            foreach (QString str, NSkinLoader::skinIdentifiers()) {
                QString id = str.section('/', 2);
                QVariantMap item;
                item["text"] = QString(id).replace('/', ' ').replace(" (Built-in)",
                                                                     tr(" (Built-in)"));
                item["value"] = id;
                data << item;
            }
            context->setContextProperty("NSkinsModel", QVariant::fromValue(data));
        }
#endif

        { // styles
            QStringList data;
            foreach (QString str, QStyleFactory::keys()) {
                data << str;
            }
            context->setContextProperty("NStylesModel", QVariant::fromValue(data));
        }

        { // encodings
            QStringList data;
            foreach (int mib, QTextCodec::availableMibs()) {
                data << QTextCodec::codecForMib(mib)->name();
            }
            context->setContextProperty("NEncodingsModel", QVariant::fromValue(data));
        }

        { // plugins
            QList<QVariantMap> data;
            QList<Descriptor> descriptors = NPluginLoader::descriptors();
            NFlagIterator<N::PluginType> iter(N::MaxPlugin);
            while (iter.hasNext()) {
                iter.next();
                N::PluginType type = iter.value();
                QString typeString = ENUM_TO_STR(N, PluginType, type);

                QList<int> indexesFilteredByType;
                for (int i = 0; i < descriptors.count(); ++i) {
                    if (descriptors.at(i)[TypeRole] == type) {
                        indexesFilteredByType << i;
                    }
                }

                QStringList pluginsList;
                foreach (int i, indexesFilteredByType) {
                    QString containerName = descriptors.at(i)[ContainerNameRole].toString();
                    pluginsList << containerName;
                }
                QVariantMap item;
                item["pluginType"] = typeString;
                item["plugins"] = pluginsList;
                data << item;
            }
            context->setContextProperty("NPluginsModel", QVariant::fromValue(data));
        }

        { // shortcuts
            m_shortcutEditorModel = new NShortcutEditorModel(m_player);
            context->setContextProperty("NShortcutEditorModel", m_shortcutEditorModel);
        }

        context->setContextProperty("NPreferencesDialogHandler", this);

        m_settingsOveraly = new NSettingsOverlay(this);
        context->setContextProperty("NSettings", m_settingsOveraly);
    });

    connect(this, &NDialogHandler::afterShown, [this](QObject *root) {
        connect(root, SIGNAL(apply()), this, SLOT(applySettings()));
        connect(root, SIGNAL(accepted()), this, SLOT(applySettings()));
    });
}

void NPreferencesDialogHandler::applySettings()
{
    m_settingsOveraly->commit();
    m_shortcutEditorModel->applyShortcuts();
    emit settingsApplied();
}
