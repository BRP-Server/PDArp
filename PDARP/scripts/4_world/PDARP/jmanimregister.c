modded class JMAnimRegister
{
	override void OnRegisterOneHanded( DayZPlayerType pType, DayzPlayerItemBehaviorCfg pBehavior )
    {
        super.OnRegisterOneHanded( pType, pBehavior );
		pType.AddItemInHandsProfileIK("ItemPDA", "dz/anims/workspaces/player/player_main/props/player_main_1h_compass.asi", pBehavior, "dz/anims/anm/player/ik/gear/compass.anm");   
		if (PDArpDebugMode) Print(PDArpModPreffix + "OnRegisterOneHanded.");
    }
};
