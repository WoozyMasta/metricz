/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Lightweight 1-second FPS sampler.
    \details Accumulates frame count and elapsed time via MissionServer::OnUpdate(timeslice)
             and computes average FPS once per ~1s window.
*/
class MetricZ_FrameMonitor
{
	static float s_FPS; //!< Last computed average FPS over ~1s window
	static float s_AccTime; //!< Accumulated time in seconds within the current window
	static int s_AccFrames; //!< Accumulated frames within the current window

	// window stats between scrapes
	static float s_WindowMin;
	static float s_WindowMax;
	static float s_WindowSum;
	static int s_WindowCount;
	static bool s_WindowHasSample;

	/**
	    \brief Feed per-frame timing and update the rolling 1s FPS.
	    \param timeslice Delta time of the last frame in seconds.
	    \note Resets the window once accumulated time reaches or exceeds 1.0s.
	*/
	static void OnUpdate(float timeslice)
	{
		s_AccTime += timeslice;
		s_AccFrames++;

		if (s_AccTime >= 1.0) {
			if (s_AccTime > 0)
				s_FPS = s_AccFrames / s_AccTime;

			RecordWindowSample(s_FPS);

			s_AccTime = s_AccTime - 1.0;
			s_AccFrames = 0;

			if (s_AccTime > 1.0)
				s_AccTime = 0;
		}
	}

	/**
	    \brief Record 1-second FPS sample into window stats.
	*/
	protected static void RecordWindowSample(float fps)
	{
		if (!s_WindowHasSample) {
			s_WindowMin = fps;
			s_WindowMax = fps;
			s_WindowHasSample = true;
		} else {
			if (fps < s_WindowMin)
				s_WindowMin = fps;
			if (fps > s_WindowMax)
				s_WindowMax = fps;
		}

		s_WindowSum += fps;
		s_WindowCount++;
	}

	/**
	    \brief Snapshot and reset window stats.
	    \param[out] min Min FPS in window or 0
	    \param[out] max Max FPS in window or 0
	    \param[out] avg Average FPS in window or 0
	    \param[out] samples Number of 1s samples in window
	*/
	static void SnapshotWindow(out float min, out float max, out float avg, out int samples)
	{
		if (!s_WindowHasSample || s_WindowCount == 0) {
			min = s_FPS;
			max = s_FPS;
			avg = s_FPS;
			samples = 1;
		} else {
			min = s_WindowMin;
			max = s_WindowMax;
			avg = s_WindowSum / s_WindowCount;
			samples = s_WindowCount;
		}

		// reset window
		s_WindowMin = 0;
		s_WindowMax = 0;
		s_WindowSum = 0;
		s_WindowCount = 0;
		s_WindowHasSample = false;
	}

	/**
	    \brief Return the last computed 1s average FPS.
	    \return Average FPS sampled over the last completed ~1s window; 0 if not computed yet.
	*/
	static float GetFPS()
	{
		return s_FPS;
	}
}
#endif
