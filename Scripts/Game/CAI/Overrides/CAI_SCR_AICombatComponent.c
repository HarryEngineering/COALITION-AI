modded class SCR_AICombatComponent : ScriptComponent
{
	protected ref CAI_Settings m_CAI_Settings;	//!< Settings this soldier currently uses, see CAI_Refresh
	protected int m_iCAI_Revision = -1;			//!< CAI_SettingsManager revision m_CAI_Settings was resolved at
	protected AIGroup m_CAI_Group;				//!< Group m_CAI_Settings was resolved for
	protected bool m_bCAI_Assaulting;			//!< Group is in a Search and Destroy assault, cached in CAI_Apply
	protected bool m_bCAI_SkillOverridden;

	// Rocket launcher against infantry, see CAI_EvaluateLauncher
	protected static const float CAI_LAUNCHER_MIN_DISTANCE = 40;		//!< Not closer than this, the shooter would be in the blast
	protected static const float CAI_LAUNCHER_MAX_DISTANCE = 300;
	protected static const float CAI_LAUNCHER_DURATION_MS = 20000;	//!< How long after the decision the launcher stays the weapon of choice
	protected static ref array<EWeaponType> s_aCAI_LauncherWhitelist = {EWeaponType.WT_ROCKETLAUNCHER};
	protected ref BaseTarget m_CAI_LauncherTarget;	//!< Infantry target the launcher decision below was made for
	protected bool m_bCAI_UseLauncher;
	protected float m_fCAI_LauncherEndTime_ms;		//!< World time

	//------------------------------------------------------------------------------------------------
	//! Settings this soldier currently uses. Never returns null.
	//! The cache is refreshed once per behavior evaluation, not here.
	CAI_Settings CAI_GetSettings()
	{
		if (!m_CAI_Settings)
			return CAI_SettingsManager.GetVanilla();

		return m_CAI_Settings;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true while this soldier's group is in a banzai charge (SCR_AIGroup.CAI_StartBanzai)
	bool CAI_GetBanzaiPosition(out vector outPosition)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(m_CAI_Group);
		if (!group || !group.CAI_IsBanzai())
			return false;

		outPosition = group.CAI_GetBanzaiPosition();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true while this soldier's group assaults all-out on a Search and Destroy waypoint (SCR_AIGroup.CAI_UpdateAssault)
	bool CAI_IsAssaulting()
	{
		return m_bCAI_Assaulting;
	}

	//------------------------------------------------------------------------------------------------
	//! Position this soldier must run straight at while fighting: a banzai charge position, or the centre of the
	//! Search and Destroy waypoint during an assault. Once within the arrival distance the soldier fights normally.
	//! \param[out] outSprint true for a banzai charge: sprint and don't aim. False for an assault: run and fire on the move.
	//! \return false if there is no such position
	bool CAI_GetChargePosition(out vector outPosition, out float outArrivalDistSq, out bool outSprint)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(m_CAI_Group);
		if (!group)
			return false;

		if (group.CAI_IsBanzai())
		{
			outPosition = group.CAI_GetBanzaiPosition();
			outArrivalDistSq = 100;
			outSprint = true;
			return true;
		}

		float radius;
		if (!group.CAI_GetAssaultArea(outPosition, radius))
			return false;

		radius = Math.Max(radius, 10);
		outArrivalDistSq = radius * radius;
		outSprint = false;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Looks the settings up again if they were edited or this soldier changed group
	protected void CAI_Refresh(SCR_AIThreatSystem threatSystem)
	{
		if (!m_Agent)
		{
			AIControlComponent controlComp = AIControlComponent.Cast(GetOwner().FindComponent(AIControlComponent));
			if (controlComp)
				m_Agent = SCR_ChimeraAIAgent.Cast(controlComp.GetAIAgent());
		}

		AIGroup group;
		if (m_Agent)
			group = m_Agent.GetParentGroup();

		int revision = CAI_SettingsManager.GetRevision();
		if (m_CAI_Settings && revision == m_iCAI_Revision && group == m_CAI_Group)
			return;

		m_iCAI_Revision = revision;
		m_CAI_Group = group;

		CAI_Settings groupOverride;
		SCR_AIGroup scrGroup = SCR_AIGroup.Cast(group);
		if (scrGroup)
			groupOverride = scrGroup.CAI_GetOverride();

		m_CAI_Settings = CAI_SettingsManager.Resolve(groupOverride);

		CAI_Apply(threatSystem);
	}

	//------------------------------------------------------------------------------------------------
	//! Pushes settings into the systems that keep their own copy of a value
	protected void CAI_Apply(SCR_AIThreatSystem threatSystem)
	{
		SCR_AIGroup scrGroup = SCR_AIGroup.Cast(m_CAI_Group);
		m_bCAI_Assaulting = scrGroup && scrGroup.CAI_IsAssaulting();

		// Skill. Only reset it if we were the ones who changed it, scenarios and other mods set skill too.
		EAISkill skill = m_CAI_Settings.m_eSkillOverride;
		if (skill != EAISkill.NONE)
		{
			SetAISkill(skill);
			m_bCAI_SkillOverridden = true;
		}
		else if (m_bCAI_SkillOverridden)
		{
			ResetAISkill();
			m_bCAI_SkillOverridden = false;
		}

		if (threatSystem)
		{
			// A soldier in a banzai charge or an assault can't be suppressed. Their start and end invalidate the settings, which gets us here.
			vector banzaiPos;
			if (m_bCAI_Assaulting || CAI_GetBanzaiPosition(banzaiPos))
				threatSystem.CAI_SetSuppressionScale(0);
			else
				threatSystem.CAI_SetSuppressionScale(m_CAI_Settings.m_fSuppressionScale);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called by SCR_AIUtilityComponent at every behavior evaluation
	override void UpdatePerceptionFactor(PerceptionComponent perceptionComp, SCR_AIThreatSystem threatSystem)
	{
		CAI_Refresh(threatSystem);

		float scale = m_CAI_Settings.m_fPerceptionScale;
		float suppressedFactor = m_CAI_Settings.m_fSuppressedPerception;

		// Nothing to change, leave it to vanilla
		if (scale == CAI_Settings.VANILLA_PERCEPTION && suppressedFactor == CAI_Settings.VANILLA_SUPPRESSED_PERCEPTION)
		{
			super.UpdatePerceptionFactor(perceptionComp, threatSystem);
			return;
		}

		// Same as vanilla, except for the configurable THREATENED factor and the overall scale
		float perceptionFactor;
		switch (threatSystem.GetState())
		{
			case EAIThreatState.SAFE:
				perceptionFactor = PERCEPTION_FACTOR_SAFE; break;
			case EAIThreatState.VIGILANT:
				perceptionFactor = PERCEPTION_FACTOR_VIGILANT; break;
			case EAIThreatState.ALERTED:
				perceptionFactor = PERCEPTION_FACTOR_ALERTED; break;
			case EAIThreatState.THREATENED:
				perceptionFactor = suppressedFactor; break;
		}

		perceptionFactor *= m_fEquipmentPerceptionFactor;
		perceptionFactor *= m_fPerceptionFactor;
		perceptionFactor *= scale;

		perceptionComp.SetPerceptionFactor(perceptionFactor);
	}

	//------------------------------------------------------------------------------------------------
	//! Vanilla keeps rocket launchers for vehicles. Here a soldier may decide, once per infantry target, to use his
	//! launcher on it. While that decision stands, the weapon vanilla just selected is replaced by the launcher.
	//! \return true if the launcher is now the selected weapon
	protected bool CAI_EvaluateLauncher()
	{
		BaseTarget target = GetCurrentTarget();
		if (!target || target.GetUnitType() != EAIUnitType.UnitType_Infantry || m_CurrentTurretController)
		{
			m_CAI_LauncherTarget = null;
			m_bCAI_UseLauncher = false;
			return false;
		}

		float worldTime_ms = GetGame().GetWorld().GetWorldTime();

		// New target, roll the dice once
		if (target != m_CAI_LauncherTarget)
		{
			m_CAI_LauncherTarget = target;
			m_bCAI_UseLauncher = Math.RandomInt(0, 100) < m_CAI_Settings.m_iLauncherVsInfantryChance;
			m_fCAI_LauncherEndTime_ms = worldTime_ms + CAI_LAUNCHER_DURATION_MS;
		}

		if (!m_bCAI_UseLauncher)
			return false;

		if (worldTime_ms > m_fCAI_LauncherEndTime_ms)
		{
			m_bCAI_UseLauncher = false;
			return false;
		}

		// Only at a target he sees, and far enough away. These can change, so the decision is kept.
		float distance = target.GetDistance();
		if (distance < CAI_LAUNCHER_MIN_DISTANCE || distance > CAI_LAUNCHER_MAX_DISTANCE || !IsTargetVisible(target))
			return false;

		// The selector only offers launchers against vehicles, so ask it for a weapon against a vehicle at this distance.
		// It also picks the rocket to load. No result means no launcher or no rockets left.
		bool found = m_WeaponTargetSelector.SelectWeaponAgainstUnitTypeAndDistance(EAIUnitType.UnitType_VehicleUnarmored, distance, true, false, s_aCAI_LauncherWhitelist);
		if (!found)
			found = m_WeaponTargetSelector.SelectWeaponAgainstUnitTypeAndDistance(EAIUnitType.UnitType_VehicleMedium, distance, true, false, s_aCAI_LauncherWhitelist);
		if (!found)
			found = m_WeaponTargetSelector.SelectWeaponAgainstUnitTypeAndDistance(EAIUnitType.UnitType_VehicleHeavy, distance, true, false, s_aCAI_LauncherWhitelist);

		BaseWeaponComponent weapon;
		BaseMagazineComponent magazine;
		int muzzleId;
		if (found)
			m_WeaponTargetSelector.GetSelectedWeapon(weapon, muzzleId, magazine);

		if (!weapon || weapon.GetWeaponType() != EWeaponType.WT_ROCKETLAUNCHER)
		{
			m_bCAI_UseLauncher = false;
			return false;
		}

		// Same bookkeeping vanilla does for the weapon it selects
		if (weapon != m_SelectedWeaponComp || muzzleId != m_iSelectedMuzzle)
		{
			array<BaseMuzzleComponent> muzzles = {};
			weapon.GetMuzzlesList(muzzles);
			if (muzzles.IsIndexValid(muzzleId))
				m_SelectedWeaponResource = m_ConfigComponent.GetTreeNameForWeaponType(weapon.GetWeaponType(), muzzles[muzzleId].GetMuzzleType());
			else
				m_SelectedWeaponResource = m_ConfigComponent.GetTreeNameForWeaponType(weapon.GetWeaponType(), 0);
		}

		m_SelectedWeaponComp = weapon;
		m_iSelectedMuzzle = muzzleId;
		m_SelectedMagazineComp = magazine;

		m_WeaponTargetSelector.GetSelectedWeaponProperties(m_fSelectedWeaponMinDist, m_fSelectedWeaponMaxDist, m_bSelectedWeaponDirectDamage);
		m_fSelectedWeaponMinDist = Math.Max(m_fSelectedWeaponMinDist, CAI_LAUNCHER_MIN_DISTANCE);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Vanilla re-evaluates target and weapon every 0.5 s. Pull the next evaluation closer if settings ask for it.
	override void EvaluateWeaponAndTarget(out bool outWeaponEvent, out bool outSelectedTargetChanged,
		out BaseTarget outPrevTarget, out BaseTarget outCurrentTarget,
		out bool outRetreatTargetChanged, out bool outCompartmentChanged)
	{
		// What we left selected last time, which may be the launcher. Vanilla is about to select again.
		BaseWeaponComponent prevWeapon = m_SelectedWeaponComp;
		BaseMagazineComponent prevMagazine = m_SelectedMagazineComp;
		int prevMuzzle = m_iSelectedMuzzle;
		float nextEvaluationBefore_ms = m_fNextWeaponTargetEvaluation_ms;

		// Grenades under fire: vanilla reads the threat inside the call below to decide whether frags are allowed
		SCR_AIThreatSystem threatSystem;
		if (m_Utility && m_CAI_Settings && m_CAI_Settings.m_bGrenadesUnderFire)
			threatSystem = m_Utility.m_ThreatSystem;

		if (threatSystem)
			threatSystem.CAI_SetGrenadeThreatCap(true);

		super.EvaluateWeaponAndTarget(outWeaponEvent, outSelectedTargetChanged, outPrevTarget, outCurrentTarget, outRetreatTargetChanged, outCompartmentChanged);

		if (threatSystem)
			threatSystem.CAI_SetGrenadeThreatCap(false);
		if (!m_CAI_Settings)
			return;

		// Vanilla only evaluates every 0.5 s and returns early otherwise, in which case there is nothing to adjust
		if (m_fNextWeaponTargetEvaluation_ms != nextEvaluationBefore_ms && CAI_EvaluateLauncher())
		{
			outWeaponEvent = m_SelectedWeaponComp != prevWeapon || m_iSelectedMuzzle != prevMuzzle || m_SelectedMagazineComp != prevMagazine;
		}
		float interval_ms = 1000 * m_CAI_Settings.m_fTargetEvalInterval;
		if (interval_ms >= WEAPON_TARGET_UPDATE_PERIOD_MS)
			return;

		float nextEvaluation_ms = GetGame().GetWorld().GetWorldTime() + Math.Max(interval_ms, 100);
		if (nextEvaluation_ms < m_fNextWeaponTargetEvaluation_ms)
			m_fNextWeaponTargetEvaluation_ms = nextEvaluation_ms;
	}
}
