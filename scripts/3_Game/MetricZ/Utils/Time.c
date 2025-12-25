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
	// Unix epoch start year
	static const int EPOCH_START_YEAR = 1970;
	// Inclusive max year for 32-bit int seconds (1970-01-01..2037-12-31 safety)
	static const int EPOCH_MAX_YEAR = 2037;
	// Size of year window
	static const int EPOCH_YEAR_SPAN = EPOCH_MAX_YEAR - EPOCH_START_YEAR + 1;

	// Days per month in non-leap year
	static const int MDAYS[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	/**
	    \brief Unix epoch seconds derived from in-game world date.
	    \return int Epoch seconds (world time, minute precision).
	*/
	static int GameEpochSeconds()
	{
		int y, m, d, hh, mm;
		g_Game.GetWorld().GetDate(y, m, d, hh, mm);

		return EpochSecondsForDate(y, m, d, hh, mm, 0);
	}

	/**
	    \brief Unix epoch seconds in UTC.
	    \return int Epoch seconds (wall-clock UTC).
	*/
	static int EpochSecondsUTC()
	{
		int y, m, d;
		int hh, mm, ss;
		GetYearMonthDayUTC(y, m, d);
		GetHourMinuteSecondUTC(hh, mm, ss);

		return EpochSecondsForDate(y, m, d, hh, mm, ss);
	}

	/**
	    \brief Count leap years up to year-1 (Gregorian).
	    \param year Target year
	    \return int Number of leap years
	*/
	private static int LeapYearsUpTo(int year)
	{
		int y = year - 1;
		return (y / 4) - (y / 100) + (y / 400);
	}

	/**
	    \brief Normalize year into safe epoch window [EPOCH_START_YEAR;EPOCH_MAX_YEAR].
	    \details
	        - If year already in window -> returned as-is.
	        - Otherwise year is shifted
	*/
	private static int NormalizeYearForEpoch(int year)
	{
		if (year >= EPOCH_START_YEAR && year <= EPOCH_MAX_YEAR)
			return year;

		// shift into [EPOCH_START_YEAR; EPOCH_START_YEAR+EPOCH_YEAR_SPAN-1] with wrap-around
		int norm = year - ((year - EPOCH_START_YEAR) / EPOCH_YEAR_SPAN) * EPOCH_YEAR_SPAN;

		// fix possible off-by-EPOCH_YEAR_SPAN due to division truncation for negative values
		while (norm < EPOCH_START_YEAR)
			norm += EPOCH_YEAR_SPAN;

		while (norm > EPOCH_MAX_YEAR)
			norm -= EPOCH_YEAR_SPAN;

		return norm;
	}

	/**
	    \brief Unix epoch seconds for given Y-M-D hh:mm:ss.
	*/
	private static int EpochSecondsForDate(int y, int m, int d, int hh, int mm, int ss)
	{
		y = NormalizeYearForEpoch(y);

		int years = y - EPOCH_START_YEAR;
		int leaps = LeapYearsUpTo(y) - LeapYearsUpTo(EPOCH_START_YEAR);
		int days_years = years * 365 + leaps;

		int days_months = 0;
		for (int i = 1; i < m; ++i)
			days_months += MDAYS[i - 1];

		bool isLeap = ((y % 4 == 0) && (y % 100 != 0)) || (y % 400 == 0);
		if (isLeap && m > 2)
			days_months += 1;

		int days_total = days_years + days_months + (d - 1);

		return days_total * 86400 + hh * 3600 + mm * 60 + ss;
	}
}
#endif
