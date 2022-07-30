modded class MissionGameplay
{	
	override void OnInit()
  	{
		super.OnInit();
	}
	
	override void OnKeyRelease(int key)
	{
		super.OnKeyRelease(key);
		
		PluginPDARP pluginPDARP;
		if ( key == KeyCode.KC_ESCAPE )
		{	
			Class.CastTo(pluginPDARP, GetPlugin(PluginPDARP));
			if (pluginPDARP && pluginPDARP.IsOpen())
			{
				pluginPDARP.Close();
			}
		}
		else if ( key == KeyCode.KC_RETURN )
		{
			Class.CastTo(pluginPDARP, GetPlugin(PluginPDARP));
			if (pluginPDARP && pluginPDARP.IsOpen())
			{
				pluginPDARP.m_PDARPMenu.m_externalSendEvent = true;
			}
		}
	}
}