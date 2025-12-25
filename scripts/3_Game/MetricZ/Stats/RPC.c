/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief RPC counters aggregator
*/
class MetricZ_RpcStats
{
	protected static ref map<int, int> s_InputRPCsRegistry = new map<int, int>(); // rpc_type -> count
	protected static ref MetricZ_MetricInt s_RpcTotal = new MetricZ_MetricInt(
	    "rpc_input",
	    "Total input RPC calls",
	    MetricZ_MetricType.COUNTER);

	/**
	    \brief Increment counter for an RPC type.
	    \param rpc_type Engine RPC id.
	*/
	static void Inc(int rpc_type)
	{
		int v;
		if (s_InputRPCsRegistry.Find(rpc_type, v))
			s_InputRPCsRegistry.Set(rpc_type, v + 1);
		else
			s_InputRPCsRegistry.Insert(rpc_type, 1);
	}

	/**
	    \brief Emit HELP/TYPE and per-RPC samples.
	    \details Uses the in-memory registry and writes one sample per rpc_type with label {id="<id>"}.
	              Headers written once per family.
	    \param MetricZ_SinkBase sink instance
	*/
	static void Flush(MetricZ_SinkBase sink)
	{
		if (!sink || s_InputRPCsRegistry.Count() == 0)
			return;

		// headers once
		s_RpcTotal.WriteHeaders(sink);

		// emit per id
		for (int i = 0; i < s_InputRPCsRegistry.Count(); ++i) {
			int id = s_InputRPCsRegistry.GetKey(i);
			int val = s_InputRPCsRegistry.GetElement(i);

			s_RpcTotal.Set(val);

			map<string, string> labels = new map<string, string>();
			labels.Insert("id", id.ToString());

			s_RpcTotal.Flush(sink, MetricZ_LabelUtils.MakeLabels(labels));
		}
	}
}
#endif
