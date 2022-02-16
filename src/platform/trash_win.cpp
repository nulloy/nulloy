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

// clang-format off
#include <windows.h>
#include <shellapi.h>
// clang-format on

#include <QString>

int _trash(const QString &file, QString *error)
{
    Q_UNUSED(error);
    QString file_nul = file;
    file_nul.append("00");
    file_nul[file.size()] = 0;
    file_nul[file.size() + 1] = 0;
    SHFILEOPSTRUCT shfo = SHFILEOPSTRUCT();
    shfo.wFunc = FO_DELETE;
    shfo.pFrom = (wchar_t *)(file_nul.utf16());
    shfo.fFlags = FOF_NOCONFIRMATION | FOF_SIMPLEPROGRESS | FOF_NOERRORUI | FOF_ALLOWUNDO;
    shfo.fAnyOperationsAborted = false;
    shfo.hNameMappings = NULL;
    shfo.pTo = NULL;
    shfo.lpszProgressTitle = NULL;
    return SHFileOperation(&shfo);
}
