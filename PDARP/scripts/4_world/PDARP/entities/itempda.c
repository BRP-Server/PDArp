class ItemPDA : ItemBase
{	
	override void EEInit()
	{
		super.EEInit();
		
		if (PDArpDebugMode) Print(PDArpModPreffix + "EEInit.");
		UpdateVisualStyle();
	}
	
	override void SetActions()
	{
		super.SetActions();
		
		AddAction(ActionOpenPDA);
		if (PDArpDebugMode) Print(PDArpModPreffix + "SetActions.");
	}
	
	override void OnIsPlugged(EntityAI source_device)
	{
		super.OnIsPlugged(source_device);
		
		if (PDArpDebugMode) Print(PDArpModPreffix + "OnIsPlugged.");
		SetVisualStyle(1);
	}
	
	override void OnIsUnplugged(EntityAI last_energy_source)
	{
		super.OnIsUnplugged(last_energy_source);
		
		if (PDArpDebugMode) Print(PDArpModPreffix + "OnIsUnplugged.");
		SetVisualStyle(0);
	}
	
	override void OnWork(float consumed_energy)
	{
		super.OnWork(consumed_energy);
		
		if (PDArpDebugMode) Print(PDArpModPreffix + "OnWork.");
		UpdateVisualStyle();
	}
	
	override void OnWorkStart() 
	{
		super.OnWorkStart();
		
		if (PDArpDebugMode) Print(PDArpModPreffix + "OnWorkStart.");
		UpdateVisualStyle();
	}
	
	override void OnWorkStop() 
	{
		super.OnWorkStop();
		
		if (PDArpDebugMode) Print(PDArpModPreffix + "OnWorkStop.");
		UpdateVisualStyle();
	}
	
	override void OnInitEnergy() 
	{
		super.OnInitEnergy();
		
		if (PDArpDebugMode) Print(PDArpModPreffix + "OnInitEnergy.");
		UpdateVisualStyle();
	}
	
	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();
		
		if (PDArpDebugMode) Print(PDArpModPreffix + "OnVariablesSynchronized.");
		UpdateVisualStyle();
	}
	
	void UpdateVisualStyle()
	{
		if (GetGame().IsClient())
		{
			int texIndex = 0;
			if (!IsRuined() && HasEnergyManager() && GetCompEM().CanWork())
			{
				texIndex = 1;
			}
			
			SetVisualStyle(texIndex);
		}
	}
	
	void SetVisualStyle(int id)
	{
		TStringArray textures = GetHiddenSelectionsTextures();
		string texture = textures.Get(id);
		SetObjectTexture(0, texture);
		if (PDArpDebugMode) Print(PDArpModPreffix + "Change PDA texture to " + id + " =>" + texture);
	}
};