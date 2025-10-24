/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Per-player metrics collector.
    \details Caches metric instances and label sets. Updated during scrape.
*/
class MetricZ_PlayerMetrics : MetricZ_EntityMetricsBase
{
	// Spawn timestamp in seconds
	protected int m_InitTick;

	// metrics cached labels
	protected string m_LabelsExtra;

	// parent player
	protected PlayerBase m_Player;

	// stats
	protected ref MetricZ_MetricInt m_IsLoaded;
	protected ref MetricZ_MetricFloat m_Health;
	protected ref MetricZ_MetricFloat m_Blood;
	protected ref MetricZ_MetricFloat m_Shock;
	protected ref MetricZ_MetricFloat m_Energy;
	protected ref MetricZ_MetricFloat m_Water;
	protected ref MetricZ_MetricFloat m_Toxicity;
	protected ref MetricZ_MetricFloat m_TempC;
	protected ref MetricZ_MetricFloat m_Weight;
	protected ref MetricZ_MetricFloat m_Wetness;
	protected ref MetricZ_MetricFloat m_LifeSeconds;

	// position
	protected ref MetricZ_MetricFloat m_PosX;
	protected ref MetricZ_MetricFloat m_PosY;
	protected ref MetricZ_MetricFloat m_PosZ;

	// extra stats
	protected ref MetricZ_MetricInt m_AgentsCount;
	protected ref MetricZ_MetricInt m_BleedingSources;
	protected ref MetricZ_MetricInt m_ImmunityBoosted;
	protected ref MetricZ_MetricInt m_Unconscious;
	protected ref MetricZ_MetricInt m_Restrained;
	protected ref MetricZ_MetricInt m_ThirdPerson;
	protected ref MetricZ_MetricInt m_GodMode;

	// analytics
	protected ref MetricZ_MetricFloat m_StatPlaytimeSeconds;
	protected ref MetricZ_MetricFloat m_StatDistanceMeters;
	protected ref MetricZ_MetricFloat m_StatLongestSurvivorHitMeters;
	protected ref MetricZ_MetricInt m_StatPlayersKilledTotal;
	protected ref MetricZ_MetricInt m_StatInfectedKilledTotal;

