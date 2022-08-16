private const string PDARP_DIR_PATH = "$profile:PDArp";
private const string PDARP_SETTINGS_PATH = PDARP_DIR_PATH + "\\settings.json";
private const string PDARP_DATA_DIR_PATH = PDARP_DIR_PATH + "\\data";
private const string PDARP_STATE_PATH = PDARP_DATA_DIR_PATH + "\\state.json";

class PDArpContact {
	string id;
	string name;
	
	void PDArpContact(string _id, string _name) {
		auto trace = CF_Trace_0(this, "PDArpContact");
		id = _id;
		name = _name;
	}
}

class ChatPreferences {
	string id;
	string name;
	bool muted;
	int unread;
	int lastUpdated;
	
	void ChatPreferences(string _id, string _name, bool _muted) {
		auto trace = CF_Trace_0(this, "ChatPreferences");
		id = _id;
		name = _name;
		muted = _muted;
		unread = 0;
		lastUpdated = CF_Date.Now().GetTimestamp();
	}
}

class ChatMessage {
	int id;
	string sender_id;
	string message;
	int time;

	void ChatMessage(int _id, string deviceId, string txt) {
		auto trace = CF_Trace_0(this, "ChatMessage");
		id = _id;
		sender_id = deviceId;
		message = txt;
		time = CF_Date.Now().GetTimestamp();
	}
}

enum RoomType {
	DirectMessage = 0,
	Group = 1
}

class ChatRoom {
	string id;
	string name;
	RoomType type;
	ref array<string> deviceIds;
	ref array<ref ChatMessage> messages;
	int msgCount;
	
	void ChatRoom(string _id, string _name, array<string> _devices) {
		auto trace = CF_Trace_0(this, "ChatRoom");
		id = _id;
		name = _name;
		deviceIds = _devices;
		if (deviceIds == null) {
			deviceIds = new ref array<string>;
		}
		messages = new ref array<ref ChatMessage>;
		msgCount = 0;
	}

	static ref ChatRoom LoadFromFile(string _id) {
		auto trace = CF_Trace_1("ChatRoom::LoadFromFile").Add(_id);
		string room_path = PDARP_DATA_DIR_PATH + "\\room_" + _id + ".json";
		if (FileExist(room_path)) {
			ref ChatRoom room;
			JsonFileLoader<ref ChatRoom>.JsonLoadFile(room_path, room);
			CF_Log.Debug("Loaded chatroom from file " + _id);
			return room;
		}
		return null;
	}

	void SaveToFile() {
		auto trace = CF_Trace_0(this, "SaveToFile");
		CF_Log.Debug("Saving room " + id);
		string roomPath = PDARP_DATA_DIR_PATH + "\\room_" + id + ".json";
		JsonFileLoader<ref ChatRoom>.JsonSaveFile(roomPath, this);
	}
}

class DeviceMemory {
	string id;
	ref array<ref ChatPreferences> chatRooms;
	ref array<ref PDArpContact> contacts;

	void DeviceMemory(string _id, ref array<ref PDArpContact> _contacts, ref array<ref ChatPreferences> _chatRooms) {
		auto trace = CF_Trace_0(this, "DeviceMemory");
		id = _id;
		contacts = _contacts;
		chatRooms = _chatRooms;
	}

	static ref DeviceMemory LoadFromFile(string _id) {
		auto trace = CF_Trace_1("DeviceMemory::LoadFromFile").Add(_id);

		string deviceContactsPath = PDARP_DATA_DIR_PATH + "\\device_contacts_" + _id + ".json";
		string deviceChatRoomsPath = PDARP_DATA_DIR_PATH + "\\device_chatrooms_" + _id + ".json";

		if (FileExist(deviceContactsPath) && FileExist(deviceChatRoomsPath)) {
			ref array<ref PDArpContact> _contacts;
			JsonFileLoader<ref array<ref PDArpContact>>.JsonLoadFile(deviceContactsPath, _contacts);

			ref array<ref ChatPreferences> _chatRooms;
			JsonFileLoader<ref array<ref ChatPreferences>>.JsonLoadFile(deviceChatRoomsPath, _chatRooms);
			CF_Log.Debug("Loaded device from file " + _id);

			return new ref DeviceMemory(_id, _contacts, _chatRooms);
		}

		return null;
	}

