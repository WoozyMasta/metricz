/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
class MetricZ_FileSink : MetricZ_SinkBase
{
	private FileHandle m_Fh;

	override bool Begin()
	{
		if (!MetricZ_Config.IsLoaded())
			return false;

		if (!super.Begin())
			return false;

		string file;
		if (MetricZ_Config.Get().atomicFileExport)
			file = MetricZ_Config.METRICS_TEMP;
		else
			file = MetricZ_Config.METRICS_FILE;

		m_Fh = OpenFile(file, FileMode.WRITE);
		if (!m_Fh) {
			ErrorEx("MetricZ: open file '" + file + "' failed", ErrorExSeverity.ERROR);
			return false;
		}

		return true;
	}

	override void Line(string line)
	{
		if (!IsBusy() || !m_Fh)
			return;

		if (IsBuffered())
			super.Line(line);
		else
			FPrintln(m_Fh, line);
	}

	override bool End()
	{
		if (!MetricZ_Config.IsLoaded())
			return false;

		if (!super.End())
			return false;

		if (m_Fh) {
			CloseFile(m_Fh);
			m_Fh = null;
		}

		if (MetricZ_Config.Get().atomicFileExport) {
			DeleteFile(MetricZ_Config.METRICS_FILE);

			if (!CopyFile(MetricZ_Config.METRICS_TEMP, MetricZ_Config.METRICS_FILE)) {
				ErrorEx(
				    "MetricZ: atomic publish failed '" + MetricZ_Config.METRICS_TEMP + "' -> '" + MetricZ_Config.METRICS_FILE + "'",
				    ErrorExSeverity.ERROR);
				return false;
			}

			DeleteFile(MetricZ_Config.METRICS_TEMP);
		}

		return true;
	}

	override protected void BufferFlush()
	{
		if (m_Fh && GetBufferCount() > 0)
			FPrint(m_Fh, GetBufferChunk());

		super.BufferFlush();
	}
}
#endif
