/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Sink implementation for local file export.
    \details Writes metrics to a .prom file (compatible with node-exporter textfile collector).
             Supports atomic writes (write to .tmp -> copy to .prom) to prevent partial reads.
*/
class MetricZ_FileSink : MetricZ_SinkBase
{
	private FileHandle m_Fh; //!< Native file handle for the active write operation

	/**
	    \brief Opens the target file for writing.
	    \details Selects temporary path if atomic mode is enabled, otherwise uses direct path.
	*/
	override bool Begin()
	{
		if (!MetricZ_Config.IsLoaded())
			return false;

		if (!super.Begin())
			return false;

		string file;
		if (MetricZ_Config.Get().file.atomic)
			file = MetricZ_Config.Get().file.temp_file_path;
		else
			file = MetricZ_Config.Get().file.prom_file_path;

		m_Fh = OpenFile(file, FileMode.WRITE);
		if (!m_Fh) {
			ErrorEx("MetricZ: open file '" + file + "' failed", ErrorExSeverity.ERROR);
			return false;
		}

		return true;
	}

	/**
	    \brief Writes a metric line.
	    \details Delegates to buffer if buffering is enabled, otherwise writes directly to disk.
	*/
	override void Line(string line)
	{
		if (!IsBusy() || !m_Fh)
			return;

		if (IsBuffered())
			super.Line(line);
		else
			FPrintln(m_Fh, line);
	}

	/**
	    \brief Closes the file and finalizes the export.
	    \details If atomic mode is enabled, handles the swap (delete old -> copy tmp to final -> delete tmp).
	*/
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

		if (MetricZ_Config.Get().file.atomic) {
			string promFile = MetricZ_Config.Get().file.prom_file_path;
			string tempFile = MetricZ_Config.Get().file.temp_file_path;

			DeleteFile(promFile);

			if (!CopyFile(tempFile, promFile)) {
				ErrorEx(
				    "MetricZ: atomic publish failed '" + tempFile + "' -> '" + promFile + "'",
				    ErrorExSeverity.ERROR);
				return false;
			}

			DeleteFile(tempFile);
		}

		return true;
	}

	/**
	    \brief Flushes the internal string buffer to the file handle.
	*/
	override protected void BufferFlush()
	{
		if (m_Fh && GetBufferCount() > 0)
			FPrint(m_Fh, GetBufferChunk());

		super.BufferFlush();
	}
}
#endif
