//! Combat movement of an armed vehicle. The gunner runs this and tells the driver where to go.
//! Vanilla backs away from targets closer than 60 m, closes in on targets beyond 200 m, and holds in between.
//! Here both distances come from the settings, and a vehicle which was told to rush drives at its target instead.
modded class SCR_AICombatMoveLogicVehicleGunner_Attack : SCR_AICombatMoveLogicVehicleGunnerBase
{
	protected static const float CAI_RUSH_MIN_DISTANCE = 10;	//!< A rushing vehicle only backs off when it is this close
	protected static const float CAI_RUSH_STOP_DISTANCE = 25;	//!< A rushing vehicle stops closing in at this distance
	protected static const float CAI_RUSH_WAIT_S = 0.5;			//!< Pause of a rushing vehicle between two legs of its approach
	protected static const float CAI_MIN_HOLD_BAND = 20;		//!< The engage distance is kept at least this far above the stand-off distance

	//------------------------------------------------------------------------------------------------
	//! \return true if the vehicle should drive at its target: the group's setting says so, or the group is on a
	//! banzai charge or a Search and Destroy assault
	protected bool CAI_IsRushing()
	{
		if (!m_CombatComp)
			return false;

		if (m_CombatComp.CAI_GetSettings().m_bVehicleRush || m_CombatComp.CAI_IsAssaulting())
			return true;

		vector banzaiPos;
		return m_CombatComp.CAI_GetBanzaiPosition(banzaiPos);
	}

	//------------------------------------------------------------------------------------------------
	//! Decides if the vehicle should reverse away from the target
	override protected bool MoveFromTargetCondition()
	{
		if (!m_WeaponManagerComponent || !m_CombatComp)
			return super.MoveFromTargetCondition();

		float minDistance;
		if (CAI_IsRushing())
			minDistance = CAI_RUSH_MIN_DISTANCE;
		else
			minDistance = m_CombatComp.CAI_GetSettings().m_fVehicleStandoffDistance;

		vector muzzleTransform[4];
		m_WeaponManagerComponent.GetCurrentMuzzleTransform(muzzleTransform);
		vector targetPos = m_Target.GetLastSeenPosition();

		if (vector.DistanceSq(targetPos, muzzleTransform[3]) < minDistance * minDistance)
			return true;

		// Vanilla: also move when the turret can't turn far enough to reach the target
		return !TargetWithinTurretSafeHorizontalLimits(targetPos);
	}

	//------------------------------------------------------------------------------------------------
	//! Decides if the vehicle should drive closer to the target
	override protected bool MoveToNextPosCondition()
	{
		if (!m_CombatComp)
			return super.MoveToNextPosCondition();

		bool rushing = CAI_IsRushing();

		float holdDistance;
		if (rushing)
		{
			holdDistance = CAI_RUSH_STOP_DISTANCE;
		}
		else
		{
			CAI_Settings settings = m_CombatComp.CAI_GetSettings();
			holdDistance = Math.Max(settings.m_fVehicleEngageDistance, settings.m_fVehicleStandoffDistance + CAI_MIN_HOLD_BAND);
		}

		// Close enough. Still move closer to a target which has not been seen for a while.
		if (m_fTargetDist < holdDistance && m_Target.GetTimeSinceSeen() < 15)
			return false;

		if (m_DriverState.IsExecutingRequest())
			return false;

		if (IsFirstExecution())
			return true;

		float stoppedWaitTime;
		if (rushing)
			stoppedWaitTime = CAI_RUSH_WAIT_S;
		else
			stoppedWaitTime = ResolveStoppedWaitTime(m_eThreatState);

		return m_DriverState.m_fTimerStopped_s > stoppedWaitTime;
	}
}
