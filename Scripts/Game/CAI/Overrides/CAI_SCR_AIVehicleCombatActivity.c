//! Group activity which makes a group with vehicles fight instead of driving on
modded class SCR_AIVehicleCombatActivity : SCR_AIActivityBase
{
	//------------------------------------------------------------------------------------------------
	//! Vanilla only stops for enemies within 200 m, or 550 m if they are shooting at the group.
	//! Enemies within the group's engage distance now count as well.
	override protected bool IsClusterDangerousEnough(notnull SCR_AIGroupTargetCluster c)
	{
		if (super.IsClusterDangerousEnough(c))
			return true;

		if (!c.m_State || !m_Utility || !m_Utility.m_Owner)
			return false;

		// Same age limit as vanilla
		if (c.m_State.GetTimeSinceLastNewInformation() > 20)
			return false;

		return c.m_State.m_fDistMin <= m_Utility.m_Owner.CAI_GetSettings().m_fVehicleEngageDistance;
	}
}
