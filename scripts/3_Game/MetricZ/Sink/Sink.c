/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
class MetricZ_Sink
{
	protected ref array<string> m_Lines;
	protected int m_BufferLines;
	protected bool m_BufferedExport;
	protected bool m_AtomicExport;
	protected bool m_RemoteExport;
	protected bool m_Busy;
	protected FileHandle m_Fh;

	void MetricZ_Sink()
	{
		if (!MetricZ_Config.IsLoaded())
			return;

		m_Lines = new array<string>();
		m_BufferLines = MetricZ_Config.Get().bufferLines;
		m_BufferedExport = MetricZ_Config.Get().bufferedFileExport;
		m_AtomicExport = MetricZ_Config.Get().atomicFileExport;
		m_RemoteExport = MetricZ_Config.Get().enableRemoteExport;

		m_Busy = false;
	}

	bool Begin()
	{
		if (!MetricZ_Config.IsLoaded() || m_Busy)
			return false;

		if (m_RemoteExport) {
			m_Busy = true;
			return true;
		}

		string file;
		if (m_AtomicExport)
			file = MetricZ_Config.METRICS_TEMP;
		else
			file = MetricZ_Config.METRICS_FILE;

		if (file == string.Empty)
			return false;

		m_Fh = OpenFile(file, FileMode.WRITE);
		if (!m_Fh) {
			ErrorEx("MetricZ: open file " + file + " failed", ErrorExSeverity.ERROR);
			return false;
		}

#ifdef DIAG
		ErrorEx("MetricZ: Sink: begin", ErrorExSeverity.INFO);
#endif

		m_Busy = true;
		return true;
	}

	void Line(string line)
	{
		if (!MetricZ_Config.IsLoaded() || !m_Busy)
			return;

		if (m_RemoteExport && m_Lines) {
			m_Lines.Insert(line);
			return;
		}

		if (!m_Fh)
			return;

		if (!m_BufferedExport) {
			FPrint(m_Fh, line + "\n");
			return;
		}

		if (m_Lines) {
			m_Lines.Insert(line);

			if (m_Lines.Count() >= m_BufferLines)
				Flush();
		}
	}

	bool End()
	{
		if (!MetricZ_Config.IsLoaded() || !m_Busy)
			return false;

		m_Busy = false;

		if (m_RemoteExport) {
			Post()
			return true;
		}

		if (m_Fh) {
			Flush();
			CloseFile(m_Fh);
			m_Fh = null;
		}

		if (m_AtomicExport) {
			DeleteFile(MetricZ_Config.METRICS_FILE);
			if (!CopyFile(MetricZ_Config.METRICS_TEMP, MetricZ_Config.METRICS_FILE)) {
				ErrorEx("MetricZ: publish failed " + MetricZ_Config.METRICS_TEMP + " -> " + MetricZ_Config.METRICS_FILE);
				return false;
			}

			DeleteFile(MetricZ_Config.METRICS_TEMP);

#ifdef DIAG
			ErrorEx("MetricZ: Sink: rotate files " + MetricZ_Config.METRICS_TEMP + " -> " + MetricZ_Config.METRICS_FILE, ErrorExSeverity.INFO);
#endif
		}

#ifdef DIAG
		ErrorEx("MetricZ: Sink: end", ErrorExSeverity.INFO);
#endif

		return true;
	}

	private void Flush()
	{
		if (!m_Fh || !m_Lines || m_Lines.Count() == 0)
			return;

		string result;
		foreach (string line : m_Lines)
			result += line + "\n";

		FPrint(m_Fh, result);
		m_Lines.Clear();
	}

	private void Post()
	{
		if (!m_RemoteExport || !m_Lines || m_Lines.Count() == 0)
			return;

		MetricZ_RestClient client = MetricZ_RestClient.Get();
		if (!client) {
			ErrorEx("MetricZ REST: client null", ErrorExSeverity.ERROR);
			return;
		}

		string body;
		foreach (string line : m_Lines)
			body += line + "\n";

		MetricZ_RestCallback cb = new MetricZ_RestCallback(client, body);
		client.PostBody(body, cb);

#ifdef DIAG
		ErrorEx("MetricZ: Sink: end with HTTP POST", ErrorExSeverity.INFO);
#endif
	}
}
#endif
