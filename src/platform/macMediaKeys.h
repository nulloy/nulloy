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

#ifndef N_MAC_MEDIA_KEYS_H
#define N_MAC_MEDIA_KEYS_H

#include <QObject>
#include <QString>

// Hooks into the macOS "Now Playing" system (MPRemoteCommandCenter), which is
// how keyboard media keys (F7/F8/F9), headphone buttons and the Control Center
// player reach an app. macOS only routes them to the app that most recently
// reported itself as playing, hence setPlaybackState()/setNowPlaying().
class NMacMediaKeys : public QObject
{
    Q_OBJECT

private:
    NMacMediaKeys() {}
    ~NMacMediaKeys() {}
    NMacMediaKeys(NMacMediaKeys const &copy);
    NMacMediaKeys operator=(NMacMediaKeys const &copy);

public:
    enum State { Stopped, Playing, Paused };

    static NMacMediaKeys *instance();
    void registerHandlers();
    void setPlaybackState(State state);
    void setNowPlaying(const QString &title, const QString &artist, qint64 durationMsec);

    void _emitPlay() { emit playRequested(); }
    void _emitPause() { emit pauseRequested(); }
    void _emitPlayPause() { emit playPauseRequested(); }
    void _emitStop() { emit stopRequested(); }
    void _emitNext() { emit nextRequested(); }
    void _emitPrev() { emit prevRequested(); }

signals:
    void playRequested();
    void pauseRequested();
    void playPauseRequested();
    void stopRequested();
    void nextRequested();
    void prevRequested();
};

#endif
