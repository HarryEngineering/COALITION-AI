//! Builds the cover search of a soldier who moves during a fight
modded class SCR_AICalculateCoverQueryProps_CombatMove : AITaskScripted
{
	protected SCR_AICombatComponent m_CAI_CombatComp;

	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		super.OnInit(owner);

		IEntity entity = owner.GetControlledEntity();
		if (entity)
			m_CAI_CombatComp = SCR_AICombatComponent.Cast(entity.FindComponent(SCR_AICombatComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! Vanilla fills m_CoverQueryProps and hands that same object to the cover search, so it can be adjusted afterwards:
	//! how far to look, how low a cover may be, and how squarely it must face the enemy.
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		ENodeResult result = super.EOnTaskSimulate(owner, dt);

		if (result != ENodeResult.SUCCESS || !m_CAI_CombatComp)
			return result;

		CAI_Settings settings = m_CAI_CombatComp.CAI_GetSettings();

		// Vanilla sets the distance from the move request on every call, so this does not add up
		m_CoverQueryProps.m_fSectorDistMax = m_CoverQueryProps.m_fSectorDistMax * settings.m_fCoverSearchScale;
		m_CoverQueryProps.m_fCoverHeightMin = settings.m_fCoverMinHeight;

		if (settings.m_fCoverMaxAngle != CAI_Settings.VANILLA_COVER_ANGLE)
			m_CoverQueryProps.m_fCoverToThreatAngleCosMin = Math.Cos(Math.DEG2RAD * settings.m_fCoverMaxAngle);

		return result;
	}
}