	/**
	    \brief Constructor. Initializes metric instances and spawn tick.
	*/
	void MetricZ_PlayerMetrics()
	{
		m_InitTick = g_Game.GetTickTime(); // seconds

		// vitals
		m_IsLoaded = new MetricZ_MetricInt("player_loaded", "Is player loaded (extra labels holder)", MetricZ_MetricType.GAUGE);
		m_Health = new MetricZ_MetricFloat("player_health", "Player health 0..1", MetricZ_MetricType.GAUGE);
		m_Blood = new MetricZ_MetricFloat("player_blood", "Player blood 0..1", MetricZ_MetricType.GAUGE);
		m_Shock = new MetricZ_MetricFloat("player_shock", "Player shock 0..1", MetricZ_MetricType.GAUGE);
		m_Energy = new MetricZ_MetricFloat("player_energy", "Player energy 0..1", MetricZ_MetricType.GAUGE);
		m_Water = new MetricZ_MetricFloat("player_water", "Player hydration 0..1", MetricZ_MetricType.GAUGE);
		m_Toxicity = new MetricZ_MetricFloat("player_toxicity", "Player toxicity 0..1", MetricZ_MetricType.GAUGE);
		m_TempC = new MetricZ_MetricFloat("player_temperature_celsius", "Player body temperature in celsius", MetricZ_MetricType.GAUGE);
		m_Weight = new MetricZ_MetricFloat("player_weight", "Player total weight in grams", MetricZ_MetricType.GAUGE);
		m_Wetness = new MetricZ_MetricFloat("player_wetness", "Player wetness 0..1", MetricZ_MetricType.GAUGE);
		m_LifeSeconds = new MetricZ_MetricFloat("player_lifetime_seconds", "Player lifetime since spawn or load in seconds", MetricZ_MetricType.GAUGE);

		// position
		if (MetricZ_Config.s_EnableCoordinatesMetrics) {
			m_PosX = new MetricZ_MetricFloat("player_position_x", "Player world X", MetricZ_MetricType.GAUGE);
			m_PosY = new MetricZ_MetricFloat("player_position_y", "Player world Y", MetricZ_MetricType.GAUGE);
			m_PosZ = new MetricZ_MetricFloat("player_position_z", "Player world Z", MetricZ_MetricType.GAUGE);
		}

		// extra stats
		m_AgentsCount = new MetricZ_MetricInt("player_agents_active", "Number of active disease agents affecting the player", MetricZ_MetricType.GAUGE);
		m_BleedingSources = new MetricZ_MetricInt("player_bleeding_active", "Number of active bleeding sources on player", MetricZ_MetricType.GAUGE);
		m_ImmunityBoosted = new MetricZ_MetricInt("player_immunity_boosted", "Immunity boosted (0/1)", MetricZ_MetricType.GAUGE);
		m_Unconscious = new MetricZ_MetricInt("player_unconscious", "Is unconscious (0/1)", MetricZ_MetricType.GAUGE);
		m_Restrained = new MetricZ_MetricInt("player_restrained", "Is restrained (0/1)", MetricZ_MetricType.GAUGE);
		m_ThirdPerson = new MetricZ_MetricInt("player_third_person", "Is in third person (0/1)", MetricZ_MetricType.GAUGE);
		m_GodMode = new MetricZ_MetricInt("player_godmode", "Damage disabled (0/1)", MetricZ_MetricType.GAUGE);

		// analytics
		m_StatPlaytimeSeconds = new MetricZ_MetricFloat("player_stat_playtime_seconds", "Analytics playtime", MetricZ_MetricType.GAUGE);
		m_StatDistanceMeters = new MetricZ_MetricFloat("player_stat_distance_meters", "Analytics distance", MetricZ_MetricType.GAUGE);
		m_StatLongestSurvivorHitMeters = new MetricZ_MetricFloat("player_stat_longest_survivor_hit_m", "Analytics longest survivor hit", MetricZ_MetricType.GAUGE);
		m_StatPlayersKilledTotal = new MetricZ_MetricInt("player_stat_players_killed", "Analytics players killed total", MetricZ_MetricType.COUNTER);
		m_StatInfectedKilledTotal = new MetricZ_MetricInt("player_stat_infected_killed", "Analytics infected killed total", MetricZ_MetricType.COUNTER);
	}

	/**
	    \brief One-time registry fill.
	*/
	void Init(PlayerBase player)
	{
		if (!player || player.IsDamageDestroyed())
			return;

		if (m_Registry.Count() > 0)
			return;

		m_Player = player;

		m_Registry.Insert(m_IsLoaded);
		m_Registry.Insert(m_Health);
		m_Registry.Insert(m_Blood);
		m_Registry.Insert(m_Shock);
		m_Registry.Insert(m_Energy);
		m_Registry.Insert(m_Water);
		m_Registry.Insert(m_Toxicity);
		m_Registry.Insert(m_TempC);
		m_Registry.Insert(m_Weight);
		m_Registry.Insert(m_Wetness);
		m_Registry.Insert(m_LifeSeconds);

		if (MetricZ_Config.s_EnableCoordinatesMetrics) {
			m_Registry.Insert(m_PosX);
			m_Registry.Insert(m_PosY);
			m_Registry.Insert(m_PosZ);
		}

		m_Registry.Insert(m_AgentsCount);
		m_Registry.Insert(m_BleedingSources);
		m_Registry.Insert(m_ImmunityBoosted);
		m_Registry.Insert(m_Unconscious);
		m_Registry.Insert(m_Restrained);
		m_Registry.Insert(m_ThirdPerson);
		m_Registry.Insert(m_GodMode);

		m_Registry.Insert(m_StatPlaytimeSeconds);
		m_Registry.Insert(m_StatDistanceMeters);
		m_Registry.Insert(m_StatLongestSurvivorHitMeters);
		m_Registry.Insert(m_StatPlayersKilledTotal);
		m_Registry.Insert(m_StatInfectedKilledTotal);

		SetLabels();
	}