	void SaveToFile() {
		auto trace = CF_Trace_0(this, "SaveToFile");
		string deviceContactsPath = PDARP_DATA_DIR_PATH + "\\device_contacts_" + id + ".json";
		string deviceChatRoomsPath = PDARP_DATA_DIR_PATH + "\\device_chatrooms_" + id + ".json";

		JsonFileLoader<ref array<ref PDArpContact>>.JsonSaveFile(deviceContactsPath, contacts);
		JsonFileLoader<ref array<ref ChatPreferences>>.JsonSaveFile(deviceChatRoomsPath, chatRooms);

		CF_Log.Debug("Saved device to file" + id);
	}

	void SortChatRooms() {
		auto trace = CF_Trace_0(this, "SortChatRooms");
		int i;
		int j;
		int n = chatRooms.Count();
		for (i = 0; i < n; i++) {
			for (j = 0; j < n; j++) {
				if (chatRooms.Get(i).lastUpdated > chatRooms.Get(j).lastUpdated) {
					chatRooms.SwapItems(i, j);				
				}
			}
		}
	}


	ref PDArpContact GetContact(string _id) {
		auto trace = CF_Trace_0(this, "GetContact");
		foreach (auto contact: contacts) {
			if(contact.id == _id) {
				return contact;
			}
		}
		return null;
	}

	ref ChatPreferences GetChatPreferences(string _id) {
		auto trace = CF_Trace_0(this, "GetChatPreferences");
		foreach (auto room: chatRooms) {
			if (room.id == _id) {
				return room;
			}
		}
		return null;
	}
}

class PDArpState {
	int lastId = 1;

	static ref PDArpState LoadFromFile() {
		auto trace = CF_Trace_0("PDArpState::LoadFromFile");
		ref PDArpState state;
		JsonFileLoader<ref PDArpState>.JsonLoadFile(PDARP_STATE_PATH, state);
		return state;
	}

	void SaveToFile() {
		auto trace = CF_Trace_0(this, "SaveToFile");
		JsonFileLoader<ref PDArpState>.JsonSaveFile(PDARP_STATE_PATH, this);
	}
}

class PluginPDArp extends PluginBase
{	
	EffectSound effect;
	
	ref PluginPDArpSettings m_settings;
	ref map<string, ref DeviceMemory> m_devices;
	ref map<string, ref ChatRoom> m_rooms;
	ref PDArpState m_state;
	ref map<string, ItemPDA> m_entities;

	ref PDArpMenu m_PDArpMenu;
	

	void PluginPDArp()
	{
		auto trace = CF_Trace_0(this, "PluginPDArp");
		m_devices = new ref map<string, ref DeviceMemory>;
		m_rooms = new ref map<string, ref ChatRoom>;
		m_entities = new ref map<string, ItemPDA>;

		if (FileExist(PDARP_SETTINGS_PATH)) {
			JsonFileLoader<ref PluginPDArpSettings>.JsonLoadFile(PDARP_SETTINGS_PATH, m_settings);
		} else {
			m_settings = new PluginPDArpSettings;
			JsonFileLoader<ref PluginPDArpSettings>.JsonSaveFile(PDARP_SETTINGS_PATH, m_settings);
		}
		CF_Log.Level = m_settings.logLevel;
	
        if (GetGame().IsServer()) {

			if (!FileExist(PDARP_DIR_PATH)){
				MakeDirectory(PDARP_DIR_PATH);
			}

			if (!FileExist(PDARP_DATA_DIR_PATH)){
				MakeDirectory(PDARP_DATA_DIR_PATH);
			}

			if (FileExist(PDARP_STATE_PATH)) {
                m_state = PDArpState.LoadFromFile();
            } else {
                m_state = new PDArpState();
                m_state.SaveToFile();
            }

        }

        CF_Log.Debug("PluginPDArp constructor");
	}
	
