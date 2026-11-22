class CAI_ClearBuildingWaypointClass : SCR_AIWaypointClass
{
}

//! Waypoint which makes the group clear the building it is placed on.
//! It has no behaviour tree. Like the vanilla Artillery Support waypoint, all the work is done by its waypoint
//! state, which exists while this is the group's current waypoint.
class CAI_ClearBuildingWaypoint : SCR_AIWaypoint
{
	//------------------------------------------------------------------------------------------------
	override SCR_AIWaypointState CreateWaypointState(SCR_AIGroupUtilityComponent groupUtilityComp)
	{
		return new CAI_ClearBuildingWaypointState(groupUtilityComp, this);
	}
}

//------------------------------------------------------------------------------------------------
//! Clears a building in three steps:
//! 1. Finds the building nearest to the waypoint.
//! 2. Samples its volume on a grid, floor by floor, and keeps the points which are on the soldier navmesh,
//!    on a floor of that building and under its roof. The sampling is spread over time, a few points per step.
//! 3. Deals the points out to the soldiers. Each soldier walks to his points one after another and looks around
//!    at each. When nobody has points left, the waypoint completes.
//! Soldiers who meet an enemy fight him as usual and carry on afterwards.
class CAI_ClearBuildingWaypointState : SCR_AIWaypointState
{
	protected static const float BUILDING_SEARCH_RADIUS = 25;	//!< Metres around the waypoint in which the building is looked for
	protected static const float GRID_STEP = 3.5;				//!< Horizontal distance between sample points
	protected static const float FLOOR_STEP = 2.8;				//!< Height of one storey, used to order positions by storey
	protected static const float LAYER_STEP = 1.4;				//!< Vertical distance between sample layers. Half a storey, so no floor falls between two layers.
	protected static const int MAX_SAMPLES = 600;				//!< The grid is made coarser for buildings which would need more
	protected static const float NAVMESH_SNAP_RADIUS = 2;		//!< How far a sample may be from walkable floor
	protected static const float MAX_SNAP_HEIGHT = 1.1;			//!< A point snapped further up or down than this belongs to another layer
	protected static const float MIN_SPACING_SQ = 6.25;			//!< Points closer than this (2.5 m, squared) count as the same spot
	protected static const int MAX_POSITIONS = 48;
	protected static const int SAMPLES_PER_STEP = 10;
	protected static const int QUERY_STEP_MS = 50;
	protected static const int MAX_TILE_WAITS = 60;				//!< Query steps (3 s) spent in total waiting for navmesh tiles, after that samples are tested regardless
	protected static const string LOG_PREFIX = "[COALITION AI] ";
	protected static const int UPDATE_STEP_MS = 1000;
	protected static const float POSITION_RADIUS = 2;			//!< A soldier has reached a point within this distance
	protected static const float LOOK_DURATION_S = 3;			//!< Time a soldier looks around at each point
	protected static const float TIMEOUT_MS = 300000;			//!< The waypoint completes after this time whatever happened

	protected bool m_bRunning;
	protected float m_fEndTime_ms; //!< World time

	// Building search
	protected IEntity m_Building;
	protected float m_fBuildingDistSq;
	protected vector m_vSearchPos;

	// Sampling
	protected NavmeshWorldComponent m_Navmesh;
	protected ref array<vector> m_aSamples = {};
	protected int m_iNextSample;
	protected int m_iTileWaits;

	// Why samples were rejected, printed to the log once sampling is done
	protected int m_iRejectedNoNavmesh;
	protected int m_iRejectedOtherLayer;
	protected int m_iRejectedDuplicate;
	protected int m_iRejectedFloor;
	protected int m_iRejectedNoRoof;
	protected ref array<vector> m_aPositions = {};

	// Clearing. The three arrays are parallel, one entry per soldier.
	protected ref array<AIAgent> m_aAgents = {};
	protected ref array<ref array<vector>> m_aQueues = {};
	protected ref array<ref SCR_AIMoveAndInvestigateBehavior> m_aBehaviors = {};

