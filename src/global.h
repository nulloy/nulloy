/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2015 Sergey Vlasov <sergey@vlasov.me>
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
#include <QPainter>

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

#define ENUM_TO_STR(c,e,v) (c::staticMetaObject.enumerator(c::staticMetaObject.indexOfEnumerator(#e)).valueToKey(v))
#define STR_TO_ENUM(c,e,k) (c::staticMetaObject.enumerator(c::staticMetaObject.indexOfEnumerator(#e)).keyToValue(k))

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
	Q_ENUMS(CompositionMode)
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

	enum CompositionMode {
		SourceOver      = QPainter::CompositionMode_SourceOver,
		DestinationOver = QPainter::CompositionMode_DestinationOver,
		Clear           = QPainter::CompositionMode_Clear,
		Source          = QPainter::CompositionMode_Source,
		Destination     = QPainter::CompositionMode_Destination,
		SourceIn        = QPainter::CompositionMode_SourceIn,
		DestinationIn   = QPainter::CompositionMode_DestinationIn,
		SourceOut       = QPainter::CompositionMode_SourceOut,
		DestinationOut  = QPainter::CompositionMode_DestinationOut,
		SourceAtop      = QPainter::CompositionMode_SourceAtop,
		DestinationAtop = QPainter::CompositionMode_DestinationAtop,
		Xor             = QPainter::CompositionMode_Xor,
		Plus            = QPainter::CompositionMode_Plus,
		Multiply        = QPainter::CompositionMode_Multiply,
		Screen          = QPainter::CompositionMode_Screen,
		Overlay         = QPainter::CompositionMode_Overlay,
		Darken          = QPainter::CompositionMode_Darken,
		Lighten         = QPainter::CompositionMode_Lighten,
		ColorDodge      = QPainter::CompositionMode_ColorDodge,
		ColorBurn       = QPainter::CompositionMode_ColorBurn,
		HardLight       = QPainter::CompositionMode_HardLight,
		SoftLight       = QPainter::CompositionMode_SoftLight,
		Difference      = QPainter::CompositionMode_Difference,
		Exclusion       = QPainter::CompositionMode_Exclusion,
	};

	extern const QMetaObject staticMetaObject;
};

#endif

