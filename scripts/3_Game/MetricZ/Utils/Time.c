/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Time helpers for epoch computations.
    \details Provides UTC epoch and in-game epoch seconds without relying on system time.
*/
class MetricZ_Time
{
	// Days per month in non-leap year
	static const int MDAYS[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	/**
	    \brief Count leap years up to year-1 (Gregorian).
	    \param year Target year
	    \return \p int Number of leap years
	*/
	static int LeapYearsUpTo(int year)
	{
		int y = year - 1;
		return (y / 4) - (y / 100) + (y / 400);
	}

	/**
	    \brief Unix epoch seconds derived from in-game world date.
	    \details Seconds are synthesized as API does not expose them.
	    \return \p float Epoch seconds
	*/
	static float GameEpochSeconds()
	{
		int y, m, d, hh, mm;
		GetGame().GetWorld().GetDate(y, m, d, hh, mm);

		int years = y - 1970;
		int leaps = LeapYearsUpTo(y) - LeapYearsUpTo(1970);
		int days_y = years * 365 + leaps;

		int days_m = 0;
		for (int i = 1; i < m; i++)
			days_m += MDAYS[i - 1];

		bool isLeap = ((y % 4 == 0) && (y % 100 != 0)) || (y % 400 == 0);
		if (isLeap && m > 2)
			days_m += 1;

		int days_total = days_y + days_m + (d - 1);
		return days_total * 86400.0 + hh * 3600.0 + mm * 60.0; // no seconds in API
	}

	/**
	    \brief Unix epoch seconds in UTC.
	    \return \p float Epoch seconds
	*/
	static float EpochSecondsUTC()
	{
		int y, m, d;
		int hh, mm, ss;
		GetYearMonthDayUTC(y, m, d);
		GetHourMinuteSecondUTC(hh, mm, ss);

		// days since 1970-01-01
		int years = y - 1970;
		int leaps = LeapYearsUpTo(y) - LeapYearsUpTo(1970);
		int days_years = years * 365 + leaps;

		int days_months = 0;
		for (int i = 1; i < m; i++)
			days_months += MDAYS[i - 1];

		// add Feb 29 for leap years if past Feb
		bool isLeap = ((y % 4 == 0) && (y % 100 != 0)) || (y % 400 == 0);
		if (isLeap && m > 2)
			days_months += 1;

		int days_total = days_years + days_months + (d - 1);

		float secs = days_total * 86400.0 + hh * 3600.0 + mm * 60.0 + ss;
		return secs; // float avoids 2038 int overflow
	}
}
#endif
