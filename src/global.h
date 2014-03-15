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

#ifndef N_GLOBAL_H
#define N_GLOBAL_H

#include <QtCore>

template <typename T>
class NFlagIterator
{
public:
	NFlagIterator(unsigned flags) : mFlags(flags), mFlag(0) {}
	inline T value() { return static_cast<T>(mFlag); }
	inline bool hasNext() { return mFlags > mFlag; }
	void next() { if (mFlag == 0) mFlag = 1; else mFlag <<= 1;
	              while ((mFlags & mFlag) == 0) mFlag <<= 1;
	              mFlags &= ~mFlag; }
private:
	unsigned mFlags;
	unsigned mFlag;
};

#define ENUM_NAME(c,e,v) (c::staticMetaObject.enumerator(c::staticMetaObject.indexOfEnumerator(#e)).valueToKey((v)))

#ifndef Q_MOC_RUN
namespace N
#else
class N
#endif
{
#if defined(Q_MOC_RUN)
	Q_GADGET
	Q_ENUMS(PlaybackState)
	Q_ENUMS(PluginType)
public:
#endif

	enum PlaylistRole {
		FailedRole = Qt::UserRole + 1,
		PathRole,
		DurationRole,
		CountRole,
		PositionRole
	};

	enum PlaybackState {
		PlaybackStopped,
		PlaybackPlaying,
		PlaybackPaused
	};

	enum M3uExtention {
		MinimalM3u = 0,
		ExtM3u     = 1,
		NulloyM3u  = 2
	};

	enum PluginType {
		OtherPlugin     = 0,
		PlaybackEngine  = (1<<0),
		WaveformBuilder = (1<<1),
		TagReader       = (1<<2),
		CoverReader     = (1<<3),
		MaxPlugin       = (1<<4) - 1
	};

	Q_DECLARE_FLAGS(PluginTypeFlags, PluginType)
	Q_DECLARE_OPERATORS_FOR_FLAGS(PluginTypeFlags)

	extern const QMetaObject staticMetaObject;
};

#endif

