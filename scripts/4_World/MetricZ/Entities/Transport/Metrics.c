/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Per-Transport metrics collector.
    \details Caches metric instances and label sets. Updated during scrape.
*/
class MetricZ_TransportMetrics : MetricZ_EntityMetricsBase
{
	// parent transport
	protected Transport m_Transport;

	// generic
	protected ref MetricZ_MetricFloat m_Health;
	protected ref MetricZ_MetricInt m_Passengers;
	protected ref MetricZ_MetricFloat m_SpeedMS;
	protected ref MetricZ_MetricInt m_EngineOn;
	protected ref MetricZ_MetricFloat m_FuelFraction;
	protected ref MetricZ_MetricFloat m_PosX;
	protected ref MetricZ_MetricFloat m_PosY;
	protected ref MetricZ_MetricFloat m_PosZ;

	/**
	    \brief Constructor. Initializes metric instances and spawn tick.
	*/
	void MetricZ_TransportMetrics()
	{
		m_Health = new MetricZ_MetricFloat(
		    "transport_health",
		    "Transport health 0..1",
		    MetricZ_MetricType.GAUGE);
		m_Passengers = new MetricZ_MetricInt(
		    "transport_crew_occupied",
		    "Number of occupied seats in transport",
		    MetricZ_MetricType.GAUGE);
		m_SpeedMS = new MetricZ_MetricFloat(
		    "transport_speed",
		    "Transport speed, m/s",
		    MetricZ_MetricType.GAUGE);
		m_EngineOn = new MetricZ_MetricInt(
		    "transport_engine_on",
		    "Engine is on (0/1)",
		    MetricZ_MetricType.GAUGE);
		m_FuelFraction = new MetricZ_MetricFloat(
		    "transport_fuel_fraction",
		    "Fuel fraction 0..1",
		    MetricZ_MetricType.GAUGE);

		// position
		if (MetricZ_Config.s_EnableCoordinatesMetrics) {
			m_PosX = new MetricZ_MetricFloat(
			    "transport_position_x",
			    "Transport world X",
			    MetricZ_MetricType.GAUGE);
			m_PosY = new MetricZ_MetricFloat(
			    "transport_position_y",
			    "Transport world Y",
			    MetricZ_MetricType.GAUGE);
			m_PosZ = new MetricZ_MetricFloat(
			    "transport_position_z",
			    "Transport world Z",
			    MetricZ_MetricType.GAUGE);
		}
	}

	/**
	    \brief One-time registry fill.
	*/
	void Init(Transport transport)
	{
		if (!transport || m_Registry.Count() > 0)
			return;

		m_Transport = transport;

		m_Registry.Insert(m_Health);
		m_Registry.Insert(m_Passengers);
		m_Registry.Insert(m_SpeedMS);
		m_Registry.Insert(m_EngineOn);
		m_Registry.Insert(m_FuelFraction);

		if (MetricZ_Config.s_EnableCoordinatesMetrics) {
			m_Registry.Insert(m_PosX);
			m_Registry.Insert(m_PosY);
			m_Registry.Insert(m_PosZ);
		}

		SetLabels();
	}


	/**
	    \brief Update all metrics from the transport state.
	*/
	override void Update()
	{
		if (!m_Transport || m_Registry.Count() < 1)
			return;

		// health
		m_Health.Set(m_Transport.GetHealth01());

		// crew members
		m_Passengers.Set(GetPassengersCount());

		// position
		if (MetricZ_Config.s_EnableCoordinatesMetrics) {
			vector pos = m_Transport.GetPosition();
			m_PosX.Set(pos[0]);
			m_PosY.Set(pos[1]);
			m_PosZ.Set(pos[2]);
		}

		// speed m/s via physics velocity
		vector v = GetVelocity(m_Transport);
		float speed = Math.Sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
		if (speed < 0.10)
			speed = 0;
		m_SpeedMS.Set(speed);

		// engine + fuel where available
		m_EngineOn.Set(0);
		m_FuelFraction.Set(-1.0);

		CarScript car;
		if (Class.CastTo(car, m_Transport) && car.IsVitalFuelTank()) {
			m_EngineOn.Set(MetricZ_LabelUtils.Bool(car.EngineIsOn()));
			m_FuelFraction.Set(car.GetFluidFraction(CarFluid.FUEL));

			return;
		}

		BoatScript boat;
		if (Class.CastTo(boat, m_Transport) && boat.HasEngine()) {
			// preventing rolling on waves
			if (!boat.EngineIsOn() && speed < 0.30)
				speed = 0;
			m_SpeedMS.Set(speed);

			m_EngineOn.Set(MetricZ_LabelUtils.Bool(boat.EngineIsOn()));
			m_FuelFraction.Set(boat.GetFluidFraction(BoatFluid.FUEL));

			return;
		}

		// HelicopterScript heli;
		// if (Class.CastTo(heli, m_Transport)) {}
	}

	/**
	    \brief Build and cache transport label sets.
	*/
	override protected void SetLabels()
	{
		if (!m_Transport || m_Labels != string.Empty)
			return;

		map<string, string> labels = new map<string, string>();

		string cls = m_Transport.GetType();
		cls.TrimInPlace();

		string type = m_Transport.GetVehicleType();
		type.TrimInPlace();
		type.Replace("VehicleType", "");

		labels.Insert("class", cls);
		labels.Insert("type", type);
		labels.Insert("hash", MetricZ_LabelUtils.PersistentHash(m_Transport));

		m_Labels = MetricZ_LabelUtils.MakeLabels(labels);
	}

	/**
	    \brief Return number of occupied crew seats.
	*/
	private int GetPassengersCount()
	{
		if (!m_Transport)
			return 0;

		int size = m_Transport.CrewSize();
		int passengers = 0;
		for (int i = 0; i < size; i++) {
			if (m_Transport.CrewMember(i))
				passengers++;
		}

		return passengers;
	}
}
#endif
