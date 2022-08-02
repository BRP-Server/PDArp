class PDArpContact {
	int id;
	string name;
	
	void PDArpContact(int _id, string _name) {
		id = _id;
		name = _name;
	}
}

class ChatMessage {
	int sender_id;
	string message;
	int time;

	void ChatMessage(int deviceId, string txt) {
		sender_id = deviceId;
		message = txt;
		//TODO: Get time now
	}
}

class ChatRoom {
	string id;
	string name;
	ref array<int> participant_device_ids;
	ref array<ref ChatMessage> messages;
	
	void ChatRoom(string _id, string _name, array<int> _devices, array<ChatMessage> _messages) {
		id = _id;
		name = _name;
		participant_device_ids = _devices;
		if (participant_device_ids == null) {
			participant_device_ids = new ref array<int>;
		}
		messages = new ref array<ref ChatMessage>;
		if (_messages != null) {
			foreach(ChatMessage m: _messages) {
				messages.Insert(m);
			}
		}
	}
}

class DeviceMemory {
	int id;
	ref map <int, ref PDArpContact> contacts;
	ref array<string> chatroom_ids;
}

class PluginPDArp extends PluginBase
{	
	EffectSound effect;

	static private const string DIR_PATH = "$profile:FuelControl";
	static private const string SETTINGS_PATH = DIR_PATH + "\\settings.json";
	static private const string DATA_DIR_PATH = DIR_PATH + "\\data";
	static private const string DEVICES_PATH = DATA_DIR_PATH + "\\devices.json";
	
	ref PluginPDArpSettings m_settings;
	ref map<int, ref DeviceMemory> m_devices;
	ref map<string, ref ChatRoom> m_rooms;

	ref PDArpMenu m_PDArpMenu;

	void PluginPDArp()
	{
        if (GetGame().IsServer())
        {

			if (!FileExist(DIR_PATH)){
				MakeDirectory(DIR_PATH);
			}

			if (!FileExist(DATA_DIR_PATH)){
				MakeDirectory(DATA_DIR_PATH);
			}

            if (FileExist(SETTINGS_PATH)) {
                JsonFileLoader<ref PluginPDArpSettings>.JsonLoadFile(SETTINGS_PATH, m_settings);
            } else {
                m_settings = new PluginPDArpSettings;
                JsonFileLoader<ref PluginPDArpSettings>.JsonSaveFile(SETTINGS_PATH, m_settings);
            }

			if (FileExist(DEVICES_PATH)) {
				JsonFileLoader<ref map<int, ref DeviceMemory>>.JsonLoadFile(DEVICES_PATH, m_devices);
			} else {
				m_devices = new ref map<int, ref DeviceMemory>>;
				JsonFileLoader<ref map<int, ref DeviceMemory>>.JsonSaveFile(DEVICES_PATH, m_devices);
			}

			// Load the chat rooms in memory
			foreach(auto device: m_devices) {
				foreach(auto room_id: device.chatroom_ids) {
					string room_path = DATA_DIR_PATH + "\\room_" + room_id + ".json";
					if (m_rooms.Get(room_id) && FileExist(room_path)) {
						ref ChatRoom room;
						JsonFileLoader<ref ChatRoom>.JsonLoadFile(room_path, room);
						m_rooms.Insert(room_id, room);
					} else {
						m_rooms.Remove(room_id);
					}
				}
			}
        }

		if (GetGame().IsClient()) {
			auto man = GetGame().GetPlayer();
			ref PlayerBase player = PlayerBase.Cast(man);
			auto pdas = GetWorkingPDAsOnPlayer(player);
			foreach(auto pda: pdas) {
				GetRPCManager().SendRPC( PDArpModPreffix, "GetDeviceMemory", new Param1<int>( pda.GetID() ), true );
			}
		}
        
		if (PDArpDebugMode) Print(PDArpModPreffix + "PluginPDArp construct.");
	}
	
