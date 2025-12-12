/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Solver for create sink
*/
class MetricZ_Sink
{
	/**
	    \brief Create new sink based on configuration
	*/
	static MetricZ_SinkBase New()
	{
		if (!MetricZ_Config.IsLoaded())
			return null;

		MetricZ_ConfigDTO cfg = MetricZ_Config.Get();
		if (!cfg)
			return null;

		MetricZ_SinkBase sink;

		if (cfg.file.enabled && cfg.http.enabled) {
			MetricZ_CompositeSink composite = new MetricZ_CompositeSink();
			if (!composite)
				return null;

			ErrorEx("MetricZ: do not use both exports in production (file.enabled=1, http.enabled=1)", ErrorExSeverity.WARNING);

			composite.AddSink(new MetricZ_RestSink(), cfg.http.buffer);
			composite.AddSink(new MetricZ_FileSink(), cfg.file.buffer);
			return composite;
		}

		if (cfg.http.enabled) {
			sink = new MetricZ_RestSink();
			if (!sink)
				return null;

			sink.SetBuffer(cfg.http.buffer);
			return sink;
		}

		if (cfg.file.enabled) {
			sink = new MetricZ_FileSink();
			if (!sink)
				return null;

			sink.SetBuffer(cfg.file.buffer);
			return sink;
		}

		ErrorEx("MetricZ: no exports enabled in config (file.enabled=0, http.enabled=0)", ErrorExSeverity.WARNING);
		return null;
	}
}
#endif
