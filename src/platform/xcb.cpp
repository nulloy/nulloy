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

#include "xcb.h"
#include <QDebug>
#include <xcb/xcb.h>

xcb_atom_t get_atom(xcb_connection_t *connection, const char *name)
{
    xcb_atom_t atom;
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, 0, (uint16_t)strlen(name), name);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie, NULL);
    if (!reply) {
        qCritical() << "xcb_intern_atom failed";
    }

    atom = reply->atom;
    free(reply);
    return atom;
}

void *get_property(xcb_connection_t *const connection, xcb_window_t window_id, xcb_atom_t atom,
                   xcb_atom_t type)
{
    xcb_get_property_cookie_t cookie = xcb_get_property(connection, 0, window_id, atom, type, 0,
                                                        128);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(connection, cookie, NULL);
    if (!reply) {
        qCritical() << "xcb_get_property_reply failed";
    }
    return xcb_get_property_value(reply);
}

QString NXcb::wmName()
{
    xcb_connection_t *connection = xcb_connect(NULL, NULL);

    xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

    xcb_window_t *windows = (xcb_window_t *)get_property(connection, screen->root,
                                                         get_atom(connection,
                                                                  "_NET_SUPPORTING_WM_CHECK"),
                                                         XCB_GET_PROPERTY_TYPE_ANY);

    xcb_atom_t wm_name_atom = get_atom(connection, "_NET_WM_NAME");
    xcb_atom_t utf8_string_atom = get_atom(connection, "UTF8_STRING");
    char *name = (char *)get_property(connection, *windows, wm_name_atom, utf8_string_atom);
    return QString(name);
}