	override void OnInit()
	{
		GetRPCManager().AddRPC( PDArpModPreffix, "AddContact", this, SingleplayerExecutionType.Both ); 
		GetRPCManager().AddRPC( PDArpModPreffix, "SendMessage", this, SingleplayerExecutionType.Both );
		GetRPCManager().AddRPC( PDArpModPreffix, "GetDeviceMemory", this, SingleplayerExecutionType.Both );
		GetRPCManager().AddRPC( PDArpModPreffix, "GetChatRoom", this, SingleplayerExecutionType.Both);
	}
	
	void SaveRoom(string room_id) {
		string room_path = DATA_DIR_PATH + "\\room_" + room_id + ".json";
		auto room = m_rooms.Get(room_id);
		if (room != null) {
			JsonFileLoader<ref ChatRoom>.JsonSaveFile(room_path, room);
		}
	}
	
	void SaveDevices() {
		JsonFileLoader<ref map<int, ref DeviceMemory>>.JsonSaveFile(DEVICES_PATH, m_devices);
	}

	void GetDeviceMemory( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target ) {
		DeviceMemory mem;
		if (GetGame().IsServer()) {
			Param1< int > serverData;			
			if ( !ctx.Read( serverData ) ) return;

			mem = m_devices.Get(serverData.param1);
			GetRPCManager().SendRPC( PDArpModPreffix, "GetDeviceMemory", new Param1<DeviceMemory>( mem ), true, sender );
		}
		
		if (GetGame().IsClient()) {
			Param1< DeviceMemory > clientData;
			if ( !ctx.Read( clientData ) ) return;
			
			mem = clientData.param1;
			m_devices.Set(mem.id, mem);
			foreach(auto roomId: mem.chatroom_ids) {
				GetRPCManager().SendRPC( PDArpModPreffix, "GetChatRoom", new Param1<string>( roomId ), true );
			}
		}
	}
	
	void GetChatRoom( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target ) {
		ChatRoom room;
		if (GetGame().IsServer()) {
			Param1< string > serverData;			
			if ( !ctx.Read( serverData ) ) return;

			room = m_rooms.Get(serverData.param1);
			GetRPCManager().SendRPC( PDArpModPreffix, "GetChatRoom", new Param1<ChatRoom>( room ), true, sender );
		}
		
		if (GetGame().IsClient()) {
			Param1< ChatRoom > clientData;
			if ( !ctx.Read( clientData ) ) return;
			
			room = clientData.param1;
			
			m_rooms.Set(room.id, room);
		}
	}

	void SendMessage( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target ) {
		int deviceId;
		string roomId;
		PlayerBase player;
		ref array<ItemPDA> pdas;

		if (GetGame().IsServer()) {
			Param3< int, string, string > serverData;			
			if ( !ctx.Read( serverData ) ) return;
			
			deviceId = serverData.param1;
			roomId = serverData.param2;
			string txt = serverData.param3;
	
			if (PDArpDebugMode) Print(PDArpModPreffix + "SendMessage RPC called on server from " + sender);
			ref array<Man> players = new array<Man>();
			GetGame().GetPlayers(players);
	
			foreach(auto room: m_rooms) {
				if (room.id == roomId) {
					ref ChatMessage message = new ref ChatMessage(deviceId, txt);
					room.messages.Insert(message);
					foreach (auto man: players) {
						player = PlayerBase.Cast(man);
						pdas = GetWorkingPDAsOnPlayer(player);
						foreach(auto pda: pdas) {
							foreach (int participant: room.participant_device_ids) {
								if (pda.GetID() == participant) {
									GetRPCManager().SendRPC( PDArpModPreffix, "SendMessage", new Param3<int, string, ChatMessage>( participant, roomId, message ), true, player.GetIdentity() );
								}
							}
						}
					}
					SaveRoom(room.id);
					break;
				}
			}
		}

		if (GetGame().IsClient()) {
			Param3< int, string, ChatMessage > clientData;			
			if ( !ctx.Read( serverData ) ) return;
			
			deviceId = clientData.param1;
			roomId = clientData.param2;
			ChatMessage msg = clientData.param3;

			auto gamePlayer = GetGame().GetPlayer();
			player = PlayerBase.Cast(gamePlayer);

			DeviceMemory mem;

			pdas = GetWorkingPDAsOnPlayer(player);

			// Maybe this is not needed. It just ensures the player has the pda in their inventory;
			foreach(auto p: pdas) {
				if(p.GetID() == deviceId) {
					mem = m_devices.Get(deviceId);
					break;
				}
			}

			auto contact = mem.contacts.Get(deviceId);
			
			if (contact == null) {
				mem.contacts.Set(deviceId, new ref PDArpContact(deviceId, "Unknown"));
			}
			
			
			GetGame().GetPlayer().PlaySoundSet(effect, "messagePDA_SoundSet", 0, 0);
			
			ref ChatRoom chatroom = m_rooms.Get(roomId);
			
			if (chatroom == null) {
				chatroom =  new ref ChatRoom(roomId, contact.name, null, null);
				m_rooms.Insert(roomId, chatroom);
			}
			
			chatroom.messages.Insert(msg);

		}
	}

