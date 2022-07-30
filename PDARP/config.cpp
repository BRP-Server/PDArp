class CfgPatches
{
	class PDARP_scripts
	{
		units[]={};
		requiredAddons[]=
		{
			"DZ_Data",
			"DZ_Scripts",
			"JM_CF_Scripts"
		};
	};
};
class CfgMods
{
	class PDARP
	{
		type="mod";
		class defs
		{
			class gameScriptModule
			{
				value="";
				files[]=
				{
					"PDARP/scripts/3_Game"
				};
			};
			class worldScriptModule
			{
				value="";
				files[]=
				{
					"PDARP/scripts/4_World"
				};
			};
			class missionScriptModule
			{
				value="";
				files[]=
				{
					"PDARP/scripts/5_Mission"
				};
			};
			class imageSets
			{
				files[]={};
			};
		};
	};
};
class CfgVehicles
{
	class Inventory_Base;
	class ItemPDA: Inventory_Base
	{
		scope=2;
		title="#pda_item_name";
		displayName="#pda_item_name";
		descriptionShort="#pda_item_desc";
		model="PDARP\pda.p3d";
		hiddenSelections[]=
		{
			"body"
		};
		hiddenSelectionsTextures[]=
		{
			"PDARP\data\pda_co.paa",
			"PDARP\data\pda_on_co.paa"
		};
		animClass="ItemPDA";
		itemSize[]={2,2};
		weight=250;
		repairableWithKits[]={5,7};
		repairCosts[]={30,25};
		lootTag[]=
		{
			"Police",
			"Hunting",
			"Camping",
			"Military_east"
		};
		oldpower=0;
		attachments[]=
		{
			"BatteryD"
		};
		class EnergyManager
		{
			hasIcon=1;
			autoSwitchOff=1;
			energyUsagePerSecond=9.9999997e-05;
			plugType=1;
			attachmentAction=1;
			wetnessExposure=0.1;
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=50;
					healthLevels[]=
					{
						
						{
							1,
							
							{
								"PDARP\data\pda.rvmat"
							}
						},
						
						{
							0.69999999,
							
							{
								"PDARP\data\pda.rvmat"
							}
						},
						
						{
							0.5,
							
							{
								"PDARP\data\pda_damage.rvmat"
							}
						},
						
						{
							0.30000001,
							
							{
								"PDARP\data\pda_damage.rvmat"
							}
						},
						
						{
							0,
							
							{
								"PDARP\data\pda_destruct.rvmat"
							}
						}
					};
				};
			};
		};
	};
};
class CfgSoundShaders
{
	class baseCharacter_SoundShader;
	class messagePDA_Soundshader: baseCharacter_SoundShader
	{
		samples[]=
		{
			
			{
				"PDARP\data\pda_message",
				1
			},
			
			{
				"PDARP\data\pda_message",
				1
			}
		};
		volume=0.5;
	};
};
class CfgSoundSets
{
	class baseCharacter_SoundSet;
	class messagePDA_SoundSet: baseCharacter_SoundSet
	{
		soundShaders[]=
		{
			"messagePDA_Soundshader"
		};
	};
};
class CfgModels
{
	class Default;
	class pda: Default
	{
		sections[]=
		{
			"body"
		};
	};
};
