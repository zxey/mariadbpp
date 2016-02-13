//
//  M A R I A D B + +
//
//	Author   : Sylvain Rochette Langlois
//	License  : Boost Software License (http://www.boost.org/users/license.html)
//

#include <mysql.h>
#include <sstream>
#include <mariadb++/exceptions.hpp>
#include <mariadb++/date_time.hpp>
#include <mariadb++/time.hpp>
#include <mariadb++/conversion_helper.hpp>
#include "private.hpp"

using namespace std;

#define MS_PER_SEC 1000
#define MS_PER_MIN (MS_PER_SEC * 60)
#define MS_PER_HOUR (MS_PER_MIN * 60)
#define MS_PER_DAY (MS_PER_HOUR * 24)

#define THROW_EXCEPTION(_hour, _minute, _second, _millisecond)\
{\
	if (g_log_error)\
		std::cerr << "MariaDB Invalid time: hour - " << _hour << ", minute - " << _minute << ", second - " << _second\
				<< ", millisecond - " << _millisecond << "\nIn function: " << __FUNCTION__ << '\n';\
	MARIADB_ERROR_THROW_TIME(_hour, _minute, _second, _millisecond)\
	return false;\
}

//
// Constructors
//
mariadb::time::time(u8 hour, u8 minute, u8 second, u16 millisecond)
{
	set(hour, minute, second, millisecond);
}

mariadb::time::time(const time& t)
{
	set(t.hour(), t.minute(), t.second(), t.millisecond());
}

mariadb::time::time(const tm& t)
{
	set(t.tm_hour, t.tm_min, t.tm_sec, 0);
}

mariadb::time::time(const time_t& t)
{
	tm ts;
	localtime_safe(&ts, &t);

	set(ts.tm_hour, ts.tm_min, ts.tm_sec, 0);
}

mariadb::time::time(const MYSQL_TIME& t)
{
	set(t.hour, t.minute, t.second, 0);
}

mariadb::time::time(const std::string& t)
{
	set(t);
}

//
// Operators
//
int mariadb::time::compare(const time& t) const
{
	if (hour() < t.hour())
		return -1;

	if (hour() > t.hour())
		return 1;

	if (minute() < t.minute())
		return -1;

	if (minute() > t.minute())
		return 1;

	if (second() < t.second())
		return -1;

	if (second() > t.second())
		return 1;

	if (millisecond() < t.millisecond())
		return -1;

	return millisecond() == t.millisecond() ? 0 : 1;
}

mariadb::time& mariadb::time::operator = (const time& t)
{
	set(t.hour(), t.minute(), t.second(), t.millisecond());
	return *this;
}

bool mariadb::time::operator == (const time& t) const
{
	return compare(t) == 0;
}

bool mariadb::time::operator != (const time& t) const
{
	return compare(t) != 0;
}

bool mariadb::time::operator < (const time& t) const
{
	return compare(t) < 0;
}

bool mariadb::time::operator <= (const time& t) const
{
	return compare(t) <= 0;
}

bool mariadb::time::operator > (const time& t) const
{
	return compare(t) > 0;
}

bool mariadb::time::operator >= (const time& t) const
{
	return compare(t) >= 0;
}

bool mariadb::time::set(u8 hour, u8 minute, u8 second, u16 millisecond)
{
	if (hour >= 24 || minute >= 60 || second >= 60 || millisecond >= 1000)
		THROW_EXCEPTION(hour, minute, second, millisecond);

	m_hour = hour;
	m_minute = minute;
	m_second = second;
	m_millisecond = millisecond;
	return true;
}

mariadb::u8 mariadb::time::hour() const
{
	return m_hour;
}

mariadb::u8 mariadb::time::hour(u8 hour)
{
	if (hour >= 24)
		THROW_EXCEPTION(hour, minute(), second(), millisecond());

	m_hour = hour;

	return m_hour;
}

mariadb::u8 mariadb::time::minute() const
{
	return m_minute;
}

mariadb::u8 mariadb::time::minute(u8 minute)
{
	if (minute >= 60)
		THROW_EXCEPTION(hour(), minute, second(), millisecond());

	m_minute = minute;

	return m_minute;
}

mariadb::u8 mariadb::time::second() const
{
	return m_second;
}

mariadb::u8 mariadb::time::second(u8 second)
{
	if (second >= 60)
		THROW_EXCEPTION(hour(), minute(), second, millisecond());

	m_second = second;

	return m_second;
}

mariadb::u16 mariadb::time::millisecond() const
{
	return m_millisecond;
}

mariadb::u16 mariadb::time::millisecond(u16 millisecond)
{
	if (millisecond >= 1000)
		THROW_EXCEPTION(hour(), minute(), second(), millisecond);

	m_millisecond = millisecond;

	return m_millisecond;
}

mariadb::time mariadb::time::add_hours(s32 hours) const
{
	time tmp = *this;

	if (!hours)
		return tmp;

	s32 days = hours / 24;

	hours = (hours % 24) + hour();

	if (hours >= 24)
	{
		++days;
		hours %= 24;
	}
	else if (hours < 0)
	{
		--days;
		hours = 24 - hours;
	}

	tmp.hour(hours);
	return tmp;
}

mariadb::time mariadb::time::add_minutes(s32 minutes) const
{
	time tmp = *this;

	if (!minutes)
		return tmp;

	s32 hours = minutes / 60;

	minutes = (minutes % 60) + minute();

	if (minutes >= 60)
	{
		++hours;
		minutes %= 60;
	}
	else if (minutes < 0)
	{
		--hours;
		minutes = 60 - minutes;
	}

	if (hours > 0)
		tmp = tmp.add_hours(hours);

	tmp.minute(minutes);
	return tmp;
}

