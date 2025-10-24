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
	static float s_AccTime; //!< Accumulated time in seconds within the current window
	static int s_AccFrames; //!< Accumulated frames within the current window
	static float s_FPS; //!< Last computed average FPS over ~1s window

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
			s_AccTime = 0;
			s_AccFrames = 0;
		}
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
