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
		Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));

		if ( key == KeyCode.KC_ESCAPE ) {	
			if (pluginPDArp && pluginPDArp.IsOpen()) {
				pluginPDArp.Close();
			}
		}
		else if ( key == KeyCode.KC_RETURN ) {
			if (pluginPDArp && pluginPDArp.IsOpen()) {
				pluginPDArp.m_PDArpMenu.m_externalSendEvent = true;
			}
		}
	}
}