	override void OnInit(){
		auto trace = CF_Trace_0(this, "OnInit");

		GetRPCManager().AddRPC( PDArpModPreffix, "AddContact", this, SingleplayerExecutionType.Both ); 
		GetRPCManager().AddRPC( PDArpModPreffix, "SendMessage", this, SingleplayerExecutionType.Both );
		GetRPCManager().AddRPC( PDArpModPreffix, "GetDeviceMemory", this, SingleplayerExecutionType.Both );
		GetRPCManager().AddRPC( PDArpModPreffix, "GetChatRoom", this, SingleplayerExecutionType.Both);
		GetRPCManager().AddRPC( PDArpModPreffix, "MsgAck", this, SingleplayerExecutionType.Both);
		GetRPCManager().AddRPC( PDArpModPreffix, "RenameChat", this, SingleplayerExecutionType.Both);
		GetRPCManager().AddRPC( PDArpModPreffix, "ShowError", this, SingleplayerExecutionType.Both);
		GetRPCManager().AddRPC( PDArpModPreffix, "ToggleMute", this, SingleplayerExecutionType.Both);
		CF_Log.Debug("PluginPDArp OnInit");
	}

	int GenerateMemoryIdentifier() {
		auto trace = CF_Trace_0(this, "GenerateMemoryIdentifier");
		int n = m_state.lastId + 1;
		m_state.lastId = n;
		m_state.SaveToFile();
		return n;
	}

	void ShowError( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target ) {
		auto trace = CF_Trace_0(this, "ShowError");

		if (GetGame().IsClient() && m_PDArpMenu) {
			Param1<string> clientData;			
			if ( !ctx.Read( clientData ) ) return;
			m_PDArpMenu.ShowError(clientData.param1);
		}
	}
	
	void MsgAck( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target ) {
		auto trace = CF_Trace_0(this, "MsgAck");
		if (GetGame().IsServer()) {
			Param2<string, string> serverData;			
			if ( !ctx.Read( serverData ) ) return;		
			string deviceId = serverData.param1;
			string roomId = serverData.param2;

			auto mem = m_devices.Get(deviceId);
			auto room = mem.GetChatPreferences(roomId);
			room.unread = 0;
			mem.SaveToFile();
		}
	}

	void ToggleMute( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target ) {
		auto trace = CF_Trace_0(this, "ToggleMute");

		if (GetGame().IsServer()) {
			Param2<string, string> serverData;			
			if ( !ctx.Read( serverData ) ) return;		
			string deviceId = serverData.param1;
			string roomId = serverData.param2;

			auto mem = m_devices.Get(deviceId);
			auto room = mem.GetChatPreferences(roomId);
			room.muted = !room.muted;
			mem.SaveToFile();
		}
	}

	void GetDeviceMemory( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target ) {
		auto trace = CF_Trace_0(this, "GetDeviceMemory");
		ref DeviceMemory mem;
		if (GetGame().IsServer()) {
			Param1< string > serverData;			
			if ( !ctx.Read( serverData ) ) return;

			string deviceId = serverData.param1;
			CF_Log.Debug("Client " + sender.GetPlainId() + " is requestiog DeviceMemory for " + deviceId);

			mem = m_devices.Get(deviceId);
			if (!mem) {
				mem = DeviceMemory.LoadFromFile(deviceId);
			}

			if (!mem) {
				CF_Log.Info("Creating new device memory for device " + deviceId);
				mem = new ref DeviceMemory(deviceId, new array<ref PDArpContact>, new array<ref ChatPreferences>);
				m_devices.Insert(deviceId, mem);
				mem.SaveToFile();
			}
			
			m_devices.Insert(mem.id, mem);

			foreach(auto roomPref: mem.chatRooms) {
				if (m_rooms.Get(roomPref.id) == null) {
					ChatRoom room = ChatRoom.LoadFromFile(roomPref.id);
					m_rooms.Insert(room.id, room);
				}
			}

			GetRPCManager().SendRPC( PDArpModPreffix, "GetDeviceMemory", new Param1<DeviceMemory>( mem ), true, sender );
		}
		
		if (GetGame().IsClient()) {
			Param1< DeviceMemory > clientData;
			if ( !ctx.Read( clientData ) ) return;
			
			mem = clientData.param1;
			CF_Log.Debug("Received memory " + mem.id);

			m_devices.Set(mem.id, mem);
			foreach(auto roomPref1: mem.chatRooms) {
				if (m_rooms.Get(roomPref1.id) == null) {
					GetRPCManager().SendRPC( PDArpModPreffix, "GetChatRoom", new Param1<string>( roomPref1.id ), true );
				}
			}

			if(IsOpen()) {
				m_PDArpMenu.m_dirty = true;
			}
		}
	}
	