	/**
	    \brief Update all metrics from the player state.
	*/
	override void Update()
	{
		if (!m_Player || m_Registry.Count() < 1)
			return;

		// if no identity on init try recreate labels
		if (m_Labels == string.Empty)
			SetLabels();

		// vitals
		m_IsLoaded.Set(MetricZ_LabelUtils.Bool(m_Player.IsPlayerLoaded()));
		m_Health.Set(m_Player.GetHealth01("", "Health"));
		m_Blood.Set(m_Player.GetHealth01("", "Blood"));
		m_Shock.Set(m_Player.GetHealth01("", "Shock"));
		m_Energy.Set(m_Player.GetStatEnergy().Get() / m_Player.GetStatEnergy().GetMax());
		m_Water.Set(m_Player.GetStatWater().Get() / m_Player.GetStatWater().GetMax());
		m_Toxicity.Set(m_Player.GetStatToxicity().Get() / m_Player.GetStatToxicity().GetMax());
		m_TempC.Set(m_Player.GetTemperature());
		m_Weight.Set(m_Player.GetPlayerLoad());
		m_Wetness.Set(m_Player.GetStatWet().Get());
		m_LifeSeconds.Set(g_Game.GetTickTime() - m_InitTick);

		// position
		if (MetricZ_Config.s_EnableCoordinatesMetrics) {
			vector pos = m_Player.GetPosition();
			m_PosX.Set(pos[0]);
			m_PosY.Set(pos[1]);
			m_PosZ.Set(pos[2]);
		}

		// base states
		m_AgentsCount.Set(MetricZ_LabelUtils.BitsCount(m_Player.GetAgents()));
		m_BleedingSources.Set(MetricZ_LabelUtils.BitsCount(m_Player.GetBleedingBits()));
		m_ImmunityBoosted.Set(MetricZ_LabelUtils.Bool(m_Player.m_ImmunityBoosted));
		m_Unconscious.Set(MetricZ_LabelUtils.Bool(m_Player.IsUnconscious()));
		m_Restrained.Set(MetricZ_LabelUtils.Bool(m_Player.IsRestrained()));
		m_ThirdPerson.Set(MetricZ_LabelUtils.Bool(m_Player.IsInThirdPerson()));
		m_GodMode.Set(MetricZ_LabelUtils.Bool(!m_Player.GetAllowDamage()));

		// analytics
		m_StatPlaytimeSeconds.Set(m_Player.StatGet(AnalyticsManagerServer.STAT_PLAYTIME));
		m_StatDistanceMeters.Set(m_Player.StatGet(AnalyticsManagerServer.STAT_DISTANCE));
		m_StatLongestSurvivorHitMeters.Set(m_Player.StatGet(AnalyticsManagerServer.STAT_LONGEST_SURVIVOR_HIT));
		m_StatPlayersKilledTotal.Set(m_Player.StatGet(AnalyticsManagerServer.STAT_PLAYERS_KILLED));
		m_StatInfectedKilledTotal.Set(m_Player.StatGet(AnalyticsManagerServer.STAT_INFECTED_KILLED));
	}

	/**
	    \brief Build and cache player label sets.
	*/
	override protected void SetLabels()
	{
		if (!m_Player || m_Labels != string.Empty)
			return;

		PlayerIdentity identity = m_Player.GetIdentity();
		if (!identity)
			return;

		map<string, string> labels = new map<string, string>();

		labels.Insert("steam_id", identity.GetPlainId());
		m_Labels = MetricZ_LabelUtils.MakeLabels(labels);

		string playerType = m_Player.GetType();
		playerType.TrimInPlace();

		bool _p;
		string _t;
		string bloodType = BloodTypes.GetBloodTypeName(m_Player.GetBloodType(), _t, _p);

		labels.Insert("guid", identity.GetId());

		string name = identity.GetName();
		if (name != string.Empty)
			labels.Insert("name", name);

		if (bloodType != string.Empty)
			labels.Insert("blood", bloodType);

		if (m_Player.IsMale()) {
			playerType.Replace("SurvivorM_", "");
			labels.Insert("gender", "male");
		} else {
			playerType.Replace("SurvivorF_", "");
			labels.Insert("gender", "female");
		}

		if (playerType != string.Empty)
			labels.Insert("type", playerType);

		m_LabelsExtra = MetricZ_LabelUtils.MakeLabels(labels);
	}

	override protected string LabelsFor(MetricZ_MetricBase metric)
	{
		if (metric == m_IsLoaded)
			return m_LabelsExtra;

		return m_Labels;
	}
}
#endif
