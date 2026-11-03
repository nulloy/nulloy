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

#import <Foundation/Foundation.h>
#import <MediaPlayer/MediaPlayer.h>

#include "macMediaKeys.h"

static NMacMediaKeys *_instance = NULL;

NMacMediaKeys *NMacMediaKeys::instance()
{
    if (!_instance)
        _instance = new NMacMediaKeys();
    return _instance;
}

static void _addHandler(MPRemoteCommand *command, void (NMacMediaKeys::*emitter)())
{
    command.enabled = YES;
    [command addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *) {
        (NMacMediaKeys::instance()->*emitter)();
        return MPRemoteCommandHandlerStatusSuccess;
    }];
}

void NMacMediaKeys::registerHandlers()
{
    if (@available(macOS 10.12.2, *)) {
        MPRemoteCommandCenter *center = [MPRemoteCommandCenter sharedCommandCenter];
        _addHandler(center.playCommand, &NMacMediaKeys::_emitPlay);
        _addHandler(center.pauseCommand, &NMacMediaKeys::_emitPause);
        _addHandler(center.togglePlayPauseCommand, &NMacMediaKeys::_emitPlayPause);
        _addHandler(center.stopCommand, &NMacMediaKeys::_emitStop);
        _addHandler(center.nextTrackCommand, &NMacMediaKeys::_emitNext);
        _addHandler(center.previousTrackCommand, &NMacMediaKeys::_emitPrev);
    }
}

void NMacMediaKeys::setPlaybackState(State state)
{
    if (@available(macOS 10.12.2, *)) {
        MPNowPlayingPlaybackState native = MPNowPlayingPlaybackStateStopped;
        if (state == Playing)
            native = MPNowPlayingPlaybackStatePlaying;
        else if (state == Paused)
            native = MPNowPlayingPlaybackStatePaused;
        [MPNowPlayingInfoCenter defaultCenter].playbackState = native;
    }
}

void NMacMediaKeys::setNowPlaying(const QString &title, const QString &artist,
                                  qint64 durationMsec)
{
    if (@available(macOS 10.12.2, *)) {
        NSMutableDictionary *info = [NSMutableDictionary dictionary];
        info[MPMediaItemPropertyTitle] = title.toNSString();
        if (!artist.isEmpty())
            info[MPMediaItemPropertyArtist] = artist.toNSString();
        if (durationMsec > 0)
            info[MPMediaItemPropertyPlaybackDuration] = @(durationMsec / 1000.0);
        [MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo = info;
    }
}