	// Players could have multiple PDAs on their inventory
	ref array<ItemPDA> GetWorkingPDAsOnPlayer(PlayerBase player) {
		array<EntityAI> itemsArray = new array<EntityAI>;
		array<ItemPDA> pdas = new array<ItemPDA>;
		ref ItemPDA item;		

		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
		
		ItemBase itemInHands = player.GetItemInHands();
		if (itemInHands) {
			// This is not very efficient, but must be done so the first element is the one in the hands.
			// TODO: Does EnumerateInventory append to the array or substitutes items already on it?
			//       If that is the case it would be more efficient to add this item first to the array.
			itemsArray.InsertAt(itemInHands, 0);
		}
		
		for (int i = 0; i < itemsArray.Count(); i++)
		{
			item = ItemPDA.Cast(itemsArray.Get(i));

			if (!item)
				continue;

			if (item.IsRuined())
				continue;

			if (!item.HasEnergyManager())
				continue;
			
			if (!item.GetCompEM().CanWork())
				continue;

			pdas.Insert(item);
		}
		
		return pdas;
	}
	
	void AddContact( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target ) {        
        if( type == CallType.Server ) {
			Param2< int, PDArpContact > serverData;
        	if ( !ctx.Read( serverData ) ) return;
			
			int fromDevice = serverData.param1;
			PDArpContact contact = serverData.param2;
			
			auto mem = m_devices.Get(fromDevice);
			
			mem.contacts.Insert(contact.id, contact);
			
			SaveDevices();
        } else {
			if (IsOpen()) {
				m_PDArpMenu.m_addContactStatus = 2;
			}
		}
    }

	bool IsOpen() {
		return m_PDArpMenu && m_PDArpMenu.m_active;
	}

	void Open() {
		if (IsOpen()) {
			Close();
		}
		
		if (GetGame().GetUIManager().GetMenu() != NULL) {
			if (PDArpDebugMode) Print(PDArpModPreffix + "OpenRecipesBookAction ActionCondition blocking by external menu: " + GetGame().GetUIManager().GetMenu());
			return;
		}
		
		if (PDArpDebugMode) Print(PDArpModPreffix + "PluginPDArp prepare open menu");
		m_PDArpMenu = new PDArpMenu;
		m_PDArpMenu.Init();
		GetGame().GetUIManager().ShowScriptedMenu( m_PDArpMenu, NULL );
		if (PDArpDebugMode) Print(PDArpModPreffix + "PluginPDArp post open menu: " + m_PDArpMenu);
	}

	void Close() {
		if (m_PDArpMenu) {
			m_PDArpMenu.m_active = false;
		}

		if (PDArpDebugMode) Print(PDArpModPreffix + "PluginPDArp close menu: " + m_PDArpMenu);
	}

};

class PluginPDArpSettings
{

};