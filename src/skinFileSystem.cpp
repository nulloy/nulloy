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

#include "skinFileSystem.h"

#include <QByteArray>
#include <QDebug>
#include <QString>

QHash<QString, QByteArray> NSkinFileSystem::m_fileHash;
NSkinFileSystem *NSkinFileSystem::m_instance;

NSkinFileSystem::NSkinFileSystem(QObject *parent) : QObject(parent)
{
    Q_ASSERT_X(!m_instance, "NSkinFileSystem", "NSkinFileSystem instance already exists.");

    m_instance = this;
}

NSkinFileSystem *NSkinFileSystem::instance()
{
    if (!m_instance) {
        m_instance = new NSkinFileSystem();
    }

    return m_instance;
}

void NSkinFileSystem::addFile(const QString &filePath, const QByteArray &ba)
{
    m_fileHash[filePath] = ba;
}

QByteArray NSkinFileSystem::readFile(const QString &filePath)
{
    if (m_fileHash.contains(filePath)) {
        return m_fileHash.value(filePath);
    } else {
        qCritical() << "NSkinFileSystem: file not found:" << filePath;
        return QByteArray();
    }
}
