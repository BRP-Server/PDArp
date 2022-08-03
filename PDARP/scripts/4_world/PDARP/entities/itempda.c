class ItemPDA : ItemBase
{
	int b1 = 0;
	int b2 = 0;
	int b3 = 0;
	int b4 = 0;

	string memId = "";

	void ItemPDA() {
		RegisterNetSyncVariableInt("b1");
		RegisterNetSyncVariableInt("b2");
		RegisterNetSyncVariableInt("b3");
		RegisterNetSyncVariableInt("b4");
	}

	string GetMemoryID() {
		if (memId == "") {
			string concatenated = b1.ToString() + b2.ToString() + b3.ToString() + b4.ToString();
			CF_TextReader reader = new CF_TextReader(new CF_StringStream(concatenated));
			CF_Base16Stream output = new CF_Base16Stream();
			CF_SHA256.Process(reader, output);
			memId = output.Encode().Substring(0, 8);
			memId.ToLower();
		}
		return memId;
	}
	
	override bool OnStoreLoad (ParamsReadContext ctx, int version) {
		PDArpLog.Trace("OnStoreLoad version: " + version);
		return super.OnStoreLoad(ctx, version);
	}

	override void AfterStoreLoad() {
		GetPersistentID(b1, b2, b3, b4);
		PDArpLog.Trace("AfterStoreLoad: " + GetMemoryID());
		SetSynchDirty();
	}

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