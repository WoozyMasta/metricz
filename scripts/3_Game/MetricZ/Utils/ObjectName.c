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
		"metal", "wood", "wooden", "plastic",
		"desert", "sand",
		"flat",
		"chernarus", "livonia",
	};

	/**
	    \brief Derive a readable lowercased object type/class/model name.
	    \param obj       Source object.
	    \param stripBase If true, try to strip known postfix from final name
	    \return string Non-empty trimmed lowercase name or "unknown"/"none".
	*/
	static string GetName(Object obj, bool stripBase = false)
	{
		if (!obj)
			return "none";

		string name, strippedName;

		name = obj.GetType();
		if (name != string.Empty) {
			name.TrimInPlace();
			name.ToLower();

			strippedName = name;
			if (stripBase && StripSuffix(strippedName))
				return strippedName;

			return name;
		}

		name = obj.ClassName();
		if (name != string.Empty) {
			name.TrimInPlace();
			if (name != "Object" && name != "House") {
				name.ToLower();

				strippedName = name;
				if (stripBase && StripSuffix(strippedName))
					return strippedName;

				return name;
			}
		}

		name = obj.GetDebugNameNative();
		if (name.IndexOf("NOID ") == 0)
			name = name.Substring(5, name.Length() - 5);

		if (name == string.Empty)
			return "unknown";

		array<string> parts = new array<string>();
		name.Split(":", parts);

		string part;
		if (parts.Count() == 2) {
			if (parts[1].IndexOf(".p3d") > 0)
				part = parts[1].Substring(0, parts[1].Length() - 4);
			else
				part = parts[0];
		} else
			part = name;

		part.TrimInPlace();
		part.ToLower();

		return part;
	}

	/**
	        \brief Strips known suffixes recursively and cleans up the name.
	        \param name String to process (modified in place).
	        \return bool True if name is valid (not empty), false otherwise.
	*/
	static bool StripSuffix(inout string name)
	{
		name.TrimInPlace();
		if (name == string.Empty)
			return false;

		bool foundAny = false;
		while (true) {
			int sep = name.LastIndexOf("_");
			if (sep <= 0)
				break;

			string suffix = name.Substring(sep + 1, name.Length() - (sep + 1));
			bool isMatch = false;

			if (NAME_SUFFIXES.Find(suffix) != -1)
				isMatch = true;

			else {
				int len = suffix.Length();
				if (len > 3 && suffix.Substring(len - 3, 3) == "rnd") {
					if (IsDigits(suffix.Substring(0, len - 3)))
						isMatch = true;
				}
			}

			if (isMatch) {
				name = name.Substring(0, sep);
				foundAny = true;
			} else
				break;
		}

		while (name.Contains("__"))
			name.Replace("__", "_");

		while (name.Length() > 0 && name.Get(0) == "_")
			name = name.Substring(1, name.Length() - 1);

		while (name.Length() > 0 && name.Get(name.Length() - 1) == "_")
			name = name.Substring(0, name.Length() - 1);

		if (name == string.Empty)
			return false;

		return true;
	}

	/**
	    \brief Check that string consists only of digits 0-9.
	*/
	static bool IsDigits(string s)
	{
		if (s == string.Empty)
			return false;

		for (int i = 0; i < s.Length(); ++i) {
			int code = s.Substring(i, 1).ToAscii();
			if (code < 48 || code > 57) // '0'..'9'
				return false;
		}

		return true;
	}
}
