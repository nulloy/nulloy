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

#include <QString>
#include <windows.h>
#include <shellapi.h>

int _trash(const QString &path, QString *error)
{
	QString new_path = path;
	new_path.append("00");
	new_path[new_path.size() - 2] = 0;
	new_path[new_path.size() - 1] = 0;
	SHFILEOPSTRUCT shfo = {0};
	shfo.wFunc = FO_DELETE;
	shfo.pFrom = (wchar_t *)(new_path.utf16());
	shfo.fFlags = FOF_NOCONFIRMATION | FOF_SIMPLEPROGRESS | FOF_NOERRORUI | FOF_ALLOWUNDO;
	shfo.fAnyOperationsAborted = FALSE;
	shfo.hNameMappings = NULL;
	shfo.pTo = NULL;
	shfo.lpszProgressTitle = NULL;
	return SHFileOperation(&shfo);
}