	void GetChatRoom( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target ) {
		auto trace = CF_Trace_0(this, "GetChatRoom");

		ChatRoom room;		
		if (GetGame().IsServer()) {
			Param1< string > serverData;			
			if ( !ctx.Read( serverData ) ) return;

			room = m_rooms.Get(serverData.param1);
			CF_Log.Debug("Client " + sender.GetPlainId() + " requesting chat room " + serverData.param1);
			GetRPCManager().SendRPC( PDArpModPreffix, "GetChatRoom", new Param1<ChatRoom>( room ), true, sender );
		}

		if (GetGame().IsClient()) {
			Param1< ChatRoom > clientData;
			if ( !ctx.Read( clientData ) ) return;
			
			room = clientData.param1;
			CF_Log.Debug("Received room " + room.id);

			m_rooms.Set(room.id, room);

			if (IsOpen()) {
				m_PDArpMenu.m_dirty = true;
			}
		}
	}

	void RenameChat(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target ) {
		auto trace = CF_Trace_0(this, "RenameChat");

		string deviceId;
		string roomId;

		if (GetGame().IsServer()) {
			Param3<string, string, string > serverData;			
			if ( !ctx.Read( serverData ) ) return;
			deviceId = serverData.param1;
			roomId = serverData.param2;
			string newName = serverData.param3;

			auto device = m_devices.Get(deviceId);
			auto roomPrefs = device.GetChatPreferences(roomId);
			auto room = m_rooms.Get(roomId);
			if (room.type == RoomType.DirectMessage) { // Direct messages changes the name of the contact.
				string contactId;
				foreach(auto _id: room.deviceIds) {
					if (_id != deviceId) {
						contactId = _id;
						break;
					}
				}
				auto contact = device.GetContact(contactId);
				contact.name = newName;
				device.SaveToFile();
				GetRPCManager().SendRPC( PDArpModPreffix, "GetDeviceMemory", new Param1<DeviceMemory>( device ), true, sender );
			} else { // If its a group the name changes for everyone
				room.name = newName;
				room.SaveToFile();
				GetRPCManager().SendRPC( PDArpModPreffix, "GetChatRoom", new Param1<ChatRoom>( room ), true, sender );
			}
		}


	}

