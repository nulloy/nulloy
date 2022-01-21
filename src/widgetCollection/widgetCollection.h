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

#ifndef N_WIDGET_COLLECTION_H
#define N_WIDGET_COLLECTION_H

#include <QtGlobal>
#if QT_VERSION < QT_VERSION_CHECK(5, 2, 2)
#include <QtDesigner/QDesignerCustomWidgetCollectionInterface>
#include <QtDesigner/QDesignerCustomWidgetInterface>
#else
#include <QtUiPlugin/QDesignerCustomWidgetCollectionInterface>
#include <QtUiPlugin/QDesignerCustomWidgetInterface>
#endif

class NWidgetPlugin : public QDesignerCustomWidgetInterface
{
    Q_INTERFACES(QDesignerCustomWidgetInterface)

private:
    bool m_initialized;
    QString m_className;
    QString m_domXml;
    QString m_header;

protected:
    NWidgetPlugin(const QString &className);

public:
    QString name() const { return m_className; }
    void initialize(QDesignerFormEditorInterface *core);
    bool isInitialized() const { return m_initialized; }
    virtual QIcon icon() const { return QIcon(); }
    virtual QString whatsThis() const { return QString(); }
    virtual QString toolTip() const { return QString(); }
    virtual QString group() const { return "Nulloy"; }
    virtual QString domXml() const { return m_domXml; }
    virtual QString includeFile() const { return m_header; }
};

class NWidgetCollection : public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)
    Q_PLUGIN_METADATA(IID "com.nulloy.NWidgetCollection")

public:
    NWidgetCollection(QObject *parent = 0);
    virtual QList<QDesignerCustomWidgetInterface *> customWidgets() const { return m_plugins; }

private:
    QList<QDesignerCustomWidgetInterface *> m_plugins;
};

#endif
