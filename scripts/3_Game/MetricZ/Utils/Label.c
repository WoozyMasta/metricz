/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Utility helpers for MetricZ.
*/
class MetricZ_LabelUtils
{
	protected static bool s_BaseLabelReady; //!< Indicates whether base labels are cached
	protected static string s_BaseLabel; //!< Cached base labels: key="val",key2="val2"
	protected static string s_BaseLabelBraced; //!< Cached braced base labels: {key="val",key2="val2"}
	protected static ref map<string, bool> s_DenyLabels; //!< Denylist for forbidden label keys

	/**
	    \brief Escape backslash and quote for Prometheus label values.
	    \param s Input string
	    \return \p string Escaped value
	*/
	static string Escape(string s)
	{
		string bs = "\\";
		string dq = "\"";
		s.Replace(bs, bs + bs);
		s.Replace(dq, bs + dq);

		return s;
	}

	/**
	    \brief Generate stable hash int from entity persistent ID.
	    \details Combines four persistent-ID ints and hashes them.
	             Falls back to a pseudo-unique hash for non-persistent entities.
	    \param entity EntityAI with persistence
	    \return \p int hash like -123456789 or 987654321
	*/
	static int PersistentHash(EntityAI entity)
	{
		if (!entity)
			return 0;

		int p1, p2, p3, p4;
		entity.GetPersistentID(p1, p2, p3, p4);

		// Non-persistent objects get a random hash;
		// stability is guaranteed only if called once per instance for labels.
		if (p1 == 0 && p2 == 0 && p3 == 0 && p4 == 0)
			return string.Format("%1_%2", entity.GetType(), Math.RandomInt(1, int.MAX)).Hash();

		return string.Format("%1_%2_%3_%4", p1, p2, p3, p4).Hash();
	}

	/**
	    \brief Build a Prometheus label set string.
	    \details Auto-includes base labels {world, host, instance_id}.
	             Merges user labels without overwriting base keys.
	             Skips empty world/host. Order is unspecified.
	    \param labels Optional map of extra key->value pairs
	    \return \p string "{k="v",...}"
	*/
	static string MakeLabels(map<string, string> labels = null)
	{
		if (!labels || labels.Count() == 0) {
			if (s_BaseLabelReady)
				return s_BaseLabelBraced;
			else
				return string.Format("{%1}", BaseLabels());
		}

		string result = string.Format("{%1", BaseLabels());
		foreach (string k, string v : labels) {
			k.TrimInPlace();
			v.TrimInPlace();

			if (k == string.Empty || v == string.Empty)
				continue;

			k.ToLower();
			k.Replace(" ", "_");

			if (IsDenied(k))
				continue;

			result += string.Format(",%1=\"%2\"", k, Escape(v))
		}
		result += "}";

		return result;
	}

	/**
	    \brief Invalidate cached base labels.
	    \details Clears the cached string and marks it as not ready.
	             Call this if world/host/instance_id can change at runtime.
	*/
	static void InvalidateBaseLabels()
	{
		s_BaseLabelReady = false;
		s_BaseLabel = string.Empty;
		s_BaseLabelBraced = string.Empty;
	}

	/**
	    \brief Build or return cached base label fragment.
	    \details Produces "world=...,host=...,instance_id=..." without braces.
	             Result is cached after first build to avoid repeated string ops.
	    \return \p string Comma-separated base labels without surrounding braces.
	*/
	static string BaseLabels()
	{
		if (!g_Game || s_BaseLabelReady)
			return s_BaseLabel;

		s_BaseLabel = "";

		// base: world
		string worldName = MetricZ_Config.Get().geo.world_name;
		if (worldName != string.Empty)
			s_BaseLabel += string.Format("world=\"%1\",", Escape(worldName));

		// base: host
		string host = MetricZ_Config.Get().settings.host_name_resolved;
		if (host != string.Empty)
			s_BaseLabel += string.Format("host=\"%1\",", Escape(host));

		// base: instance id (allowed to be "0")
		s_BaseLabel += string.Format("instance_id=\"%1\"", Escape(MetricZ_Config.Get().settings.instance_id_resolved));
		s_BaseLabelBraced = string.Format("{%1}", s_BaseLabel);
		s_BaseLabelReady = true;

		return s_BaseLabel;
	}

	/**
	    \brief Convert bool to 0/1.
	    \param x Boolean
	    \return \p int 1 if true else 0
	*/
	static int Bool(bool x)
	{
		if (x)
			return 1;

		return 0;
	}

	/**
	    \brief Count number of set bits in bitmask.
	    \details Uses Brian Kernighan's algorithm. Each iteration clears the lowest set bit.
	    \param mask Integer bitmask
	    \return \p int Number of bits set to 1
	*/
	static int BitsCount(int mask)
	{
		int count = 0;
		while (mask != 0) {
			mask &= mask - 1; // clear lowest set bit
			count++;
		}

		return count;
	}

	/**
	    \brief Check whether a label key is forbidden.
	    \details
	        - Rejects all Prometheus internal labels (__*)
	        - Rejects predefined, exporter-owned, and reserved labels
	    \param key Normalized label key
	    \return \p bool True if the key must not be accepted
	*/
	private static bool IsDenied(string key)
	{
		InitDenyLabels();

		// __* and __name__ are reserved
		if (key.Length() >= 2 && key.Get(0) == "_" && key.Get(1) == "_")
			return true;

		return s_DenyLabels.Contains(key);
	}

	/**
	    \brief Initialize denylist of forbidden Prometheus label keys.
	*/
	private static void InitDenyLabels()
	{
		if (s_DenyLabels)
			return;

		s_DenyLabels = new map<string, bool>();

		// Predefined labels
		s_DenyLabels.Insert("world", true);
		s_DenyLabels.Insert("host", true);
		s_DenyLabels.Insert("instance_id", true);
		// MetricZ Exporter labels
		s_DenyLabels.Insert("exporter", true);
		// Prometheus reserved labels
		s_DenyLabels.Insert("job", true);
		s_DenyLabels.Insert("instance", true);
		// For future histogram buckets and summaries
		s_DenyLabels.Insert("le", true);
		s_DenyLabels.Insert("quantile", true);
	}
}
#endif