	void SendMessage( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target ) {
		auto trace = CF_Trace_0(this, "SendMessage");

		string deviceId;
		string roomId;
		ref array<ItemPDA> pdas;
		ItemPDA entity;

		if (GetGame().IsServer()) {
			Param3<string, string, ChatMessage > serverData;			
			if ( !ctx.Read( serverData ) ) return;
			
			deviceId = serverData.param1;
			roomId = serverData.param2;
			ref ChatMessage message = serverData.param3;
	
			CF_Log.Debug("Client " + sender.GetPlainId() + " sending message from device " + deviceId + " to chat room " + roomId);

			auto room = m_rooms.Get(roomId);
			if (room != null) {
				auto txDevice = m_devices.Get(deviceId);
				auto txPrefs = txDevice.GetChatPreferences(roomId);
				txPrefs.unread = 0;
				txPrefs.lastUpdated = message.time;
				txDevice.SaveToFile();
				room.messages.Insert(message);
				foreach(auto participant: room.deviceIds) {	
					auto rxDevice = m_devices.Get(participant);
					auto rxPrefs = rxDevice.GetChatPreferences(roomId);
					if (rxPrefs) {
						if (rxDevice.id != deviceId) {
							rxPrefs.unread = rxPrefs.unread + 1;
							rxPrefs.lastUpdated = message.time;
							rxDevice.SaveToFile();
						}
						entity = m_entities.Get(participant);
						if (!rxPrefs.muted && entity.CanWorkAsPDA()) {
							auto players = GetPlayersCloseToPDA(participant);
							foreach(auto player: players) {
								auto playerIdentity = player.GetIdentity();
								CF_Log.Trace("Broadcasting message to " + playerIdentity.GetPlainId());
								// This should play a sound on the client.
								GetRPCManager().SendRPC( PDArpModPreffix, "SendMessage", new Param3<string, string, ChatMessage>( participant, roomId, message ), true, playerIdentity);
							}
						}
					}
				}
				room.msgCount = room.msgCount + 1;
				room.SaveToFile();		
			}
			
		}

		if (GetGame().IsClient()) {
			Param3<string, string, ChatMessage > clientData;			
			if ( !ctx.Read( clientData ) ) return;
			
			deviceId = clientData.param1;
			roomId = clientData.param2;
			ChatMessage msg = clientData.param3;
			
			CF_Log.Debug("Received message for " + deviceId + " on room " + roomId);


			auto gamePlayer = GetGame().GetPlayer();
			player = PlayerBase.Cast(gamePlayer);

			pdas = GetWorkingPDAsOnPlayer(player);
			DeviceMemory mem;
			
			foreach(auto p: pdas) {
				if (p.GetMemoryID() == deviceId){
					mem = m_devices.Get(p.GetMemoryID());
					break;
				}
			}

			if (mem && msg.sender_id != mem.id) {
				auto roomPref = mem.GetChatPreferences(roomId);
				if (roomPref) {
					roomPref.unread = roomPref.unread + 1;
					roomPref.lastUpdated = msg.time;
				}
			}

			if (!mem || !m_PDArpMenu || m_PDArpMenu.m_pdaId != mem.id) {
				entity = m_entities.Get(deviceId);
				if (entity)
					entity.PlaySoundSet(effect, "messagePDA_SoundSet", 0, 0);
			}
			
			ref ChatRoom chatroom = m_rooms.Get(roomId);
			if (chatroom && msg.sender_id != mem.id) {
				if (chatroom.msgCount < msg.id) { // avoid dupes
					chatroom.messages.Insert(msg);
					chatroom.msgCount = msg.id;
				}
			}

			if (msg.sender_id != mem.id && m_PDArpMenu && m_PDArpMenu.m_pdaId == deviceId) {
				m_PDArpMenu.m_dirty = true;
			}
		}
	}

	// Players could have multiple PDAs on their inventory
	ref array<ItemPDA> GetWorkingPDAsOnPlayer(PlayerBase player) {
		auto trace = CF_Trace_0(this, "GetWorkingPDAsOnPlayer");

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

			if (!item || !item.CanWorkAsPDA())
				continue;

			pdas.Insert(item);
		}
		
