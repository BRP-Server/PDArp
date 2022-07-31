modded class PluginManager
{
	override void Init()
	{
		if (GetGame().IsClient())
		{
			RegisterPlugin("PluginPDArp", true, false);
		}
		
		RegisterPlugin("PluginLogicPDA", true, true);

		super.Init();
	}
}