// Game Master attributes for COALITION AI settings.
//
// The same attributes show up in two dialogs:
// - game mode attributes: edit the mission-wide settings
// - AI group attributes: edit that group's override. They stay disabled until "Custom AI settings" is ticked,
//   until then they show the mission-wide values the group is using.
//
// The attribute manager finds attributes by class, so every setting needs a class of its own.
// They are listed in Configs/Editor/AttributeLists/Edit.conf.

//------------------------------------------------------------------------------------------------
class CAI_EditorAttributeHelper
{
	//------------------------------------------------------------------------------------------------
	static SCR_AIGroup GetGroup(Managed item)
	{
		SCR_EditableGroupComponent editableGroup = SCR_EditableGroupComponent.Cast(item);
		if (!editableGroup)
			return null;

		return SCR_AIGroup.Cast(editableGroup.GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	//! \param forWriting when true, a group without an override gets one
	//! \return Settings edited through the item, or null if the item is neither the game mode nor an AI group
	static CAI_Settings GetSettings(Managed item, bool forWriting)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(item);
		if (gameMode)
			return gameMode.CAI_GetSettings();

		SCR_AIGroup group = GetGroup(item);
		if (!group)
			return null;

		if (forWriting)
			return group.CAI_CreateOverride();

		CAI_Settings groupOverride = group.CAI_GetOverride();
		if (groupOverride)
			return groupOverride;

		return CAI_SettingsManager.GetGlobal();
	}

	//------------------------------------------------------------------------------------------------
	//! Greys out attributes which have no effect with the current state of the three checkboxes.
	//! Called from the checkbox which changed, with its new value.
	static void UpdateEnabledStates(SCR_AttributesManagerEditorComponent manager, typename changedType, SCR_BaseEditorAttributeVar changedVar)
	{
		if (!manager)
			return;

		bool overrideOn = IsToggleOn(manager, CAI_GroupOverrideEditorAttribute, changedType, changedVar);
		bool modOn = overrideOn && IsToggleOn(manager, CAI_EnabledEditorAttribute, changedType, changedVar);
		bool shareOn = modOn && IsToggleOn(manager, CAI_ShareTargetsEditorAttribute, changedType, changedVar);
		bool artilleryOn = modOn && IsToggleOn(manager, CAI_ArtilleryEditorAttribute, changedType, changedVar);
		bool huntOn = modOn && IsToggleOn(manager, CAI_HuntEditorAttribute, changedType, changedVar);

		manager.SetAttributeEnabled(CAI_EnabledEditorAttribute, overrideOn);

		manager.SetAttributeEnabled(CAI_SkillEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_AimErrorEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_NightAimPenaltyEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_FireTempoEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_PerceptionEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_SuppressedPerceptionEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_SuppressionEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_TargetEvalIntervalEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_DecisionDelayEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_ShareTargetsEditorAttribute, modOn);

		manager.SetAttributeEnabled(CAI_ShareRangeEditorAttribute, shareOn);
		manager.SetAttributeEnabled(CAI_ShareIntervalEditorAttribute, shareOn);

		manager.SetAttributeEnabled(CAI_ArtilleryEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_ArtilleryRangeEditorAttribute, artilleryOn);
		manager.SetAttributeEnabled(CAI_ArtilleryRoundsEditorAttribute, artilleryOn);
		manager.SetAttributeEnabled(CAI_ArtilleryCooldownEditorAttribute, artilleryOn);
		manager.SetAttributeEnabled(CAI_ArtillerySafeDistanceEditorAttribute, artilleryOn);
		manager.SetAttributeEnabled(CAI_ArtilleryMinTargetsEditorAttribute, artilleryOn);

		manager.SetAttributeEnabled(CAI_HuntEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_AssaultSearchAndDestroyEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_LauncherChanceEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_AutoFireRangeEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_RifleBurstEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_MachineGunBurstEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_PresetEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_BreakCasualtiesEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_FallbackDistanceEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_StaticWeaponsEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_StaticWeaponRadiusEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_GrenadesUnderFireEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_SmokeBeforeChargeEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_DismountEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_DismountDistanceEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_VehicleRetreatDamageEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_VehicleStandoffEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_VehicleEngageEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_VehicleRushEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_RemountEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_BailOutEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_CoverAngleEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_CoverMinHeightEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_CoverSearchEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_CoverHoldEditorAttribute, modOn);
		manager.SetAttributeEnabled(CAI_HuntRadiusEditorAttribute, huntOn);
	}

	//------------------------------------------------------------------------------------------------
	//! A checkbox which is not part of the open dialog counts as ticked
	protected static bool IsToggleOn(SCR_AttributesManagerEditorComponent manager, typename type, typename changedType, SCR_BaseEditorAttributeVar changedVar)
	{
		if (type == changedType)
			return changedVar && changedVar.GetBool();

		// The return value is false for server attributes even when the attribute was found, so only the out value counts
		SCR_BaseEditorAttribute attribute;
		manager.GetActiveAttribute(type, attribute);
		if (!attribute)
			return true;

		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		return var && var.GetBool();
	}
}

//------------------------------------------------------------------------------------------------
//! Slider editing one float of CAI_Settings
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_BaseSliderEditorAttribute : SCR_BaseValueListEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	//! To be overridden by the attribute of each setting
	protected ECAI_Setting GetSetting()
	{
		return ECAI_Setting.AIM_ERROR;
	}

