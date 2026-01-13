/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief HTTP transport statistics aggregator
*/
class MetricZ_HttpStats
{
	// Stats
	protected static int s_TotalRetries; //!< Total number of retries
	protected static int s_TotalBytes; //!< Total number of bytes sent

	// Caches
	protected static ref map<string, int> s_Lookup = new map<string, int>(); //!< map `type:status` -> index in arrays
	protected static ref array<string> s_CacheLabels = new array<string>(); //!< label cache
	protected static ref array<int> s_Counts = new array<int>(); //!< actual counters matched by index

	// Metrics
	protected static ref MetricZ_MetricInt s_MetricRequests = new MetricZ_MetricInt(
	    "http_requests",
	    "Total HTTP requests by type and status",
	    MetricZ_MetricType.COUNTER);
	protected static ref MetricZ_MetricInt s_MetricRetries = new MetricZ_MetricInt(
	    "http_retries",
	    "Total HTTP callback retries",
	    MetricZ_MetricType.COUNTER);
	protected static ref MetricZ_MetricInt s_MetricBytes = new MetricZ_MetricInt(
	    "http_sent_bytes",
	    "Total bytes sent via HTTP body",
	    MetricZ_MetricType.COUNTER);

	/**
	    \brief Increment request counter.
	    \details Handles full label construction once per new type/status pair.
	    \param type Request type
	    \param status Request status
	*/
	static void IncRequest(string type, string status)
	{
		if (!MetricZ_Config.IsLoaded() || MetricZ_Config.Get().disabled_metrics.http)
			return;

		string key = string.Format("%1:%2", type, status);
		int idx;

		if (s_Lookup.Find(key, idx)) {
			s_Counts.Set(idx, s_Counts.Get(idx) + 1);
			return;
		}

		map<string, string> labelsMap = new map<string, string>();
		labelsMap.Insert("type", type);
		labelsMap.Insert("status", status);
		string labels = MetricZ_LabelUtils.MakeLabels(labelsMap);

		s_Lookup.Insert(key, s_Counts.Count());
		s_CacheLabels.Insert(labels);
		s_Counts.Insert(1);
	}

	/**
	    \brief Increment retry counter.
	*/
	static void IncRetry()
	{
		s_TotalRetries++;
	}

	/**
	    \brief Add to total sent bytes.
	    \param bytes Number of bytes to add
	*/
	static void AddBytes(int bytes)
	{
		if (bytes < 0)
			return;

		if (s_TotalBytes + bytes < 0)
			s_TotalBytes = 0;

		s_TotalBytes += bytes;
	}

	/**
	    \brief Emit metrics to sink.
	*/
	static void Flush(MetricZ_SinkBase sink)
	{
		if (!sink || !MetricZ_Config.IsLoaded())
			return;

		s_MetricRetries.Set(s_TotalRetries);
		s_MetricRetries.FlushWithHead(sink);

		s_MetricBytes.Set(s_TotalBytes);
		s_MetricBytes.FlushWithHead(sink);

		int count = s_Counts.Count();
		if (count > 0) {
			s_MetricRequests.WriteHeaders(sink);

			for (int i = 0; i < count; ++i) {
				s_MetricRequests.Set(s_Counts.Get(i));
				s_MetricRequests.Flush(sink, s_CacheLabels.Get(i));
			}
		}
	}
}
#endif
