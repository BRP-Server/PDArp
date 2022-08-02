modded class PluginManager
{
	override void Init()
	{		
		RegisterPlugin("PluginPDArp", true, true);
		super.Init();
	}
}