	//------------------------------------------------------------------------------------------------
	//! Not saved with Game Master sessions: loading would write a value to every group and so give each one an override
	override bool IsSerializable()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		CAI_Settings settings = CAI_EditorAttributeHelper.GetSettings(item, false);
		if (!settings)
			return null;

		return SCR_BaseEditorAttributeVar.CreateFloat(settings.GetValue(GetSetting()));
	}

	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;

		CAI_Settings settings = CAI_EditorAttributeHelper.GetSettings(item, true);
		if (!settings)
			return;

		settings.SetValue(GetSetting(), var.GetFloat());
		CAI_SettingsManager.Invalidate();
	}
}

//------------------------------------------------------------------------------------------------
//! Checkbox editing one bool of CAI_Settings
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_BaseCheckboxEditorAttribute : SCR_BaseEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	//! To be overridden by the attribute of each setting
	protected ECAI_Setting GetSetting()
	{
		return ECAI_Setting.ENABLED;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsSerializable()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		CAI_Settings settings = CAI_EditorAttributeHelper.GetSettings(item, false);
		if (!settings)
			return null;

		return SCR_BaseEditorAttributeVar.CreateBool(settings.GetValue(GetSetting()) != 0);
	}

	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;

		CAI_Settings settings = CAI_EditorAttributeHelper.GetSettings(item, true);
		if (!settings)
			return;

		float value = 0;
		if (var.GetBool())
			value = 1;

		settings.SetValue(GetSetting(), value);
		CAI_SettingsManager.Invalidate();
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{
		CAI_EditorAttributeHelper.UpdateEnabledStates(manager, Type(), var);
	}
}

//------------------------------------------------------------------------------------------------
//! Group only: whether the group has settings of its own
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_GroupOverrideEditorAttribute : SCR_BaseEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override bool IsSerializable()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_AIGroup group = CAI_EditorAttributeHelper.GetGroup(item);
		if (!group)
			return null;

		return SCR_BaseEditorAttributeVar.CreateBool(group.CAI_GetOverride() != null);
	}

	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;

		SCR_AIGroup group = CAI_EditorAttributeHelper.GetGroup(item);
		if (!group)
			return;

		if (var.GetBool())
			group.CAI_CreateOverride();
		else
			group.CAI_ClearOverride();
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{
		CAI_EditorAttributeHelper.UpdateEnabledStates(manager, Type(), var);
	}
}

//------------------------------------------------------------------------------------------------
//! Dropdown of forced skill levels. The entries and their EAISkill values are defined in Edit.conf.
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_SkillEditorAttribute : SCR_BaseFloatValueHolderEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override bool IsSerializable()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		CAI_Settings settings = CAI_EditorAttributeHelper.GetSettings(item, false);
		if (!settings || !m_aValues)
			return null;

		int index = ConvertValueToIndex(settings.GetValue(ECAI_Setting.SKILL));
		if (index == -1)
			return null;

		return SCR_BaseEditorAttributeVar.CreateInt(index);
	}

	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var || !m_aValues)
			return;

		float skill;
		if (!ConvertIndexToValue(var.GetInt(), skill))
			return;

		CAI_Settings settings = CAI_EditorAttributeHelper.GetSettings(item, true);
		if (!settings)
			return;

		settings.SetValue(ECAI_Setting.SKILL, skill);
		CAI_SettingsManager.Invalidate();
	}
}

