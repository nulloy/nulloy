/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2013 Sergey Vlasov <sergey@vlasov.me>
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

#include "tagReaderTaglib.h"

#include <QCoreApplication>
#include <QFileInfo>

void NTagReaderTaglib::init()
{
	if (m_init)
		return;

	m_init = TRUE;
	m_tagRef = NULL;
}

void NTagReaderTaglib::setSource(const QString &file)
{
	m_path = file;
	m_tagRef = new TagLib::FileRef(file.toUtf8().data());
}

NTagReaderTaglib::~NTagReaderTaglib()
{
	if (!m_init)
		return;

	if (m_tagRef)
		delete m_tagRef;
}

QString NTagReaderTaglib::toString(const QString &format)
{
	bool res;
	return parse(format, &res);
}

QString NTagReaderTaglib::parse(const QString &format, bool *success, bool stopOnFail)
{
	if (format.isEmpty())
		return "";

	*success = TRUE;

	if (!m_tagRef->file()->isValid())
		return "NTagReaderTaglib::InvalidFile";

	TagLib::Tag *tag = m_tagRef->tag();
	TagLib::AudioProperties *prop = m_tagRef->audioProperties();

	int seconds_total = prop->length();

	QString res;
	for (int i = 0; i < format.size(); ++i) {
		if (format.at(i) == '%') {
			++i;
			QChar ch = format.at(i);
			if (ch == 'a') {
				QString str = TStringToQString(tag->artist());
				if (!(*success = !str.isEmpty()))
					str = "<Unknown artist>";
				res += str;
			} else if (ch == 't') {
				QString str = TStringToQString(tag->title());
				if (!(*success = !str.isEmpty()))
					str = "<Unknown title>";
				res += str;
			} else if (ch == 'A') {
				QString str = TStringToQString(tag->album());
				if (!(*success = !str.isEmpty()))
					str = "<Unknown album>";
				res += str;
			} else if (ch == 'c') {
				QString str = TStringToQString(tag->comment());
				if (!(*success = !str.isEmpty()))
					str = "<Empty comment>";
				res += str;
			} else if (ch == 'g') {
				QString str = TStringToQString(tag->genre());
				if (!(*success = !str.isEmpty()))
					str = "<Unknown genre>";
				res += str;
			} else if (ch == 'y') {
				QString str = QString::number(tag->year());
				if (str == "0") {
					str = "<Unknown year>";
					*success = FALSE;
				}
				res += str;
			} else if (ch == 'n') {
				QString str = QString::number(tag->track());
				if (str == "0") {
					str = "<Unknown track number>";
					*success = FALSE;
				}
				res += str;
			} else if (ch == 'd') {
				QString duration;
				if (seconds_total > 0) {
					int seconds = seconds_total % 60;
					int minutes = (seconds_total - seconds) / 60;
					int hours = minutes / 60;
					minutes = minutes % 60;
					if (hours > 0)
						duration.sprintf("%d:%02d:%02d", hours, minutes, seconds);
					else
						duration.sprintf("%d:%02d", minutes, seconds);
				} else {
					duration = "<Unknown duration>";
					*success = FALSE;
				}
				res += duration;
			} else if (ch == 'D') {
				QString duration;
				if (seconds_total == 0) {
					duration = "<Unknown duration>";
					*success = FALSE;
				} else {
					duration = QString::number(seconds_total);
				}
				res += duration;
			} else if (ch == 'B') {
				QString str = QString::number(prop->bitrate() / 1000);
				if (str == "0") {
					str = "<Unknown bitrate>";
					*success = FALSE;
				}
				res += str;
			} else if (ch == 's') {
				QString str = QString::number(prop->sampleRate());
				if (str == "0") {
					str = "<Unknown sample rate>";
					*success = FALSE;
				}
				res += str;
			} else if (ch == 'C') {
				QString str = QString::number(prop->channels());
				if (str == "0") {
					str = "<Unknown channels number>";
					*success = FALSE;
				}
				res += str;
			} else if (ch == 'f') {
				res += QFileInfo(m_path).baseName();
			} else if (ch == 'F') {
				res += QFileInfo(m_path).fileName();
			} else if (ch == 'p') {
				res += QFileInfo(m_path).absoluteFilePath();
			} else if (ch == 'v') {
				res += QCoreApplication::applicationVersion();
			} else {
				res += ch;
			}
		} else if (format.at(i) == '{') {
			++i;
			int matchedAt = format.indexOf('}', i);
			if (matchedAt == -1) {
				res += "<condition error: unmatched '{'>";
				return res;
			}

			QString condition = format.mid(i, matchedAt - 1);

			if (condition.indexOf('{') != -1) {
				res += "<condition error: extra '{'>";
				return res;
			}

			QStringList values = condition.split('|');
			if (values.count() < 2) {
				res += "<condition error: missing '|'>";
				return res;
			} else if (values.count() > 2) {
				res += "<condition error: extra '|'>";
				return res;
			}

			bool cond_res;
			QString cond_true = parse(values.at(0), &cond_res, TRUE);
			if (cond_res) {
				res += cond_true;
			} else {
				res += parse(values.at(1), &cond_res);
			}
			i = matchedAt;
		} else {
			res += format.at(i);
		}
		if (!*success && stopOnFail)
			return "";
	}

	return res;
}

bool NTagReaderTaglib::isValid()
{
	return (m_tagRef && m_tagRef->file() && m_tagRef->file()->isValid());
}

/* vim: set ts=4 sw=4: */
