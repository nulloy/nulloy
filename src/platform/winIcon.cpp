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
#include "winIcon.h"

#include <QDir>
#include <QPixmap>
#include <QVector>
#include <QtWin>

#include <shellapi.h>

QList<QIcon> NWinIcon::getIcons(const QString &dllPath)
{
    QList<QIcon> icons;

    if (!QFileInfo::exists(dllPath)) {
        return icons;
    }

    const wchar_t *path = reinterpret_cast<const wchar_t *>(
        QDir::toNativeSeparators(dllPath).utf16());
    const UINT count = ExtractIconEx(path, -1, 0, 0, 0);
    if (count == 0) {
        return icons;
    }

    QVector<HICON> large(count);
    QVector<HICON> small(count);

    ExtractIconEx(path, 0, large.data(), small.data(), count);

    for (int i = 0; i < count; ++i) {
        QIcon icon;
        HICON hIcon;

        hIcon = small[i];
        icon.addPixmap(QtWin::fromHICON(hIcon));
        DestroyIcon(hIcon);

        hIcon = large[i];
        icon.addPixmap(QtWin::fromHICON(hIcon));
        DestroyIcon(hIcon);

        icons.append(icon);
    }

    return icons;
}
