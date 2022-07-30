modded class PluginManager
{
	override void Init()
	{
		if (GetGame().IsClient())
		{
			RegisterPlugin("PluginPDARP", true, false);
		}
		
		RegisterPlugin("PluginLogicPDA", true, true);

		super.Init();
	}
}