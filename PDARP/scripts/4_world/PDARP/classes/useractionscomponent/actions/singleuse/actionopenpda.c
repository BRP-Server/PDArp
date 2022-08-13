class ActionOpenPDA extends ActionSingleUseBase {
	PDArpLog logger;

	void ActionOpenPDA() {
		logger = GetPDArpLog();
		logger.Debug("ActionOpenPDA construct.");
		m_CommandUID      = DayZPlayerConstants.CMD_ACTIONMOD_OPENITEM_ONCE;
		m_CommandUIDProne = DayZPlayerConstants.CMD_ACTIONFB_OPENITEM_ONCE;
	}
	
	override void CreateConditionComponents() {	
		m_ConditionItem = new CCINonRuined;
		m_ConditionTarget = new CCTNone;
	}

	override bool HasTarget()
	{
		return false;
	}

	override string GetText()
	{
		return "#open";
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		if (!item)
		{
			return false;
		}
		
		if ( !item.IsRuined() && item.HasEnergyManager() && item.GetCompEM().CanWork() )
		{
			if (GetGame().IsClient())
			{
				PluginPDArp pluginPDArp;
				Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
				
				if (!pluginPDArp)
				{
					return false;
				}
				
				if (pluginPDArp.IsOpen())
				{
					return false;
				}
			}
				
			return true;
		}
		
		return false;
	}
	
	override bool ActionConditionContinue( ActionData action_data )
	{
		return true;
	}
	
	override void OnExecuteClient( ActionData action_data )
	{		
		action_data.m_MainItem.GetCompEM().ConsumeEnergy(0.01);
		
		PluginPDArp pluginPDArp;
		Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
		logger.Debug("ActionOpenPDA execute => Plug: " + pluginPDArp);
		
		if (pluginPDArp && !pluginPDArp.IsOpen())
		{					
			pluginPDArp.Open();
		}
	}

	override void OnExecuteServer( ActionData action_data )
	{
		action_data.m_MainItem.GetCompEM().ConsumeEnergy(0.01);
	}
};