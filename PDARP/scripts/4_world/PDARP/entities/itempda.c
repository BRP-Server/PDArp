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

	void ~ItemPDA() {
		PluginPDArp pluginPDArp;
		Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
		if (pluginPDArp && pluginPDArp.m_entities) {
			pluginPDArp.m_entities.Remove(hashedMemId);
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
		if (hashedMemId == "" && memId !=  0) {
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
		GetMemoryID();
		PluginPDArp pluginPDArp;
		Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
		pluginPDArp.m_entities.Set(hashedMemId, this);
		SetSynchDirty();
	}

	override void EEInit()
	{
		super.EEInit();
		PDArpLog.Debug("ItemPDA EEInit");
		UpdateVisualStyle();
	}
	
	override void SetActions()
	{
		super.SetActions();
		
		AddAction(ActionOpenPDA);
		PDArpLog.Debug("ItemPDA SetActions");
	}
	
	override void OnIsPlugged(EntityAI source_device)
	{
		super.OnIsPlugged(source_device);
		PDArpLog.Debug("ItemPDA OnIsPlugged");
		SetVisualStyle(1);
	}
	
	override void OnIsUnplugged(EntityAI last_energy_source)
	{
		super.OnIsUnplugged(last_energy_source);
		PDArpLog.Debug("ItemPDA OnIsUnplugged");
		SetVisualStyle(0);
	}
	
	override void OnWork(float consumed_energy)
	{
		super.OnWork(consumed_energy);
		PDArpLog.Debug("ItemPDA OnWork");
		UpdateVisualStyle();
	}
	
	override void OnWorkStart() 
	{
		super.OnWorkStart();
		PDArpLog.Debug("ItemPDA OnWorkStart");
		UpdateVisualStyle();
	}
	
	override void OnWorkStop() 
	{
		super.OnWorkStop();
		PDArpLog.Debug("ItemPDA OnWorkStop");
		UpdateVisualStyle();
	}
	
	override void OnInitEnergy() 
	{
		super.OnInitEnergy();
		PDArpLog.Debug("ItemPDA OnInitEnergy");
		UpdateVisualStyle();
	}
	
	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();
		PDArpLog.Debug("ItemPDA OnVariablesSynchronized");
		hashedMemId = "";
		GetMemoryID();
		if (hashedMemId != "") {
			PluginPDArp pluginPDArp;
			Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
			pluginPDArp.m_entities.Set(hashedMemId, this);
		}
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
		PDArpLog.Trace("ItemPDA Change texture to " + id + " => " + texture);
	}
};