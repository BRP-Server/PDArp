class ItemPDA : ItemBase
{
	int memId = 0;
	string hashedMemId = "";

	void ItemPDA() {
		RegisterNetSyncVariableInt("memId");
		if (GetGame().IsServer()) {
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(GenerateMemoryIdentifier, 500);
		}
	}

	private void GenerateMemoryIdentifier() {
		if (memId == 0) {
			PluginPDArp pluginPDArp;
			Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
			memId = pluginPDArp.GenerateMemoryIdentifier();
			hashedMemId = "";
			GetMemoryID();
			SetSynchDirty();
		}
	}

	string GetMemoryID() {
		if (hashedMemId == "") {
			CF_TextReader reader = new CF_TextReader(new CF_StringStream(memId.ToString()));
			CF_Base16Stream output = new CF_Base16Stream();
			CF_SHA256.Process(reader, output);
			hashedMemId = output.Encode().Substring(0, 8);
			hashedMemId.ToLower();
		}
		return hashedMemId;
	}

	override void OnStoreSave( ParamsWriteContext ctx ) {
		super.OnStoreSave(ctx);
		ctx.Write(memId);
	}
	
	override bool OnStoreLoad(ParamsReadContext ctx, int version) {
		auto loaded = super.OnStoreLoad(ctx, version);
		if (loaded) {
			ctx.Read(memId);
			PDArpLog.Trace("OnStoreLoad version: " + version);
			return true;
		}
		return loaded;
	}

	override void AfterStoreLoad() {
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
		hashedMemId = "";
		GetMemoryID();
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