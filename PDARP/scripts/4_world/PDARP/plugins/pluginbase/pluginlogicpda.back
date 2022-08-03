class PluginLogicPDA extends PluginBase
{	
	EffectSound effect;
    
    ref PluginPDArp_ServerOptions m_server_options;
	
	void PluginLogicPDA()
	{
        if (GetGame().IsServer())
        {
            string serverOptionsPath = "$profile:\\PDArp_ServerOptions.json";
            if (FileExist(serverOptionsPath))
            {
                JsonFileLoader<ref PluginPDArp_ServerOptions>.JsonLoadFile(serverOptionsPath, m_server_options);
            }
            else
            {
                m_server_options = new PluginPDArp_ServerOptions;
                JsonFileLoader<ref PluginPDArp_ServerOptions>.JsonSaveFile(serverOptionsPath, m_server_options);
            }
        }
        
		if (PDArpDebugMode) Print(PDArpModPreffix + "PluginLogicPDA construct.");
	}
	
	void ~PluginLogicPDA()
	{
        if (m_server_options) delete m_server_options;
        
		if (PDArpDebugMode) Print(PDArpModPreffix + "PluginLogicPDA destruct.");
	}
	
	override void OnInit()
	{
		GetRPCManager().AddRPC( PDArpModPreffix, "GetVisualUserId", this, SingleplayerExecutionType.Both ); 
		GetRPCManager().AddRPC( PDArpModPreffix, "AddContact", this, SingleplayerExecutionType.Both ); 
		GetRPCManager().AddRPC( PDArpModPreffix, "CheckContacts", this, SingleplayerExecutionType.Both ); 
		GetRPCManager().AddRPC( PDArpModPreffix, "SendMessage", this, SingleplayerExecutionType.Both );
        GetRPCManager().AddRPC( PDArpModPreffix, "SendGlobalMessage", this, SingleplayerExecutionType.Both );
				
		if (GetGame().IsClient())
		{
			GetRPCManager().SendRPC( PDArpModPreffix, "GetVisualUserId", new Param1<string>( "" ), true );
		}
	}
	
	void SendMessage( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    { 
		if( type == CallType.Server )
        {
			Param2< string, string > serverData;			
        	if ( !ctx.Read( serverData ) ) return;
			
			if (PDArpDebugMode) Print(PDArpModPreffix + "SendMessage RPC called on server from " + sender);
			
			string senderId = sender.GetId();
			string senderName = sender.GetName();
			ref array<Man> players = new array<Man>();
			GetGame().GetPlayers(players);
			for (int q = 0; q < players.Count(); q++)
			{
				ref PlayerBase player = PlayerBase.Cast(players[q]);
				if (player)
				{
					ref PlayerIdentity identity = player.GetIdentity();
					string identityId = identity.GetId();
					if (serverData.param1 == identityId)
					{
						if (HasWorkingPDA(player))
						{
							string identityName = identity.GetName();
							if (PDArpDebugMode) Print(PDArpModPreffix + "Found target player 1; S: " + sender + "I: " + identity);
							if (PDArpDebugMode) Print(PDArpModPreffix + "RPC data 1: " + identityId + " | " + identityName + " | " + senderId + " | " + serverData.param2);
							
							GetRPCManager().SendRPC( PDArpModPreffix, "SendMessage", new Param4< string, string, string, string >( identityId, identityName, senderId, serverData.param2 ), true, sender );
							
							if (!(senderId == identityId))
							{
								if (PDArpDebugMode) Print(PDArpModPreffix + "RPC data 2: " + senderId + " | " + senderName + " | " + senderId + " | " + serverData.param2);
								GetRPCManager().SendRPC( PDArpModPreffix, "SendMessage", new Param4< string, string, string, string >( senderId, senderName, senderId, serverData.param2 ), true, identity );
							}
							
							return;
						}
					}
				}
			}
			
			GetRPCManager().SendRPC( PDArpModPreffix, "SendMessage", new Param4< string, string, string, string >( "", "", "", "" ), true, sender );
		}
		else
		{
			Param4< string, string, string, string > clientData;			
        	if ( !ctx.Read( clientData ) ) return;
			
			if (PDArpDebugMode) Print(PDArpModPreffix + "SendMessage RPC called on cleint from " + sender);
			
			string contactId = clientData.param1;
			string contactName = clientData.param2;
			string userSenderId = clientData.param3;
			string message = clientData.param4;
					
			if (PDArpDebugMode) Print(PDArpModPreffix + "SendMessage received: " + contactId + " | " + contactName + " | " + userSenderId + " | " + message);
			
			PluginPDArp pluginPDArp;
			Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));			
			if (pluginPDArp)
			{
				ref PluginPDArp_Conversation msgContact = pluginPDArp.FindContact(contactId);
				if (msgContact == null)
				{
					pluginPDArp.AddContact(contactId, contactName);
					msgContact = pluginPDArp.FindContact(contactId);
				}
				
				if (!msgContact.m_IsBanned)
				{
					if (userSenderId == contactId && !pluginPDArp.m_options.m_Muted)
					{	
						GetGame().GetPlayer().PlaySoundSet(effect, "messagePDA_SoundSet", 0, 0);
					}
					
					pluginPDArp.AddComment(contactId, userSenderId, message);
				}
			}
		}
	}
	
	bool HasWorkingPDA(PlayerBase player)
	{
		array<EntityAI> itemsArray = new array<EntityAI>;		
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
		
		ItemBase itemInHands = player.GetItemInHands();
		if (itemInHands)
		{
			itemsArray.Insert(EntityAI.Cast(itemInHands));
		}
		
		ItemPDA item;		
		for (int i = 0; i < itemsArray.Count(); i++)
		{
			ItemPDA.CastTo(item, itemsArray.Get(i));

			if (!item)
				continue;

			if (item.IsRuined())
				continue;

			if (!item.HasEnergyManager())
				continue;
			
			if (!item.GetCompEM().CanWork())
				continue;

			return true;
		}
		
		return false;
	}
	
	void CheckContacts( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    { 
		if( type == CallType.Server )
        {
			Param1< array<string> > serverData;			
        	if ( !ctx.Read( serverData ) ) return;
			
			if (PDArpDebugMode) Print(PDArpModPreffix + "CheckContacts RPC called on server from " + sender);
			
			ref array<string> request = new array<string>();
			request.Copy(serverData.param1);
			
			ref array<Man> players = new array<Man>();
			GetGame().GetPlayers(players);
			
			ref array<string> result = new array<string>();
			for (int i = 0; i < request.Count(); i++)
			{
				string uid = request[i];				
				for (int q = 0; q < players.Count(); q++)
				{
					ref PlayerBase player = PlayerBase.Cast(players[q]);
					if (player)
					{
						ref PlayerIdentity identity = player.GetIdentity();
						string identityId = identity.GetId();
						if (uid == identityId && HasWorkingPDA(player))
						{
							if (PDArpDebugMode) Print(PDArpModPreffix + "CheckContacts Player " + identity.GetName() + " is online.");
							result.Insert(uid);
						}
					}
				}
			}
			
			GetRPCManager().SendRPC( PDArpModPreffix, "CheckContacts", new Param1< ref array<string> >( result ), true, sender );
		}
		else
        {
			Param1< array<string> > clientData;
        	if ( !ctx.Read( clientData ) ) return;
			
			if (PDArpDebugMode) Print(PDArpModPreffix + "CheckContacts RPC called on client from " + sender);
			
			PluginPDArp pluginPDArp;
			Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
			if (pluginPDArp)
			{
				pluginPDArp.m_onlineContacts.Clear();
				pluginPDArp.m_onlineContacts.Copy(clientData.param1);
				if (pluginPDArp.IsOpen())
				{
					pluginPDArp.m_PDArpMenu.m_dirty = true;
				}
			}
		}
	}
	
	void GetVisualUserId( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {   
		if( type == CallType.Server )
        {
			if (PDArpDebugMode) Print(PDArpModPreffix + "GetVisualUserId RPC called on server from " + sender);
			string userVisualId = sender.GetPlainId();
			GetRPCManager().SendRPC( PDArpModPreffix, "GetVisualUserId", new Param3<string, bool, bool>( userVisualId, m_server_options.m_enableGlobalChannel, m_server_options.m_enableGlobalChannelSound ), true, sender );
		}
		else
        {
			if (PDArpDebugMode) Print(PDArpModPreffix + "GetVisualUserId RPC called on client from " + sender);
			
			Param3<string, bool, bool> clientData;
        	if ( !ctx.Read( clientData ) ) return;
			
			PluginPDArp pluginPDArp;
			Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
			if (pluginPDArp)
			{
				pluginPDArp.m_steamId = clientData.param1;
                pluginPDArp.m_enableGlobalChat = clientData.param2;
                pluginPDArp.m_enableGlobalChatSound = clientData.param3;
			}
		}
	}
	
	void AddContact( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {        
        if( type == CallType.Server )
        {
			Param1< string > serverData;
        	if ( !ctx.Read( serverData ) ) return;
			string requestName = serverData.param1;
			
            if (PDArpDebugMode) Print(PDArpModPreffix + "AddContact RPC called on server from " + sender + " | " + requestName);
			
			ref array<Man> players = new array<Man>();
			GetGame().GetPlayers(players);
			for (int i = 0; i < players.Count(); i++)
			{
				PlayerBase player = PlayerBase.Cast(players[i]);
				if (player)
				{
					PlayerIdentity identity = player.GetIdentity();
					string contactPlainId = identity.GetPlainId();
					string contactSteamId = identity.GetId();
					string contactName = identity.GetName();
					if ( (contactPlainId == requestName) || (contactName == requestName) ) 
					{
						if (PDArpDebugMode) Print(PDArpModPreffix + "Found identity: " + identity + " | " + contactPlainId + " | " + contactName);
						if (!(sender.GetId() == contactSteamId) || PDArpDebugMode)
						{
							if (PDArpDebugMode) Print(PDArpModPreffix + "AddContact player with id " + requestName + " found: " + contactSteamId + "; " + contactName);
							GetRPCManager().SendRPC( PDArpModPreffix, "AddContact", new Param2<string, string>( contactSteamId, contactName ), true, sender );
							return;
						}
					}
				}
			}
			
			if (PDArpDebugMode) Print(PDArpModPreffix + "AddContact player with id " + requestName + " not found.");			
			GetRPCManager().SendRPC( PDArpModPreffix, "AddContact", new Param2<string, string>( "", "" ), true, sender );
        }
        else
        {
			Param2< string, string > clientData;
        	if ( !ctx.Read( clientData ) ) return;
			
			if (PDArpDebugMode) Print(PDArpModPreffix + "AddContact RPC called on client from " + sender + "; " + clientData.param1 + " " + clientData.param2);
			
			PluginPDArp pluginPDArp;
			Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
			if (pluginPDArp && pluginPDArp.IsOpen())
			{
				pluginPDArp.AddContact(clientData.param1, clientData.param2);
			}
        }
    }
    
    void SendGlobalMessage( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if( type == CallType.Client )
		{
			PluginPDArp pluginPDArp;
			Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));			
			if (pluginPDArp && pluginPDArp.m_enableGlobalChat)
			{
				Param2< string, string > clientData;			
        		if ( !ctx.Read( clientData ) ) return;
				
				pluginPDArp.m_globalMessages.Insert(clientData);
                pluginPDArp.m_globalChatUnreaded = pluginPDArp.m_globalChatUnreaded + 1;
				
				PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
				if (player && HasWorkingPDA(player))
				{
					if (!pluginPDArp.m_options.m_Muted && pluginPDArp.m_enableGlobalChatSound)
					{	
						GetGame().GetPlayer().PlaySoundSet(effect, "messagePDA_SoundSet", 0, 0);
					}
					
					if (pluginPDArp.IsOpen())
					{
						pluginPDArp.m_PDArpMenu.m_sendMessageStatus = 2;
					}
				}
			}
		}
        else
        {
            Param1< string > serverData;			
        	if ( !ctx.Read( serverData ) ) return;
			
			if (!sender) return;
			
			if (!m_server_options.m_enableGlobalChannel) return;
			
			GetRPCManager().SendRPC( PDArpModPreffix, "SendGlobalMessage", new Param2< string, string >( sender.GetName(), serverData.param1 ), true );	
        }
	}
};

class PluginPDArp_ServerOptions
{
    bool m_enableGlobalChannel = false;
    bool m_enableGlobalChannelSound = false;
};