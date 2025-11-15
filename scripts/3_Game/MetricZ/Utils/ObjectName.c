/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Helpers for deriving normalized object names.
    \details Provides a consistent, lowercased type name and optional suffix stripping
             (color/base/state/mag variants) for labeling and metric grouping.
*/
class MetricZ_ObjectName
{
	// Known suffixes to strip from type names
	static const ref array<string> NAME_SUFFIXES = {
		"base",
		"colorbase",

		// state
		"opened", "closed",
		"dirty", "rust", "damaged", "wet", "damp", "dry", "clean",

		// colors
		"black", "white",
		"grey", "gray", "lightgrey", "lightgray", "darkgrey", "darkgray", "silver",
		"red", "pink", "magenta", "gold",
		"orange", "tan",
		"yellow", "mustard",
		"green", "lightgreen", "darkgreen", "olive", "lime",
		"cyan", "teal", "aqua",
		"blue", "lightblue", "darkblue", "cobalt",
		"violet", "purple", "lavender",
		"brown", "lightbrown", "darkbrown", "beige",

		// seasons
		"winter",
		"summer",
		"spring",
		"autumn",

		// patterns / styles
		"natural",
		"stripes", "greenchecks",
		"camo", "multicam", "flecktarn", "digital",
		"snow", "arctic",
		"woodland", "forest", "jungle", "mossy",
		"tropic", "tropical",
		"khaki",
		"navy",
		"metal",
		"desert", "sand",
		"flat",
	};

	/**
	    \brief Derive a readable object type/name.
	    \details Resolution order:
	             1) Object.GetType()
	             2) Object.ClassName() (skips generic "Object"/"House")
	             3) Object.GetDebugNameNative() with "NOID " prefix handling and ".p3d" trimming.
	             Result is lowercased; optionally strips common postfix after last "_" (color/base/state/etc.).
	    \param obj       Source object.
	    \param stripBase If true, try to strip known postfix from final name
	                     (suffixes in NAME_SUFFIXES or "<digits>rnd" magazine-style endings).
	    \return string Non-empty trimmed lowercase name or "unknown"/"none".
	*/
	static string GetName(Object obj, bool stripBase = false)
	{
		if (!obj)
			return "none";

		string typeName = obj.GetType();
		if (typeName != string.Empty) {
			typeName.TrimInPlace();
			typeName.ToLower();

			if (stripBase)
				typeName = StripSuffix(typeName);

			return typeName;
		}

		string className = obj.ClassName();
		if (className != string.Empty) {
			className.TrimInPlace();
			if (className != "Object" && className != "House") {
				className.ToLower();

				if (stripBase)
					className = StripSuffix(className);

				return className;
			}
		}

		string dbg = obj.GetDebugNameNative();

		// drop "NOID " prefix for house proxies
		if (dbg.IndexOf("NOID ") == 0)
			dbg = dbg.Substring(5, dbg.Length() - 5);

		if (dbg == string.Empty)
			return "unknown";

		array<string> parts = new array<string>();
		dbg.Split(":", parts);

		string part;
		if (parts.Count() == 2) {
			if (parts[1].IndexOf(".p3d") > 0)
				part = parts[1].Substring(0, parts[1].Length() - 4);
			else
				part = parts[0];
		} else
			part = dbg;

		part.TrimInPlace();
		part.ToLower();

		return part;
	}

	/**
	    \brief Strip a single known suffix from a normalized name.
	    \details
	        Input must already be trimmed and lowercased.
	        Handles:
	          - known NAME_SUFFIXES (e.g. "_black", "_winter", "_colorbase", ...)
	          - magazine-style numeric suffix "<digits>rnd" (e.g. "_30rnd")
	    \param name Normalized type name (lowercase).
	    \return string Name without the last known suffix, or original name if nothing matches.
	*/
	static string StripSuffix(string name)
	{
		name.TrimInPlace();
		if (name == string.Empty)
			return name;

		int sep = name.LastIndexOf("_");
		if (sep <= 0 || sep >= name.Length() - 1)
			return name;

		string suffix = name.Substring(sep + 1, name.Length() - (sep + 1));
		if (suffix == string.Empty)
			return name;

		for (int i = 0; i < NAME_SUFFIXES.Count(); i++) {
			if (suffix == NAME_SUFFIXES[i])
				return name.Substring(0, sep);
		}

		// generic <digits>rnd, e.g. 5rnd, 20rnd, 100rnd, 1000rnd...
		int len = suffix.Length();
		if (len > 3 && suffix.Substring(len - 3, 3) == "rnd") {
			if (IsDigits(suffix.Substring(0, len - 3)))
				return name.Substring(0, sep);
		}

		return name;
	}

	/**
	    \brief Check that string consists only of digits 0-9.
	*/
	static bool IsDigits(string s)
	{
		if (s == string.Empty)
			return false;

		for (int i = 0; i < s.Length(); i++) {
			int code = s.Substring(i, 1).ToAscii();
			if (code < 48 || code > 57) // '0'..'9'
				return false;
		}

		return true;
	}
}
