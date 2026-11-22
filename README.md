# COALITION AI
An Arma Reforger addon by [COALITION](https://coalitiongroup.net) that makes the vanilla AI more accurate, more reactive and more aggressive, and gives Game Masters new orders to direct it. It changes how the existing AI behaves rather than replacing it, and depends only on the base game.

Every setting can be set by mission makers in Workbench and changed live by a Game Master, for the whole mission or for a single group.

> **Status: in development.** The addon is new. Several features have had little or no testing in a real session, and defaults will change. Bug reports are welcome, see [Reporting Issues](#contributing-and-reporting-issuesfeature-requests).

## Features

### Smarter, deadlier soldiers
- **Accuracy**: a multiplier on the AI's random aiming error, and a smaller penalty for shooting at poorly lit targets.
- **Fire tempo**: less time settling the aim between shots and bursts.
- **Spotting**: faster visual detection, and suppressed soldiers are no longer nearly blind.
- **Suppression**: soldiers take less suppression from impacts, fly-bys and explosions, so they keep fighting under fire.
- **Faster reactions**: targets and weapons are re-evaluated more often, and groups commit to a contact after 2 seconds instead of 5.
- **Forced skill**: optionally force one vanilla skill level on all AI.
- **Skill presets**: one dropdown sets all of the above, plus bursts, launcher use and when a group breaks off. **Militia** and **Regular** fire often and long but are inaccurate, **Elite** is accurate and steady, **CAG** is very accurate, very fast and never breaks off.

### Weapons
- **Launchers against infantry**: a soldier with a rocket launcher may use it on infantry he can see, between 40 and 300 m. Vanilla keeps launchers for vehicles.
- **Automatic fire at range**: rifles that can fire automatic or burst do so out to 150 m instead of 50 m.
- **Longer bursts**: configurable burst length for rifles, and for machine guns and automatic rifles such as the M249 and RPK.
- **Grenades under fire**: soldiers keep throwing frag grenades while suppressed. Vanilla soldiers stop.
- **Static weapons**: groups in contact crew empty machine guns, mortars and other static weapons near them, up to half of the group.

### Group coordination
- **Contact reports**: a group that sees enemies reports them to friendly groups in range, which move to investigate and join the fight.
- **Artillery support**: a friendly group crewing mortars or other static artillery answers a contact report with a fire mission. It holds fire if friendly AI or players are near the target.
- **Hunt**: groups without orders keep searching for the nearest enemy player. Off by default.
- **Search and Destroy assault**: any group on a Search and Destroy waypoint assaults all-out. It charges into the area firing on the move, cannot be suppressed, fires at full tempo and keeps pushing instead of waiting in cover.
- **Breaking off**: a group in contact that has lost most of its soldiers throws smoke, drops its orders and falls back away from the enemy. Vanilla groups fight to the last man.
- **Smoke**: groups throw smoke ahead of a banzai charge, ahead of a close Search and Destroy assault, and when they break off.

### Cover
- **Better cover**: soldiers only take cover that faces the enemy within 45 degrees and is at least half a meter high.
- **Staying in cover**: a soldier who found cover stays about twice as long before moving closer. Assaults and banzai charges ignore this.

### Vehicles
- **Keeping their distance**: armed vehicles back away from targets closer than 100 m, close in on targets beyond 300 m, and hold and fire in between. Both distances are settings.
- **Stopping to fight**: vehicles stop for enemies within their engage distance instead of driving past them.
- **Rushing**: with **Vehicles rush targets** ticked, on a banzai charge, or on a Search and Destroy assault, armed vehicles drive straight at their target instead.
- **Dismount on contact**: passengers get out when the group has identified enemies nearby. Drivers and gunners stay.
- **Remount**: those passengers get back in once the group has been out of contact for half a minute.
- **Bail out**: in contact, drivers and passengers leave a vehicle that can no longer move. Gunners stay on their weapon.
- **Retreat when damaged**: a group whose vehicle is badly damaged in contact breaks off and falls back.

### New Game Master orders
All four are placed like any other waypoint: select a group, pick the order from the command bar, click the destination.

- **Banzai charge**: the group breaks formation and sprints straight at the position, ignoring cover and suppression. Sprinting soldiers cannot aim, so they do not fire until they arrive.
- **Clear building**: place it on a building. The group works out the walkable spots inside, floor by floor, splits them among its soldiers and sweeps them, then carries on with its next waypoint.
- **Garrison building**: place it on a building. The soldiers spread out inside and hold it. They fight from where they are and return to their spots afterwards. The order stays until the group is given another one.
- **Man trenches**: place it on a trench line. The group finds the trench spots inside the waypoint's radius, spreads out along them and holds, like a garrison. Dug and built trenches both count.

The command bar is widened from 9 to 14 slots so the vanilla orders and the four new ones fit on one page.

### Movement
- **Roads**: for soldiers on foot, roads now cost several times more than open ground, where vanilla makes them the cheapest surface. They head for a waypoint across country and only use or cross a road when there is no reasonable way around. Vehicles are not affected.

## Settings

| Setting | COALITION AI default | Vanilla |
| --- | --- | --- |
| Enhanced AI (master switch) | On | - |
| Skill preset | Custom | - |
| Forced skill | Unit default | Unit default |
| Aiming error | x0.6 | x1 |
| Night aiming penalty | x1.5 | x2 |
| Fire tempo | x1.35 | x1 |
| Spotting speed | x1.5 | x1 |
| Spotting while suppressed | x0.7 | x0.4 |
| Suppression taken | x0.6 | x1 |
| Target re-evaluation | 0.3 s | 0.5 s |
| Group decision delay | 2 s | 5 s |
| Contact reports | On, 500 m, every 8 s | Off |
| Artillery support | On | Off |
| Artillery range / rounds / cooldown | 2000 m / 4 / 120 s | - |
| Artillery safe distance / minimum targets | 150 m / 3 | - |
| Hunt players / hunt radius | Off / 1000 m | Off |
| Assault on Search and Destroy | On | Off |
| Launchers against infantry | 50 percent | 0 percent |
| Automatic fire range | 150 m | 50 m |
| Rifle burst length | up to 6 rounds | about 1 to 4 |
| Machine gun burst length | up to 15 rounds | about 2 to 7 |
| Grenades under fire | On | Off |
| Crew static weapons / search radius | On / 40 m | Off |
| Smoke before a charge | On | Off |
| Break off at casualties / fall back distance | 60 percent / 250 m | Never |
| Dismount on contact / distance | On / 300 m | Off |
| Vehicle retreat damage | 50 percent | Never |
| Vehicle stand-off / engage distance | 100 m / 300 m | 60 m / 200 m |
| Vehicles rush targets | Off | Off |
| Remount after contact | On | Off |
| Bail out when immobilised | On | Off |
| Cover facing angle | 45 degrees | 60 degrees |
| Lowest usable cover | 0.5 m | Any |
| Cover search range | x1.5 | x1 |
| Time in cover | x2 | x1 |

### Skill presets
| Value | Militia | Regular | Elite | CAG |
| --- | --- | --- | --- | --- |
| Aiming error | x1.6 | x1.15 | x0.6 | x0.3 |
| Night aiming penalty | x2.5 | x2 | x1.5 | x1.1 |
| Fire tempo | x1.5 | x1.5 | x1.35 | x1.6 |
| Spotting speed | x0.9 | x1.1 | x1.5 | x2.2 |
| Spotting while suppressed | x0.4 | x0.5 | x0.7 | x1 |
| Suppression taken | x1.3 | x1 | x0.6 | x0.25 |
| Target re-evaluation | 0.5 s | 0.4 s | 0.3 s | 0.2 s |
| Group decision delay | 5 s | 3.5 s | 2 s | 0.5 s |
| Launchers against infantry | 40 percent | 50 percent | 50 percent | 30 percent |
| Automatic fire range | 200 m | 150 m | 150 m | 75 m |
| Rifle burst length | 8 | 6 | 6 | 3 |
| Machine gun burst length | 20 | 15 | 15 | 8 |
| Break off at casualties | 40 percent | 55 percent | 70 percent | Never |
| Time in cover | x0.6 | x1 | x2 | x2.5 |
| Cover facing angle | 75 degrees | 60 degrees | 45 degrees | 40 degrees |

A preset only writes the values in this table. Changing one of them afterwards switches the preset back to Custom.

With the master switch off, every setting falls back to its vanilla value.

### Where to change them
- **Game Master, whole mission**: open the game mode attributes and choose the **COALITION AI** tab.
- **Game Master, one group**: open the group's attributes, choose the **COALITION AI** tab and tick **Custom AI settings**. Untick it to return the group to the mission-wide settings.
- **Workbench, whole mission**: add `CAI_SettingsComponent` to the game mode entity and edit its settings. Missions without the component use the defaults above.
- **Workbench, one group**: every AI group has a **CAI Override** property. Leave it empty to use the mission-wide settings.

The road change is the exception. It is a project setting in `addon.gproj` and cannot be changed in game.

## For Scripters
- `SCR_AIGroup.CAI_StartBanzai(vector position)` gives a group a banzai waypoint in front of its other waypoints. `CAI_StopBanzai()` calls it off.
- `SCR_AIGroup.CAI_FallBack(vector enemyPos, float distance)` makes a group break off now. `CAI_ThrowSmoke(vector towardsPosition)` has it throw smoke.
- `CAI_Settings.ApplyPreset(ECAI_Preset preset)` applies a skill preset to a settings object.
- `SCR_AIGroup.CAI_GetSettings()` returns the settings a group is using. `CAI_CreateOverride()` and `CAI_ClearOverride()` manage its own copy.
- `SCR_BaseGameMode.CAI_GetSettings()` returns the mission-wide settings.
- After changing values on a settings object, call `CAI_SettingsManager.Invalidate()` so the AI picks them up.

## Structure
| Folder | Contents |
| --- | --- |
| `Scripts/Game/CAI/Settings` | `CAI_Settings`, the settings manager and `CAI_SettingsComponent` |
| `Scripts/Game/CAI/Overrides` | Modded vanilla AI classes: combat component, aiming, fire timing, threat, combat movement, group perception and target clusters |
| `Scripts/Game/CAI/Waypoints` | The Clear building, Garrison building and Man trenches waypoints |
| `Scripts/Game/CAI/Editor` | Game Master attributes for every setting |
| `Configs/Editor` | Attribute list, attribute category, command bar entries and placeable waypoint registry |
| `PrefabsEditable/Auto/AI/Waypoints` | Banzai charge, Clear building, Garrison building and Man trenches waypoint prefabs |
| `UI` | Attribute tab icon and the widened command bar layout |

## Compatibility
- Works alongside Game Master Enhanced. A forced skill set here takes priority over GME's skill attributes while it is set.
- The addon replaces the vanilla command bar layout (`UI/layouts/Editor/Toolbar/CommandMenu/CommandMenu.layout`). Another addon replacing the same file will conflict.
- `addon.gproj` carries a copy of the vanilla navmesh settings with the road costs changed. It may need refreshing after a game update that changes those settings.
- Game Master changes to these settings are not stored in Game Master session saves.

## Taking Part in Events
COALITION hosts events and weekly sessions open to the public. If you would like to take part in our Arma Reforger events, feel free to join our [Discord](https://discord.gg/the-coalition)!

## Contributing and Reporting Issues/Feature Requests
This addon is open to be used by and contributed to by other Reforger groups.
If you would like to report bugs or feedback, feel free to create an Issue on the GitHub page, or let us know in our [Discord](https://discord.gg/the-coalition) if that is easier for you.
If you would like to contribute to this repo, feel free to fork and PR as you see fit.

## Further Info
Most of the discussion relating to this addon takes place on our [Discord](https://discord.gg/the-coalition), feel free to take part or just read up.

<img src="http://coalitiongroup.net/coalition.png">

[![COALITION DISCORD](https://img.shields.io/badge/COALITION_Discord-blue)](https://discord.gg/the-coalition)

![Discord Banner 1](https://discordapp.com/api/guilds/237991125523103747/widget.png?style=banner1)
