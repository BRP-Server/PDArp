class ItemPDA : ItemBase
{	
	override void EEInit()
	{
		super.EEInit();
		
		if (PDARPDebugMode) Print(PDARPModPreffix + "EEInit.");
		UpdateVisualStyle();
	}
	
	override void SetActions()
	{
		super.SetActions();
		
		AddAction(ActionOpenPDA);
		if (PDARPDebugMode) Print(PDARPModPreffix + "SetActions.");
	}
	
	override void OnIsPlugged(EntityAI source_device)
	{
		super.OnIsPlugged(source_device);
		
		if (PDARPDebugMode) Print(PDARPModPreffix + "OnIsPlugged.");
		SetVisualStyle(1);
	}
	
	override void OnIsUnplugged(EntityAI last_energy_source)
	{
		super.OnIsUnplugged(last_energy_source);
		
		if (PDARPDebugMode) Print(PDARPModPreffix + "OnIsUnplugged.");
		SetVisualStyle(0);
	}
	
	override void OnWork(float consumed_energy)
	{
		super.OnWork(consumed_energy);
		
		if (PDARPDebugMode) Print(PDARPModPreffix + "OnWork.");
		UpdateVisualStyle();
	}
	
	override void OnWorkStart() 
	{
		super.OnWorkStart();
		
		if (PDARPDebugMode) Print(PDARPModPreffix + "OnWorkStart.");
		UpdateVisualStyle();
	}
	
	override void OnWorkStop() 
	{
		super.OnWorkStop();
		
		if (PDARPDebugMode) Print(PDARPModPreffix + "OnWorkStop.");
		UpdateVisualStyle();
	}
	
	override void OnInitEnergy() 
	{
		super.OnInitEnergy();
		
		if (PDARPDebugMode) Print(PDARPModPreffix + "OnInitEnergy.");
		UpdateVisualStyle();
	}
	
	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();
		
		if (PDARPDebugMode) Print(PDARPModPreffix + "OnVariablesSynchronized.");
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
		if (PDARPDebugMode) Print(PDARPModPreffix + "Change PDA texture to " + id + " =>" + texture);
	}
};