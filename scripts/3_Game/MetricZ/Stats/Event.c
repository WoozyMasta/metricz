/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Event counters aggregator
*/
class MetricZ_EventStats
{
	protected static const int UNKNOWN_EVENT = -1;
	protected static bool s_Initialized;
	protected static ref map<EventType, ref MetricZ_MetricInt> s_EventsRegistry;

	/**
	    \brief Increment counter for an EventType.
	    \param eventTypeId Engine event id.
	*/
	static void Inc(EventType eventTypeId)
	{
		if (!s_Initialized)
			return;

		MetricZ_MetricInt metric;
		if (s_EventsRegistry.Find(eventTypeId, metric)) {
			metric.Inc();
			return;
		}

		if (s_EventsRegistry.Find(UNKNOWN_EVENT, metric))
			metric.Inc();
	}

	/**
	    \brief Emit HELP/TYPE and per-event samples.
	    \details Writes one sample per EventType with label event name.
	    \param fh Open file handle.
	*/
	static void Flush(FileHandle fh)
	{
#ifdef DIAG
		float t0 = g_Game.GetTickTime();
#endif

		if (!fh || !s_Initialized)
			return;

		bool headerWritten;
		foreach (MetricZ_MetricBase metric : s_EventsRegistry) {
			if (!headerWritten) {
				metric.WriteHeaders(fh);
				headerWritten = true;
			}

			metric.Flush(fh);
		}

#ifdef DIAG
		ErrorEx("MetricZ events_total scraped in " + (g_Game.GetTickTime() - t0).ToString() + "s", ErrorExSeverity.INFO);
#endif
	}

	/**
	    \brief Build the EventType->MetricZ_MetricInt metrics registry once.
	*/
	static void Init()
	{
		if (s_Initialized || MetricZ_Config.s_DisableEventMetrics)
			return;

		s_EventsRegistry = new map<EventType, ref MetricZ_MetricInt>();

		map<EventType, string> eventNames = new map<EventType, string>();

		// placeholder for all unknown
		eventNames.Insert(UNKNOWN_EVENT, "Unknown");
		// system
		eventNames.Insert(StartupEventTypeID, "Startup");
		eventNames.Insert(WorldCleaupEventTypeID, "WorldCleanUP");
		eventNames.Insert(SelectedUserChangedEventTypeID, "SelectedUserChanged");
		eventNames.Insert(ScriptLogEventTypeID, "ScriptLog");

		// MP session
		eventNames.Insert(MPSessionStartEventTypeID, "MPSessionStart");
		eventNames.Insert(MPSessionEndEventTypeID, "MPSessionEnd");
		eventNames.Insert(MPSessionFailEventTypeID, "MPSessionFail");
		eventNames.Insert(MPSessionPlayerReadyEventTypeID, "MPSessionPlayerReady");
		eventNames.Insert(MPConnectionLostEventTypeID, "MPConnectionLost");
		eventNames.Insert(MPConnectionCloseEventTypeID, "MPConnectionClose");

		// misc
		eventNames.Insert(ProgressEventTypeID, "Progress");
		eventNames.Insert(NetworkManagerClientEventTypeID, "NetworkManagerClient");
		eventNames.Insert(NetworkManagerServerEventTypeID, "NetworkManagerServer");
		eventNames.Insert(DialogQueuedEventTypeID, "DialogQueued");

		// chat
		eventNames.Insert(ChatMessageEventTypeID, "ChatMessage");
		eventNames.Insert(ChatChannelEventTypeID, "ChatChannel");

		// client lifecycle
		eventNames.Insert(ClientConnectedEventTypeID, "ClientConnected");
		eventNames.Insert(ClientPrepareEventTypeID, "ClientPrepare");
		eventNames.Insert(ClientNewEventTypeID, "ClientNew");
		eventNames.Insert(ClientNewReadyEventTypeID, "ClientNewReady");
		eventNames.Insert(ClientRespawnEventTypeID, "ClientReSpawn");
		eventNames.Insert(ClientReconnectEventTypeID, "ClientReconnect");
		eventNames.Insert(ClientReadyEventTypeID, "ClientReady");
		eventNames.Insert(ClientDisconnectedEventTypeID, "ClientDisconnected");
		eventNames.Insert(ClientRemovedEventTypeID, "ClientRemoved");

		// connectivity and server stats
		eventNames.Insert(ConnectivityStatsUpdatedEventTypeID, "ConnectivityStatsUpdated");
		eventNames.Insert(ServerFpsStatsUpdatedEventTypeID, "ServerFpsStatsUpdated");

		// login/logout flow
		eventNames.Insert(LogoutCancelEventTypeID, "LogoutCancel");
		eventNames.Insert(LoginTimeEventTypeID, "LoginTime");
		eventNames.Insert(RespawnEventTypeID, "Respawn");
		eventNames.Insert(PreloadEventTypeID, "Preload");
		eventNames.Insert(LogoutEventTypeID, "Logout");
		eventNames.Insert(LoginStatusEventTypeID, "LoginStatus");

		// VON
		eventNames.Insert(VONStateEventTypeID, "VONState");
		eventNames.Insert(VONStartSpeakingEventTypeID, "VONStartSpeaking");
		eventNames.Insert(VONStopSpeakingEventTypeID, "VONStopSpeaking");
		eventNames.Insert(VONUserStartedTransmittingAudioEventTypeID, "VONStartedTransmitting");
		eventNames.Insert(VONUserStoppedTransmittingAudioEventTypeID, "VONStoppedTransmitting");

		// other
		eventNames.Insert(PartyChatStatusChangedEventTypeID, "PartyChatStatusChanged");
		eventNames.Insert(DLCOwnerShipFailedEventTypeID, "DLCOwnerShipFailed");
		eventNames.Insert(SetFreeCameraEventTypeID, "SetFreeCamera");
		eventNames.Insert(ConnectingStartEventTypeID, "ConnectingStart");
		eventNames.Insert(ConnectingAbortEventTypeID, "ConnectingAbort");
		eventNames.Insert(PlayerDeathEventTypeID, "PlayerDeath");
		eventNames.Insert(NetworkInputBufferEventTypeID, "NetworkInputBuffer");

		// create metrics per event type with static labels
		foreach (int id, string name: eventNames) {
			MetricZ_MetricInt metric = new MetricZ_MetricInt(
			    "events",
			    "Total events by EventType",
			    MetricZ_MetricType.COUNTER);

			// build labels once
			map<string, string> labels = new map<string, string>();
			labels.Insert("event", name);
			metric.MakeLabels(labels);

			s_EventsRegistry.Insert(id, metric);
		}

		s_Initialized = true;
	}
}
#endif
