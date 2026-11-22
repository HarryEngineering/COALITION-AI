//! Combat movement of a soldier who is attacking a target.
//! During a banzai charge (SCR_AIGroup.CAI_StartBanzai), and on the way into a Search and Destroy waypoint during an
//! assault (SCR_AIGroup.CAI_UpdateAssault), vanilla's cover-to-cover logic is replaced by a straight move at the
//! charge position. A banzai charge sprints and doesn't fire. An assault runs and aims at the target, so the
//! attack tree keeps firing while the soldier moves.
modded class SCR_AICombatMoveLogic_Attack : SCR_AICombatMoveLogicBase
{
	protected static const float CAI_ASSAULT_STOPPED_WAIT_S = 1.5;	//!< Longest an assaulting soldier stays put before pushing on
	protected static const float CAI_BANZAI_MOVE_DURATION_S = 8;	//!< Length of one leg of the run, a new one starts when it ends

	//------------------------------------------------------------------------------------------------
	override protected ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		// Same throttle as vanilla (0.5 s), checked first so nothing below runs every tick
		float currentTime_ms = GetGame().GetWorld().GetWorldTime();
		if (currentTime_ms < m_fNextUpdate_ms)
			return ENodeResult.RUNNING;

		vector banzaiPos;
		float arrivalDistSq;
		bool sprint;
		if (!m_CombatComp || !m_MyEntity || !m_CombatComp.CAI_GetChargePosition(banzaiPos, arrivalDistSq, sprint))
			return super.EOnTaskSimulate(owner, dt);

		if (vector.DistanceSqXZ(m_MyEntity.GetOrigin(), banzaiPos) < arrivalDistSq)
			return super.EOnTaskSimulate(owner, dt);

		m_fNextUpdate_ms = currentTime_ms + m_fUpdateInterval_ms;

		if (!OnUpdate(owner, dt))
			return ENodeResult.FAIL;

		if (!m_State || !m_Utility || !m_CharacterController)
			return ENodeResult.FAIL;

		// Combat movement is not used in turrets and vehicles
		SCR_AIBehaviorBase executedBehavior = SCR_AIBehaviorBase.Cast(m_Utility.GetExecutedAction());
		if (executedBehavior && !executedBehavior.m_bUseCombatMove)
			return ENodeResult.RUNNING;

		// Already running at the position
		if (m_State.IsExecutingRequest())
		{
			SCR_AICombatMoveRequest_Move currentRequest = SCR_AICombatMoveRequest_Move.Cast(m_State.GetRequest());
			if (currentRequest && !currentRequest.m_bTryFindCover && currentRequest.m_eDirection == SCR_EAICombatMoveDirection.CUSTOM_POS
				&& vector.DistanceSqXZ(currentRequest.m_vMovePos, banzaiPos) < 1)
				return ENodeResult.RUNNING;
		}

		SCR_AICombatMoveRequest_Move rq = new SCR_AICombatMoveRequest_Move();
		rq.m_eReason = SCR_EAICombatMoveReason.STANDARD;
		rq.m_vTargetPos = ResolveRequestTargetPos();
		rq.m_vMovePos = banzaiPos;
		rq.m_eDirection = SCR_EAICombatMoveDirection.CUSTOM_POS;
		rq.m_bTryFindCover = false;
		rq.m_eStanceMoving = ECharacterStance.STAND;
		rq.m_eStanceEnd = ECharacterStance.STAND;

		// Banzai charge: sprint, which rules out aiming. Assault: run, so the soldier can fire on the move.
		if (sprint)
			rq.m_eMovementType = EMovementType.SPRINT;
		else
			rq.m_eMovementType = EMovementType.RUN;

		rq.m_fMoveDuration_s = CAI_BANZAI_MOVE_DURATION_S;
		rq.m_bAimAtTarget = SCR_AICombatMoveUtils.IsAimingAndMovementPossible(rq.m_eStanceMoving, rq.m_eMovementType)
			&& IsAimingAndMovingAllowedForWeapon(m_CombatComp.GetSelectedWeaponType());
		rq.m_bAimAtTargetEnd = true;

		m_State.ApplyNewRequest(rq);

		return ENodeResult.RUNNING;
	}

	//------------------------------------------------------------------------------------------------
	//! How long a soldier stays where he is before moving closer to the target. Vanilla waits 3 to 40 s.
	override protected float ResolveStoppedWaitTime(bool inCover, EAIThreatState threat, EWeaponType weaponType)
	{
		float waitTime = super.ResolveStoppedWaitTime(inCover, threat, weaponType);

		if (!m_CombatComp)
			return waitTime;

		if (m_CombatComp.CAI_IsAssaulting())
			return Math.Min(waitTime, CAI_ASSAULT_STOPPED_WAIT_S);

		// A soldier who found cover makes use of it
		if (inCover)
			waitTime *= m_CombatComp.CAI_GetSettings().m_fCoverHoldScale;

		return waitTime;
	}
}