mariadb::time mariadb::time::add_seconds(s32 seconds) const
{
	time tmp = *this;

	if (!seconds)
		return tmp;

	s32 minutes = seconds / 60;

	seconds = (seconds % 60) + second();

	if (seconds >= 60)
	{
		++minutes;
		seconds %= 60;
	}
	else if (seconds < 0)
	{
		--minutes;
		seconds = 60 - seconds;
	}

	if (minutes > 0)
		tmp = tmp.add_minutes(minutes);

	tmp.second(seconds);
	return tmp;
}

mariadb::time mariadb::time::add_milliseconds(s32 milliseconds) const
{
	time tmp = *this;

	if (!milliseconds)
		return tmp;

	s32 seconds = milliseconds / 1000;

	milliseconds = (milliseconds % 1000) + millisecond();

	if (milliseconds >= 1000)
	{
		++seconds;
		milliseconds %= 1000;
	}
	else if (milliseconds < 0)
	{
		--seconds;
		milliseconds = 1000 - milliseconds;
	}

	if (seconds > 0)
		tmp = tmp.add_seconds(seconds);

	tmp.millisecond(milliseconds);
	return tmp;
}

mariadb::time mariadb::time::subtract(const time_span& dur) const
{
	bool negative = !dur.negative();

	time_span tmp = dur;
	tmp.negative(negative);
	return add(tmp);
}

mariadb::time mariadb::time::add(const time_span& dur) const
{
	s32 negate = dur.negative() ? -1 : 1;
	time tmp = *this;

	tmp.add_hours(negate * dur.hours());
	tmp.add_minutes(negate * dur.minutes());
	tmp.add_seconds(negate * dur.seconds());
	tmp.add_milliseconds(negate * dur.milliseconds());
	return tmp;
}

mariadb::time_span mariadb::time::time_between(const time& t) const
{
	if (t == *this)
		return time_span(0, 0, 0, 0, 0);

	if (t > *this)
	{
		time_span dur = t.time_between(*this);
		dur.negative(true);
		return dur;
	}

	bool negative = false;
	s64 ms = (hour() * MS_PER_HOUR) + (minute() * MS_PER_MIN) + (second() * MS_PER_SEC) + millisecond();
	s64 t_ms = (t.hour() * MS_PER_HOUR) + (t.minute() * MS_PER_MIN) + (t.second() * MS_PER_SEC) + t.millisecond();
	s64 total_ms = 0;

	//
	// Need to stop one day after in this case
	//
	if (t_ms > ms)
		total_ms = MS_PER_DAY - (t_ms - ms);
	else
		total_ms = ms - t_ms;

	u32 hours = static_cast<u32>(total_ms / MS_PER_HOUR);
	total_ms = total_ms % MS_PER_HOUR;

	u32 minutes = static_cast<u32>(total_ms / MS_PER_MIN);
	total_ms = total_ms % MS_PER_MIN;

	u32 seconds = static_cast<u32>(total_ms / MS_PER_SEC);
	total_ms = total_ms % MS_PER_SEC;

	return time_span(hours, minutes, seconds, static_cast<u32>(total_ms), negative);
}

time_t mariadb::time::mktime() const
{
	tm time_struct;

	time_struct.tm_year = 1900;
	time_struct.tm_mon = 0;
	time_struct.tm_mday = 1;
	time_struct.tm_hour = hour();
	time_struct.tm_min = minute();
	time_struct.tm_sec = second();

	return ::mktime(&time_struct);
}

MYSQL_TIME mariadb::time::mysql_time() const
{
	MYSQL_TIME t;

	t.year = 0;
	t.month = 0;
	t.day = 0;
	t.hour = hour();
	t.minute = minute();
	t.second = second();
	t.neg = false;
	t.second_part = 0;
	t.time_type = MYSQL_TIMESTAMP_TIME;
	return t;
}

double mariadb::time::diff_time(const time& t) const
{
	time_t time_val = mktime();
	time_t t_time_val = t.mktime();

	return ::difftime(time_val, t_time_val);
}

mariadb::time mariadb::time::now()
{
	time_t local_time = ::time(NULL);
	tm ts;
	localtime_safe(&ts, &local_time);

	return time(ts);
}

mariadb::time mariadb::time::now_utc()
{
	time_t utc_time = ::time(NULL);
	tm ts;
	gmtime_safe(&ts, &utc_time);

	return time(ts);
}

bool mariadb::time::set(const std::string& t)
{
	if (t.empty() ||
		t.length() < 2)
		return false;

	u8 h = string_cast<u8>(t.substr(0, 2).c_str());
	u8 min = 0;
	double s = 0;

	if (t.length() >= 3)
	{
		min = string_cast<u8>(t.substr(3, 2).c_str());

		if (t.length() >= 6)
			s = string_cast<double>(t.substr(6).c_str());
	}

	u16 ms = static_cast<u16>((s - static_cast<u8>(s)) * 1000);

	return set(h, min, static_cast<u8>(s), ms);
}

const std::string mariadb::time::str_time(bool with_millisecond) const
{
    char buffer[14];

    if (with_millisecond && millisecond())
		snprintf(buffer, sizeof(buffer), "%02i:%02i:%02i.%03i", hour(), minute(), second(), millisecond());
	else
		snprintf(buffer, sizeof(buffer), "%02i:%02i:%02i", hour(), minute(), second());
	return std::string(buffer);
}

ostream& mariadb::operator << (ostream& os, const time& t)
{
	os << t.str_time();
	return os;
}
