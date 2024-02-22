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

#ifndef N_PREFERENCES_DIALOG_HANDLER_H
#define N_PREFERENCES_DIALOG_HANDLER_H

#include "dialogHandler.h"

class NPlayer;
class NSettingsOverlay;
class NShortcutEditorModel;

class NPreferencesDialogHandler : public NDialogHandler
{
    Q_OBJECT

public:
    NPreferencesDialogHandler(NPlayer *player, QObject *parentWindow = nullptr);

signals:
    void settingsApplied();

private slots:
    void applySettings();

private:
    NPlayer *m_player;
    NSettingsOverlay *m_settingsOveraly;
    NShortcutEditorModel *m_shortcutEditorModel;
};

#endif
