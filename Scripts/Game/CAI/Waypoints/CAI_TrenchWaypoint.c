class CAI_TrenchWaypointClass : CAI_GarrisonWaypointClass
{
}

//! Waypoint which makes the group man the trenches inside its radius and stay there.
//! It derives from the Garrison waypoint, so SCR_AIGroup.CAI_IsGarrisoned covers it too.
class CAI_TrenchWaypoint : CAI_GarrisonWaypoint
{
	//------------------------------------------------------------------------------------------------
	override SCR_AIWaypointState CreateWaypointState(SCR_AIGroupUtilityComponent groupUtilityComp)
	{
		return new CAI_TrenchWaypointState(groupUtilityComp, this);
	}
}

//------------------------------------------------------------------------------------------------
//! Holds like the Garrison waypoint, but looks for spots in trenches instead of inside a building.
//! A spot counts as being in a trench if a soldier can walk there, the sky above it is open, and at chest height
//! something solid is close on two opposite sides. Terrain counts as solid, so dug trenches work as well as built ones.
//! Narrow gaps between other objects pass the same test.
class CAI_TrenchWaypointState : CAI_GarrisonWaypointState
{
	protected static const float TRENCH_GRID_STEP = 2.5;
	protected static const float MIN_SEARCH_RADIUS = 10;
	protected static const float MAX_SEARCH_RADIUS = 60;
	protected static const float CHEST_HEIGHT = 1;
	protected static const float WALL_DISTANCE = 2.2;	//!< A wall further away than this does not make a trench
	protected static const float SKY_HEIGHT = 4;		//!< Anything this close above the spot makes it a room, not a trench
	protected static const int WALL_DIRECTIONS = 4;		//!< Pairs of opposite directions tested, 45 degrees apart

	protected int m_iRejectedRoofed;
	protected int m_iRejectedNoWalls;

	//------------------------------------------------------------------------------------------------
	//! Grid of points on the ground inside the waypoint's radius
	override protected bool PrepareSamples()
	{
		vector center = m_Waypoint.GetOrigin();
		float radius = Math.Clamp(m_Waypoint.GetCompletionRadius(), MIN_SEARCH_RADIUS, MAX_SEARCH_RADIUS);
		BaseWorld world = GetGame().GetWorld();

		// Coarser grid if the area would need too many samples
		float step = TRENCH_GRID_STEP;
		while (Math.PI * radius * radius / (step * step) > MAX_SAMPLES)
		{
			step *= 1.25;
		}

		int count = Math.Ceil(radius / step);
		vector sample;
		for (int x = -count; x <= count; x++)
		{
			for (int z = -count; z <= count; z++)
			{
				float offsetX = x * step;
				float offsetZ = z * step;
				if (offsetX * offsetX + offsetZ * offsetZ > radius * radius)
					continue;

				sample[0] = center[0] + offsetX;
				sample[2] = center[2] + offsetZ;
				sample[1] = world.GetSurfaceY(sample[0], sample[2]);
				m_aSamples.Insert(sample);
			}
		}

		PrintFormat("%1%2: searching %3 m around the waypoint, %4 samples", LOG_PREFIX, Type(), radius, m_aSamples.Count());
		return !m_aSamples.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override protected bool IsPlaceValid()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void PrintRejections()
	{
		PrintFormat("%1rejected: %2 not on navmesh, %3 too close to another, %4 under a roof, %5 not between walls. Tile waits: %6", LOG_PREFIX,
			m_iRejectedNoNavmesh, m_iRejectedDuplicate, m_iRejectedRoofed, m_iRejectedNoWalls, m_iTileWaits);
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if something solid lies between the two points
	protected bool IsBlocked(BaseWorld world, TraceParam trace, vector from, vector to)
	{
		trace.Start = from;
		trace.End = to;
		return world.TraceMove(trace, null) < 0.999;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool TestSample(vector sample, out vector outPosition)
	{
		vector position;
		if (!m_Navmesh.GetReachablePoint(sample, NAVMESH_SNAP_RADIUS, position))
		{
			m_iRejectedNoNavmesh++;
			return false;
		}

		foreach (vector other : m_aPositions)
		{
			if (vector.DistanceSq(other, position) < MIN_SPACING_SQ)
			{
				m_iRejectedDuplicate++;
				return false;
			}
		}

		BaseWorld world = GetGame().GetWorld();
		TraceParam trace = new TraceParam();
		trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;

		vector chest = position + CHEST_HEIGHT * vector.Up;

		if (IsBlocked(world, trace, chest, chest + SKY_HEIGHT * vector.Up))
		{
			m_iRejectedRoofed++;
			return false;
		}

		for (int i = 0; i < WALL_DIRECTIONS; i++)
		{
			vector direction = vector.FromYaw(i * 180 / WALL_DIRECTIONS);
			if (IsBlocked(world, trace, chest, chest + WALL_DISTANCE * direction) && IsBlocked(world, trace, chest, chest - WALL_DISTANCE * direction))
			{
				outPosition = position;
				return true;
			}
		}

		m_iRejectedNoWalls++;
		return false;
	}
}
