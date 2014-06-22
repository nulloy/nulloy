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

#ifndef N_I18N_LOADER_H
#define N_I18N_LOADER_H

#include <QLocale>

namespace NI18NLoader
{
	QList<QLocale::Language> translations();
	void loadTranslation(QLocale::Language language = QLocale::AnyLanguage);
}

#endif

