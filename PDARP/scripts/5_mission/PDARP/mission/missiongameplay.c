modded class MissionGameplay
{	
	override void OnInit()
  	{
		super.OnInit();
	}
	
	override void OnKeyRelease(int key)
	{
		super.OnKeyRelease(key);
		
		PluginPDArp pluginPDArp;
		if ( key == KeyCode.KC_ESCAPE )
		{	
			Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
			if (pluginPDArp && pluginPDArp.IsOpen())
			{
				pluginPDArp.Close();
			}
		}
		else if ( key == KeyCode.KC_RETURN )
		{
			Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
			if (pluginPDArp && pluginPDArp.IsOpen())
			{
				pluginPDArp.m_PDArpMenu.m_externalSendEvent = true;
			}
		}
	}
}