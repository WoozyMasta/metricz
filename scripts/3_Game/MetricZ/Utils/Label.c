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
	protected static bool s_BaseLabelReady;
	protected static string s_BaseLabel; // raw: key="val",key2="val2"

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

		string seed;
		int p1, p2, p3, p4;
		entity.GetPersistentID(p1, p2, p3, p4);

		// Non-persistent objects get a random hash;
		// stability is guaranteed only if called once per instance for labels.
		if (p1 == 0 && p2 == 0 && p3 == 0 && p4 == 0) {
			seed = entity.GetType() + "_" + Math.RandomInt(1, int.MAX).ToString();
			return seed.Hash();
		}

		seed = p1.ToString() + "_" + p2.ToString() + "_" + p3.ToString() + "_" + p4.ToString();
		return seed.Hash();
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
		if (!labels || labels.Count() == 0)
			return "{" + BaseLabels() + "}";

		string result = "{" + BaseLabels();
		foreach (string k, string v : labels) {
			k.TrimInPlace();
			v.TrimInPlace();

			if (k == string.Empty || v == string.Empty)
				continue;

			k.ToLower();
			k.Replace(" ", "_");

			if (k == "world" || k == "host" || k == "instance_id")
				continue;

			result += "," + k + "=\"" + Escape(v) + "\"";
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
		s_BaseLabel = "";
	}

	/**
	    \brief Build or return cached base label fragment.
	    \details Produces "world=...,host=...,instance_id=..." without braces.
	             Result is cached after first build to avoid repeated string ops.
	    \return \p string Comma-separated base labels without surrounding braces.
	*/
	static string BaseLabels()
	{
		if (s_BaseLabelReady)
			return s_BaseLabel;

		s_BaseLabel = "";

		// base: world
		string worldName;
		g_Game.GetWorldName(worldName);
		worldName.TrimInPlace();
		worldName.ToLower();
		if (worldName != string.Empty)
			s_BaseLabel += "world=\"" + Escape(worldName) + "\",";

		// base: host
		string host = GetMachineName();
		host.TrimInPlace();
		host.ToLower();
		if (host != string.Empty)
			s_BaseLabel += "host=\"" + Escape(host) + "\",";

		// base: instance id (allowed to be "0")
		s_BaseLabel += "instance_id=\"" + MetricZ_Config.Get().settings.instance_id + "\"";
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
}
#endif
