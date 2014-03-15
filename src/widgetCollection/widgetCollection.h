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

#include <QDesignerCustomWidgetCollectionInterface>
#include <QDesignerCustomWidgetInterface>

class NDropArea;
class NLabel;
class NPlaylistWidget;
class NSlider;
class NWaveformSlider;
class NCoverWidget;
class QSizeGrip;

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
	virtual bool isContainer() const { return FALSE; }
	virtual QIcon icon() const { return QIcon(); }
	virtual QString whatsThis() const { return QString(); }
	virtual QString toolTip() const { return QString(); }
	virtual QString group() const { return "Nulloy"; }
	virtual QString domXml() const { return m_domXml; }
	virtual QString includeFile() const { return m_header; }
};

class NLabelPlugin : public QObject, public NWidgetPlugin
{
	Q_OBJECT

public:
	NLabelPlugin(QObject *parent = 0) : QObject(parent), NWidgetPlugin(this->metaObject()->className()) {}
	virtual QWidget *createWidget(QWidget *parent);
};

class NDropAreaPlugin : public QObject, public NWidgetPlugin
{
	Q_OBJECT

public:
	NDropAreaPlugin(QObject *parent = 0) : QObject(parent), NWidgetPlugin(this->metaObject()->className()) {}
	virtual bool isContainer() const { return TRUE; }
	virtual QWidget* createWidget(QWidget *parent);
};

class NPlaylistWidgetPlugin : public QObject, public NWidgetPlugin
{
	Q_OBJECT

public:
	NPlaylistWidgetPlugin(QObject *parent = 0) : QObject(parent), NWidgetPlugin(this->metaObject()->className()) {}
	virtual QWidget* createWidget(QWidget *parent);
};

class NSliderPlugin : public QObject, public NWidgetPlugin
{
	Q_OBJECT

public:
	NSliderPlugin(QObject *parent = 0) : QObject(parent), NWidgetPlugin(this->metaObject()->className()) {}
	virtual QWidget* createWidget(QWidget *parent);
};

class NWaveformSliderPlugin : public QObject, public NWidgetPlugin
{
	Q_OBJECT

public:
	NWaveformSliderPlugin(QObject *parent = 0) : QObject(parent), NWidgetPlugin(this->metaObject()->className()) {}
	virtual QWidget* createWidget(QWidget *parent);
};

class NCoverWidgetPlugin : public QObject, public NWidgetPlugin
{
	Q_OBJECT

public:
	NCoverWidgetPlugin(QObject *parent = 0) : QObject(parent), NWidgetPlugin(this->metaObject()->className()) {}
	virtual QWidget* createWidget(QWidget *parent);
};

class QSizeGripPlugin : public QObject, public NWidgetPlugin
{
	Q_OBJECT

public:
	QSizeGripPlugin(QObject *parent = 0) : QObject(parent), NWidgetPlugin(this->metaObject()->className()) {}
	virtual QWidget* createWidget(QWidget *parent);
};

class NWidgetCollection: public QObject, public QDesignerCustomWidgetCollectionInterface
{
	Q_OBJECT
	Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

public:
	NWidgetCollection(QObject *parent = 0);
	virtual QList<QDesignerCustomWidgetInterface *> customWidgets() const { return m_plugins; }

private:
	QList<QDesignerCustomWidgetInterface *> m_plugins;
};