	//------------------------------------------------------------------------------------------------
	override void OnSelected()
	{
		super.OnSelected();

		if (!m_Waypoint || !m_Utility || !m_Utility.m_Owner)
			return;

		m_bRunning = true;
		m_fEndTime_ms = GetGame().GetWorld().GetWorldTime() + TIMEOUT_MS;

		AIWorld aiWorld = GetGame().GetAIWorld();
		if (aiWorld)
			m_Navmesh = aiWorld.GetNavmeshWorldComponent("Soldiers");

		if (!m_Navmesh || !PrepareSamples())
		{
			PrintFormat("%1%2: nothing to search here (navmesh found: %3)", LOG_PREFIX, Type(), m_Navmesh != null);
			Finish();
			return;
		}

		GetGame().GetCallqueue().CallLater(QueryStep, QUERY_STEP_MS, true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDeselected()
	{
		Stop();
		super.OnDeselected();
	}

	//------------------------------------------------------------------------------------------------
	void ~CAI_ClearBuildingWaypointState()
	{
		Stop();
	}

	//------------------------------------------------------------------------------------------------
	//! Ends all timers and takes back the orders soldiers have not finished
	protected void Stop()
	{
		if (!m_bRunning)
			return;

		m_bRunning = false;

		// The game can already be gone when this runs from the destructor
		ArmaReforgerScripted game = GetGame();
		if (game && game.GetCallqueue())
		{
			game.GetCallqueue().Remove(QueryStep);
			game.GetCallqueue().Remove(UpdateStep);
		}

		foreach (SCR_AIMoveAndInvestigateBehavior behavior : m_aBehaviors)
		{
			if (!behavior)
				continue;

			EAIActionState state = behavior.GetActionState();
			if (state != EAIActionState.COMPLETED && state != EAIActionState.FAILED)
				behavior.Fail();
		}

		m_aBehaviors.Clear();
		m_aQueues.Clear();
		m_aAgents.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Completes the waypoint, the group goes on to its next one
	protected void Finish()
	{
		Stop();

		if (m_Utility && m_Utility.m_Owner && m_Waypoint)
			m_Utility.m_Owner.CompleteWaypoint(m_Waypoint);
	}

	//------------------------------------------------------------------------------------------------
	// The three methods below are what a waypoint for another kind of place replaces, together with TestSample

	//------------------------------------------------------------------------------------------------
	//! Finds the place and fills m_aSamples with the points to test.
	//! \return false if there is nothing to search
	protected bool PrepareSamples()
	{
		FindBuilding();
		if (!m_Building)
		{
			PrintFormat("%1%2: no building within %3 m of the waypoint", LOG_PREFIX, Type(), BUILDING_SEARCH_RADIUS);
			return false;
		}

		CreateSamples();

		vector boundsMin, boundsMax;
		m_Building.GetBounds(boundsMin, boundsMax);
		string prefabName;
		if (m_Building.GetPrefabData())
			prefabName = m_Building.GetPrefabData().GetPrefabName();
		PrintFormat("%1%2: building %3, size %4, %5 samples", LOG_PREFIX, Type(), prefabName, boundsMax - boundsMin, m_aSamples.Count());

		return !m_aSamples.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! \return false if the place is gone and the search should end
	protected bool IsPlaceValid()
	{
		return m_Building != null;
	}

	//------------------------------------------------------------------------------------------------
	protected void PrintRejections()
	{
		PrintFormat("%1rejected: %2 not on navmesh, %3 other layer, %4 too close to another, %5 floor of something else, %6 no roof. Tile waits: %7", LOG_PREFIX,
			m_iRejectedNoNavmesh, m_iRejectedOtherLayer, m_iRejectedDuplicate, m_iRejectedFloor, m_iRejectedNoRoof, m_iTileWaits);
	}

	//------------------------------------------------------------------------------------------------
	// Step 1: building
	//------------------------------------------------------------------------------------------------
	protected void FindBuilding()
	{
		m_vSearchPos = m_Waypoint.GetOrigin();
		m_Building = null;
		m_fBuildingDistSq = float.MAX;
		GetGame().GetWorld().QueryEntitiesBySphere(m_vSearchPos, BUILDING_SEARCH_RADIUS, QueryBuilding, null, EQueryEntitiesFlags.ALL);
	}

	//------------------------------------------------------------------------------------------------
	//! Keeps the building whose bounding box is nearest to the waypoint. A waypoint inside the box has distance 0.
	protected bool QueryBuilding(IEntity entity)
	{
		if (!Building.Cast(entity))
			return true;

		vector mins, maxs;
		entity.GetWorldBounds(mins, maxs);

		float distSq;
		for (int i = 0; i < 3; i++)
		{
			float d = Math.Max(Math.Max(mins[i] - m_vSearchPos[i], m_vSearchPos[i] - maxs[i]), 0);
			distSq += d * d;
		}

		if (distSq < m_fBuildingDistSq)
		{
			m_fBuildingDistSq = distSq;
			m_Building = entity;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Step 2: positions inside the building

	//------------------------------------------------------------------------------------------------
	//! Grid of points through the building's bounding box, in world space, lowest layer first
	protected void CreateSamples()
	{
		vector mins, maxs;
		m_Building.GetBounds(mins, maxs);

		float step = GRID_STEP;
		int countX, countY, countZ;
		while (true)
		{
			countX = Math.Ceil((maxs[0] - mins[0]) / step);
			countZ = Math.Ceil((maxs[2] - mins[2]) / step);
			countY = Math.Ceil((maxs[1] - mins[1]) / LAYER_STEP);
			countX = Math.Max(countX, 1);
			countY = Math.Max(countY, 1);
			countZ = Math.Max(countZ, 1);

			if (countX * countY * countZ <= MAX_SAMPLES)
				break;

			step *= 1.25;
		}

		vector local;
		for (int y = 0; y < countY; y++)
		{
			local[1] = mins[1] + 0.5 + y * LAYER_STEP;
			for (int x = 0; x < countX; x++)
			{
				local[0] = mins[0] + (x + 0.5) * step;
				for (int z = 0; z < countZ; z++)
				{
					local[2] = mins[2] + (z + 0.5) * step;
					m_aSamples.Insert(m_Building.CoordToParent(local));
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Runs every QUERY_STEP_MS until all samples are tested
	protected void QueryStep()
	{
		if (!m_bRunning)
			return;

		if (!IsPlaceValid() || GetGame().GetWorld().GetWorldTime() > m_fEndTime_ms)
		{
			Finish();
			return;
		}

		for (int i = 0; i < SAMPLES_PER_STEP; i++)
		{
			if (m_iNextSample >= m_aSamples.Count() || m_aPositions.Count() >= MAX_POSITIONS)
				break;

			vector sample = m_aSamples[m_iNextSample];

			// Navmesh tiles are streamed, a tile which is not there yet is requested and the sample retried next step.
			// The waiting has one budget for the whole building, so a tile which never reports as loaded can't stall it.
			if (m_iTileWaits < MAX_TILE_WAITS && !m_Navmesh.IsTileLoaded(sample))
			{
				if (!m_Navmesh.IsTileRequested(sample))
					m_Navmesh.LoadTileIn(sample);

				m_iTileWaits++;
				return;
			}

			m_iNextSample++;

			vector position;
			if (TestSample(sample, position))
				m_aPositions.Insert(position);
		}

		if (m_iNextSample < m_aSamples.Count() && m_aPositions.Count() < MAX_POSITIONS)
			return;

		// Sampling is done
		GetGame().GetCallqueue().Remove(QueryStep);

		PrintFormat("%1%2: %3 positions from %4 samples", LOG_PREFIX, Type(), m_aPositions.Count(), m_aSamples.Count());
		PrintRejections();

		m_aSamples.Clear();

		if (m_aPositions.IsEmpty() || !StartWork())
		{
			PrintFormat("%1%2: nothing to do, completing the waypoint", LOG_PREFIX, Type());
			Finish();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called once the positions inside the building are known. The Garrison waypoint does something else with them.
	//! \return false if nothing can be done, which completes the waypoint
	protected bool StartWork()
	{
		if (!AssignPositions())
			return false;

		GetGame().GetCallqueue().CallLater(UpdateStep, UPDATE_STEP_MS, true);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if there is a new spot worth visiting near the sample
	protected bool TestSample(vector sample, out vector outPosition)
	{
		vector position;
		if (!m_Navmesh.GetReachablePoint(sample, NAVMESH_SNAP_RADIUS, position))
		{
			m_iRejectedNoNavmesh++;
			return false;
		}

		// Snapped to the floor above or below, which has its own samples
		if (Math.AbsFloat(position[1] - sample[1]) > MAX_SNAP_HEIGHT)
		{
			m_iRejectedOtherLayer++;
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

		IEntity buildingRoot = m_Building.GetRootParent();
		BaseWorld world = GetGame().GetWorld();

		// Something of this building must be above the point, otherwise it is on a roof, a balcony or outside
		TraceParam trace = new TraceParam();
		trace.Flags = TraceFlags.ENTS;
		trace.Start = position + 1.5 * vector.Up;
		trace.End = position + 20 * vector.Up;
		if (world.TraceMove(trace, null) >= 0.999 || !trace.TraceEnt || trace.TraceEnt.GetRootParent() != buildingRoot)
		{
			m_iRejectedNoRoof++;
			return false;
		}

		// The floor under the point must not belong to something else. No entity at all is fine: some ground floors are bare terrain.
		trace.TraceEnt = null;
		trace.End = position - vector.Up;
		if (world.TraceMove(trace, null) < 0.999 && trace.TraceEnt && trace.TraceEnt.GetRootParent() != buildingRoot)
		{
			m_iRejectedFloor++;
			return false;
		}

		outPosition = position;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Step 3: clearing

	//------------------------------------------------------------------------------------------------
	//! Deals the positions out to the soldiers on foot, lower storeys and nearer points first
	//! \return false if nobody can clear
	protected bool AssignPositions()
	{
		SCR_AIGroup group = m_Utility.m_Owner;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!GetUtility(agent))
				continue;

			m_aAgents.Insert(agent);
			m_aQueues.Insert(new array<vector>());
			m_aBehaviors.Insert(null);
		}

		int agentCount = m_aAgents.Count();
		PrintFormat("%1%2: %3 soldiers on foot", LOG_PREFIX, Type(), agentCount);
		if (agentCount == 0)
			return false;

		// Sort key: storey first, then distance from the group
		vector groupPos = group.GetCenterOfMass();
		float baseY = m_aPositions[0][1];
		foreach (vector position : m_aPositions)
		{
			baseY = Math.Min(baseY, position[1]);
		}

		array<float> keys = {};
		foreach (vector keyPosition : m_aPositions)
		{
			int storey = Math.Round((keyPosition[1] - baseY) / FLOOR_STEP);
			keys.Insert(storey * 10000 + vector.DistanceXZ(keyPosition, groupPos));
		}

		// Selection sort, at most MAX_POSITIONS entries and it runs once
		int count = m_aPositions.Count();
		for (int i = 0; i < count; i++)
		{
			int best = i;
			for (int j = i + 1; j < count; j++)
			{
				if (keys[j] < keys[best])
					best = j;
			}

			if (best != i)
			{
				keys.SwapItems(i, best);
				m_aPositions.SwapItems(i, best);
			}

			m_aQueues[i % agentCount].Insert(m_aPositions[i]);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return The soldier's utility component if he is alive and on foot
	protected SCR_AIUtilityComponent GetUtility(AIAgent agent)
	{
		SCR_ChimeraAIAgent chimeraAgent = SCR_ChimeraAIAgent.Cast(agent);
		if (!chimeraAgent || !chimeraAgent.m_UtilityComponent)
			return null;

		ChimeraCharacter character = ChimeraCharacter.Cast(chimeraAgent.GetControlledEntity());
		if (!character)
			return null;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller || controller.GetLifeState() != ECharacterLifeState.ALIVE)
			return null;

		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (compartmentAccess && compartmentAccess.IsInCompartment())
			return null;

		return chimeraAgent.m_UtilityComponent;
	}

	//------------------------------------------------------------------------------------------------
	//! Runs every UPDATE_STEP_MS: gives each soldier his next point once he is done with the previous one
	protected void UpdateStep()
	{
		if (!m_bRunning)
			return;

		if (GetGame().GetWorld().GetWorldTime() > m_fEndTime_ms)
		{
			Finish();
			return;
		}

		bool anythingLeft;
		int agentCount = m_aAgents.Count();
		for (int i = 0; i < agentCount; i++)
		{
			array<vector> queue = m_aQueues[i];
			SCR_AIUtilityComponent utility = GetUtility(m_aAgents[i]);

			// Dead, unconscious or gone: the next soldier takes over his remaining points
			if (!utility)
			{
				m_aBehaviors[i] = null;
				if (!queue.IsEmpty() && agentCount > 1)
				{
					array<vector> nextQueue = m_aQueues[(i + 1) % agentCount];
					foreach (vector leftover : queue)
					{
						nextQueue.Insert(leftover);
					}
					queue.Clear();
				}
				continue;
			}

			SCR_AIMoveAndInvestigateBehavior behavior = m_aBehaviors[i];
			if (behavior)
			{
				EAIActionState state = behavior.GetActionState();
				if (state != EAIActionState.COMPLETED && state != EAIActionState.FAILED)
				{
					anythingLeft = true;
					continue;
				}

				m_aBehaviors[i] = null;
			}

			if (queue.IsEmpty())
				continue;

			vector target = queue[0];
			queue.RemoveOrdered(0);

			behavior = new SCR_AIMoveAndInvestigateBehavior(utility, null, target, radius: POSITION_RADIUS, isDangerous: true, duration: LOOK_DURATION_S);
			utility.AddAction(behavior);
			m_aBehaviors[i] = behavior;
			anythingLeft = true;
		}

		if (!anythingLeft)
			Finish();
	}
}
