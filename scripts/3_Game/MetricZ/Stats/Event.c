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
	protected static ref map<EventType, string> s_EventNames;
	protected static ref map<EventType, int> s_EventsRegistry = new map<EventType, int>(); // eventTypeId -> count
	protected static ref MetricZ_MetricInt s_EventTotal = new MetricZ_MetricInt(
	    "events",
	    "Total events by EventType",
	    MetricZ_MetricType.COUNTER);

	/**
	    \brief Increment counter for an EventType.
	    \param eventTypeId Engine event id.
	*/
	static void Inc(EventType eventTypeId)
	{
		int v;
		if (s_EventsRegistry.Find(eventTypeId, v))
			s_EventsRegistry.Set(eventTypeId, v + 1);
		else
			s_EventsRegistry.Insert(eventTypeId, 1);
	}

	/**
	    \brief Emit HELP/TYPE and per-event samples.
	    \details Builds the EventType->name map lazily. Writes one sample per EventType with labels {id, event}.
	    \param MetricZ_Sink sink instance
	*/
	static void Flush(MetricZ_Sink sink)
	{
		if (!sink || s_EventsRegistry.Count() == 0)
			return;

#ifdef DIAG
		float t0 = g_Game.GetTickTime();
#endif

		MakeNameMap();
		s_EventTotal.WriteHeaders(sink);

		for (int i = 0; i < s_EventsRegistry.Count(); i++) {
			int id = s_EventsRegistry.GetKey(i);
			int val = s_EventsRegistry.GetElement(i);

			s_EventTotal.Set(val);

			string name = EventName(s_EventsRegistry.GetKey(i));
			map<string, string> labels = new map<string, string>();
			labels.Insert("id", id.ToString());
			labels.Insert("event", name);

			s_EventTotal.Flush(sink, MetricZ_LabelUtils.MakeLabels(labels));
		}

#ifdef DIAG
		ErrorEx("MetricZ events_total scraped in " + (g_Game.GetTickTime() - t0).ToString() + "s", ErrorExSeverity.INFO);
#endif
	}

	/**
	    \brief Build the EventType->name map once.
	    \details No-op if the map already exists.
	*/
	protected static void MakeNameMap()
	{
		if (s_EventNames)
			return;

		s_EventNames = new map<EventType, string>();

		// system
		s_EventNames.Insert(StartupEventTypeID, "Startup");
		s_EventNames.Insert(WorldCleaupEventTypeID, "WorldCleanUP");
		s_EventNames.Insert(SelectedUserChangedEventTypeID, "SelectedUserChanged");
		s_EventNames.Insert(ScriptLogEventTypeID, "ScriptLog");

		// MP session
		s_EventNames.Insert(MPSessionStartEventTypeID, "MPSessionStart");
		s_EventNames.Insert(MPSessionEndEventTypeID, "MPSessionEnd");
		s_EventNames.Insert(MPSessionFailEventTypeID, "MPSessionFail");
		s_EventNames.Insert(MPSessionPlayerReadyEventTypeID, "MPSessionPlayerReady");
		s_EventNames.Insert(MPConnectionLostEventTypeID, "MPConnectionLost");
		s_EventNames.Insert(MPConnectionCloseEventTypeID, "MPConnectionClose");

		// misc
		s_EventNames.Insert(ProgressEventTypeID, "Progress");
		s_EventNames.Insert(NetworkManagerClientEventTypeID, "NetworkManagerClient");
		s_EventNames.Insert(NetworkManagerServerEventTypeID, "NetworkManagerServer");
		s_EventNames.Insert(DialogQueuedEventTypeID, "DialogQueued");

		// chat
		s_EventNames.Insert(ChatMessageEventTypeID, "ChatMessage");
		s_EventNames.Insert(ChatChannelEventTypeID, "ChatChannel");

		// client lifecycle
		s_EventNames.Insert(ClientConnectedEventTypeID, "ClientConnected");
		s_EventNames.Insert(ClientPrepareEventTypeID, "ClientPrepare");
		s_EventNames.Insert(ClientNewEventTypeID, "ClientNew");
		s_EventNames.Insert(ClientNewReadyEventTypeID, "ClientNewReady");
		s_EventNames.Insert(ClientRespawnEventTypeID, "ClientReSpawn");
		s_EventNames.Insert(ClientReconnectEventTypeID, "ClientReconnect");
		s_EventNames.Insert(ClientReadyEventTypeID, "ClientReady");
		s_EventNames.Insert(ClientDisconnectedEventTypeID, "ClientDisconnected");
		s_EventNames.Insert(ClientRemovedEventTypeID, "ClientRemoved");

		// connectivity and server stats
		s_EventNames.Insert(ConnectivityStatsUpdatedEventTypeID, "ConnectivityStatsUpdated");
		s_EventNames.Insert(ServerFpsStatsUpdatedEventTypeID, "ServerFpsStatsUpdated");

		// login/logout flow
		s_EventNames.Insert(LogoutCancelEventTypeID, "LogoutCancel");
		s_EventNames.Insert(LoginTimeEventTypeID, "LoginTime");
		s_EventNames.Insert(RespawnEventTypeID, "Respawn");
		s_EventNames.Insert(PreloadEventTypeID, "Preload");
		s_EventNames.Insert(LogoutEventTypeID, "Logout");
		s_EventNames.Insert(LoginStatusEventTypeID, "LoginStatus");

		// VON
		s_EventNames.Insert(VONStateEventTypeID, "VONState");
		s_EventNames.Insert(VONStartSpeakingEventTypeID, "VONStartSpeaking");
		s_EventNames.Insert(VONStopSpeakingEventTypeID, "VONStopSpeaking");
		s_EventNames.Insert(VONUserStartedTransmittingAudioEventTypeID, "VONStartedTransmitting");
		s_EventNames.Insert(VONUserStoppedTransmittingAudioEventTypeID, "VONStoppedTransmitting");

		// other
		s_EventNames.Insert(PartyChatStatusChangedEventTypeID, "PartyChatStatusChanged");
		s_EventNames.Insert(DLCOwnerShipFailedEventTypeID, "DLCOwnerShipFailed");
		s_EventNames.Insert(SetFreeCameraEventTypeID, "SetFreeCamera");
		s_EventNames.Insert(ConnectingStartEventTypeID, "ConnectingStart");
		s_EventNames.Insert(ConnectingAbortEventTypeID, "ConnectingAbort");
		s_EventNames.Insert(PlayerDeathEventTypeID, "PlayerDeath");
		s_EventNames.Insert(NetworkInputBufferEventTypeID, "NetworkInputBuffer");
	}

	/**
	    \brief Resolve a human-readable name for an EventType.
	    \param id EventType id.
	    \return \p string Name or "unknown".
	*/
	protected static string EventName(EventType id)
	{
		if (!s_EventNames)
			return "unknown";

		string name;
		if (s_EventNames.Find(id, name))
			return name;

		return "unknown";
	}
}
#endif
