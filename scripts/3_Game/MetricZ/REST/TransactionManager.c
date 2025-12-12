/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Static manager for REST transaction state.
    \details Decouples upload state from the transient RestSink object.
             Handles "zombie" callbacks from previous retries.
*/
class MetricZ_RestTransactionManager
{
	protected static string s_CurrentTxn;
	protected static bool s_Sealed;
	protected static ref array<bool> s_Chunks;

	/**
	    \brief Start a new transaction.
	    \details Resets state. Any old callbacks will be ignored after this.
	*/
	static void Start(string txn)
	{
		s_CurrentTxn = txn;
		s_Sealed = false;

		if (!s_Chunks)
			s_Chunks = new array<bool>();
		else
			s_Chunks.Clear();

#ifdef DIAG
		ErrorEx("MetricZ: Txn started: " + txn, ErrorExSeverity.INFO);
#endif
	}

	/**
	    \brief Register a new chunk being sent.
	*/
	static int AddChunk(string txn)
	{
		if (txn != s_CurrentTxn)
			return -1;

		s_Chunks.Insert(false);
		return s_Chunks.Count() - 1;
	}

	/**
	    \brief Mark transaction as fully sent by Sink
	*/
	static void Seal(string txn)
	{
		if (txn != s_CurrentTxn)
			return;

		s_Sealed = true;
		CheckCommit();
	}

	/**
	    \brief Callback when a chunk is successfully uploaded
	*/
	static void OnChunkSuccess(string txn, int idx)
	{
		if (txn != s_CurrentTxn) {
#ifdef DIAG
			ErrorEx("MetricZ: transaction ignored zombie chunk: " + txn + " (current: " + s_CurrentTxn + ")", ErrorExSeverity.INFO);
#endif
			return;
		}

		if (idx >= 0 && idx < s_Chunks.Count()) {
			s_Chunks.Set(idx, true);
			CheckCommit();
		}
	}

	/**
	    \brief Check if txn is active (for retries)
	*/
	static bool IsActive(string txn)
	{
		return (txn != string.Empty && txn == s_CurrentTxn);
	}

	/**
	    \brief Internal check to trigger Commit
	*/
	protected static void CheckCommit()
	{
		if (!s_Sealed || s_Chunks.Count() == 0)
			return;

		foreach (bool done : s_Chunks) {
			if (!done)
				return;
		}

		// Trigger commit
		MetricZ_RestClient client = MetricZ_RestClient.Get();
		if (client) {
			MetricZ_CallbackCommitMetrics cb = new MetricZ_CallbackCommitMetrics(client);
			client.CommitMetrics(s_CurrentTxn, cb);

			s_CurrentTxn = string.Empty;
		}
	}
}
#endif