		return pdas;
	}
	
	ref array<PlayerBase> GetPlayersCloseToPDA(string pdaId, int distance = 50) {
		auto trace = CF_Trace_0(this, "GetPlayersCloseToPDA");

		ref array<PlayerBase> players = new ref array<PlayerBase>;
		ref array<Man> people = new ref array<Man>;
		GetGame().GetPlayers(people);
		
		auto pda = m_entities.Get(pdaId);
		auto position = pda.GetPosition();
		
		foreach(auto p: people) {
			ref PlayerBase player = PlayerBase.Cast(p);
			if (Math.IsPointInCircle(position, distance, player.GetPosition())) {
				players.Insert(player);
			}
			// TODO: Check around the player for pdas.
		}
		
		return players;
	}
	
	void AddContact( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target ) {
		auto trace = CF_Trace_0(this, "AddContact");

        if( type == CallType.Server ) {
			Param2< string, PDArpContact > serverData;
        	if ( !ctx.Read( serverData ) ) return;
			
			string fromDevice = serverData.param1;
			PDArpContact contact = serverData.param2;
			
			auto mem1 = m_devices.Get(fromDevice);
			auto mem2 = m_devices.Get(contact.id);

			// TODO: Show error in UI
			if (mem1 == null || mem2 == null) {
				GetRPCManager().SendRPC( PDArpModPreffix, "ShowError", new Param1<string>( "The id doesn't match an existing device" ), true, sender );
				return;
			};

			auto players = GetPlayersCloseToPDA(contact.id, 15);
			bool isTooFar = true;
			foreach(auto closePlayer: players) {
				if (closePlayer.GetIdentity().GetPlainId() == sender.GetPlainId()) {
					isTooFar = false;
				}
			}

			if (isTooFar) {
				GetRPCManager().SendRPC( PDArpModPreffix, "ShowError", new Param1<string>( "You must be within 15m of the contact's device"), true, sender );
				return;
			}

			CF_Log.Debug("Client " + sender.GetPlainId() + " adding contact " + contact.id + " to device " + fromDevice);

			mem1.contacts.Insert(contact);
			mem2.contacts.Insert(new ref PDArpContact(fromDevice, "Unknown"));

			// TODO: Better way to create room identifiers.
			CF_TextReader reader = new CF_TextReader(new CF_StringStream(fromDevice + contact.id));
			CF_Base16Stream output = new CF_Base16Stream();
			CF_SHA256.Process(reader, output);
			string roomId = output.Encode().Substring(0, 8);
			roomId.ToLower();

			ref array<string> devices = new ref array<string>;
			devices.Insert(fromDevice);
			devices.Insert(contact.id);
			ChatRoom room = new ChatRoom(roomId, roomId, devices);
			m_rooms.Insert(roomId, room);

			auto preferences = new ref ChatPreferences(room.id, room.name, false);
			mem1.chatRooms.Insert(preferences);
			mem2.chatRooms.Insert(preferences);

			mem1.SaveToFile();
			mem2.SaveToFile();
			room.SaveToFile();

			// Update sender's pda
			GetRPCManager().SendRPC( PDArpModPreffix, "GetDeviceMemory", new Param1<DeviceMemory>( mem1 ), true, sender );
			GetRPCManager().SendRPC( PDArpModPreffix, "GetChatRoom", new Param1<ChatRoom>( room ), true, sender );
			GetRPCManager().SendRPC( PDArpModPreffix, "AddContact", null, true, sender );

			foreach(auto p: players) {
				auto pdas = GetWorkingPDAsOnPlayer(p);
				foreach (auto pda: pdas) {
					if (pda.GetMemoryID() == mem2.id) {
						auto identity = p.GetIdentity();
						GetRPCManager().SendRPC( PDArpModPreffix, "GetDeviceMemory", new Param1<DeviceMemory>( mem2 ), true, identity );
						GetRPCManager().SendRPC( PDArpModPreffix, "GetChatRoom", new Param1<ChatRoom>( room ), true, identity );
					}
				}
			}
        } else {
			m_PDArpMenu.ResetView();
		}
    }

	bool IsOpen() {
		return m_PDArpMenu && m_PDArpMenu.m_active;
	}


	void Open() {
		auto trace = CF_Trace_0(this, "Open");

		if (IsOpen()) {
			Close();
		}
		
		if (GetGame().GetUIManager().GetMenu() != NULL) {
			CF_Log.Debug("PluginPDArp OpenRecipesBookAction ActionCondition blocking by external menu: " + GetGame().GetUIManager().GetMenu());
			return;
		}
		CF_Log.Debug("PluginPDArp prepare open menu");
		m_PDArpMenu = new PDArpMenu;
		m_PDArpMenu.Init();
		GetGame().GetUIManager().ShowScriptedMenu( m_PDArpMenu, NULL );
		CF_Log.Debug("PluginPDArp post open menu " + m_PDArpMenu);
	}

	void Close() {
		auto trace = CF_Trace_0(this, "Close");

		if (m_PDArpMenu) {
			m_PDArpMenu.m_active = false;
		}
		CF_Log.Debug("PluginPDArp close menu " + m_PDArpMenu);
	}

};

class PluginPDArpSettings {
	CF_LogLevel logLevel = CF_LogLevel.INFO;
};