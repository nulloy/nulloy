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

#ifndef N_LABEL_PLUGIN_H
#define N_LABEL_PLUGIN_H

#include <QDesignerCustomWidgetInterface>

class NLabelPlugin : public QObject, public QDesignerCustomWidgetInterface
{
	Q_OBJECT
	Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
	explicit NLabelPlugin(const QString &group, QObject *parent = 0);

	virtual bool isContainer() const;
	virtual bool isInitialized() const;
	virtual QIcon icon() const;
	virtual QString domXml() const;
	virtual QString group() const;
	virtual QString includeFile() const;
	virtual QString name() const;
	virtual QString toolTip() const;
	virtual QString whatsThis() const;
	virtual QWidget *createWidget(QWidget *parent);
	virtual void initialize(QDesignerFormEditorInterface *core);

private:
	QString m_group;
	bool m_initialized;
};

#endif

/* vim: set ts=4 sw=4: */