//------------------------------------------------------------------------------------------------
// One class per setting

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_EnabledEditorAttribute : CAI_BaseCheckboxEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.ENABLED; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_ShareTargetsEditorAttribute : CAI_BaseCheckboxEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.SHARE_TARGETS; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_AimErrorEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.AIM_ERROR; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_NightAimPenaltyEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.NIGHT_AIM_PENALTY; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_FireTempoEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.FIRE_TEMPO; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_PerceptionEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.PERCEPTION; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_SuppressedPerceptionEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.SUPPRESSED_PERCEPTION; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_SuppressionEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.SUPPRESSION; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_TargetEvalIntervalEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.TARGET_EVAL_INTERVAL; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_DecisionDelayEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.DECISION_DELAY; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_ShareRangeEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.SHARE_RANGE; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_ShareIntervalEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.SHARE_INTERVAL; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_ArtilleryEditorAttribute : CAI_BaseCheckboxEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.ARTILLERY; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_HuntEditorAttribute : CAI_BaseCheckboxEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.HUNT; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_ArtilleryRangeEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.ARTILLERY_RANGE; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_ArtilleryRoundsEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.ARTILLERY_ROUNDS; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_ArtilleryCooldownEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.ARTILLERY_COOLDOWN; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_ArtillerySafeDistanceEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.ARTILLERY_SAFE_DISTANCE; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_ArtilleryMinTargetsEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.ARTILLERY_MIN_TARGETS; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_HuntRadiusEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.HUNT_RADIUS; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_AssaultSearchAndDestroyEditorAttribute : CAI_BaseCheckboxEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.ASSAULT_SND; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_LauncherChanceEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.LAUNCHER_CHANCE; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_AutoFireRangeEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.AUTO_FIRE_RANGE; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_RifleBurstEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.RIFLE_BURST; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_MachineGunBurstEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.MG_BURST; }
}

//------------------------------------------------------------------------------------------------
//! Dropdown of skill presets. The entries and their ECAI_Preset values are defined in Edit.conf.
//! It is listed before the sliders there, so a slider changed in the same dialog is written after the preset.
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_PresetEditorAttribute : SCR_BaseFloatValueHolderEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override bool IsSerializable()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		CAI_Settings settings = CAI_EditorAttributeHelper.GetSettings(item, false);
		if (!settings || !m_aValues)
			return null;

		int index = ConvertValueToIndex(settings.GetValue(ECAI_Setting.PRESET));
		if (index == -1)
			return null;

		return SCR_BaseEditorAttributeVar.CreateInt(index);
	}

	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var || !m_aValues)
			return;

		float preset;
		if (!ConvertIndexToValue(var.GetInt(), preset))
			return;

		CAI_Settings settings = CAI_EditorAttributeHelper.GetSettings(item, true);
		if (!settings)
			return;

		settings.SetValue(ECAI_Setting.PRESET, preset);
		CAI_SettingsManager.Invalidate();
	}
}
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_BreakCasualtiesEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.BREAK_CASUALTIES; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_FallbackDistanceEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.FALLBACK_DISTANCE; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_StaticWeaponsEditorAttribute : CAI_BaseCheckboxEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.STATIC_WEAPONS; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_StaticWeaponRadiusEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.STATIC_WEAPON_RADIUS; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_GrenadesUnderFireEditorAttribute : CAI_BaseCheckboxEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.GRENADES_UNDER_FIRE; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_SmokeBeforeChargeEditorAttribute : CAI_BaseCheckboxEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.SMOKE_BEFORE_CHARGE; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_DismountEditorAttribute : CAI_BaseCheckboxEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.DISMOUNT; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_DismountDistanceEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.DISMOUNT_DISTANCE; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_VehicleRetreatDamageEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.VEHICLE_RETREAT_DAMAGE; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_VehicleStandoffEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.VEHICLE_STANDOFF; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_VehicleEngageEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.VEHICLE_ENGAGE; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_VehicleRushEditorAttribute : CAI_BaseCheckboxEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.VEHICLE_RUSH; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_RemountEditorAttribute : CAI_BaseCheckboxEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.REMOUNT; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_BailOutEditorAttribute : CAI_BaseCheckboxEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.BAIL_OUT; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_CoverAngleEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.COVER_ANGLE; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_CoverMinHeightEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.COVER_MIN_HEIGHT; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_CoverSearchEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.COVER_SEARCH; }
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class CAI_CoverHoldEditorAttribute : CAI_BaseSliderEditorAttribute
{
	override protected ECAI_Setting GetSetting() { return ECAI_Setting.COVER_HOLD; }
}
