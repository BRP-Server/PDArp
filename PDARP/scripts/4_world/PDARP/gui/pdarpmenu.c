class PDArpMenu extends UIScriptedMenu
{
	bool m_active = false;
	bool m_dirty = false;
	
	const int m_contactMaxLength = 32;
	const int m_messageMaxLength = 256;
	int m_lastSelectedChatIdx = -1;
	bool m_isRenaming = false;

	bool m_externalSendEvent = false;

	string m_pdaId;

	// Top bar
	ref TextWidget m_yourIdTxt;
	
	// Room panel
	ref Widget m_chatRoomPanel;
	ref TextListboxWidget m_chatRoomList;
	ref ButtonWidget m_newContactBtn;

	// Add contact panel
	ref Widget m_addContactPanel;
	ref EditBoxWidget m_addContactIdInput;
	ref EditBoxWidget m_addContactNameInput;
	ref ButtonWidget m_addContactSubmitBtn;
	ref ButtonWidget m_addContactCancelBtn;

	// Chat panel
	ref Widget m_chatPanel;
	ref TextWidget m_roomNameTxt;
	ref ButtonWidget m_renameRoomBtn;
	ref ButtonWidget m_muteRoomBtn;
	ref EditBoxWidget m_renameRoomInput;

	ref TextListboxWidget m_messagesList;
	ref EditBoxWidget m_sendMsgInput;
	ref ButtonWidget m_sendMsgBtn;


	void PDArpMenu()
	{				
		if (PDArpDebugMode) Print(PDArpModPreffix + "PDArpMenu construct");
	}	
	
	void ~PDArpMenu()
	{
		if (PDArpDebugMode) Print(PDArpModPreffix + "PDArpMenu destruct");
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		player.GetInputController().SetDisabled(false);
	}

    override Widget Init()
    {
		if (PDArpDebugMode) Print(PDArpModPreffix + "PDArpMenu init");
		if (layoutRoot == null)
			layoutRoot = GetGame().GetWorkspace().CreateWidgets( "PDArp/scripts/layouts/PDArpMenu.layout" );

		// Top bar
		m_yourIdTxt = TextWidget.Cast( layoutRoot.FindAnyWidget( "your_id_txt" ) );

		// Room panel
		m_chatRoomPanel = Widget.Cast(layoutRoot.FindAnyWidget("chatroom_panel"));
        m_chatRoomList = TextListboxWidget.Cast( layoutRoot.FindAnyWidget( "chatroom_list" ) );
		m_newContactBtn = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "new_contact_btn" ) );

		// Add contact panel
		m_addContactPanel = Widget.Cast(layoutRoot.FindAnyWidget("add_contact_panel"));
		m_addContactIdInput = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("add_contact_id_input"));
		m_addContactNameInput = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("add_contact_name_input"));
		m_addContactSubmitBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("add_contact_submit_btn"));
		m_addContactCancelBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("add_contact_cancel_btn"));

		// Chat panel
		m_chatPanel = Widget.Cast(layoutRoot.FindAnyWidget("chat_panel"));
		m_roomNameTxt = TextWidget.Cast(layoutRoot.FindAnyWidget("room_name_txt"));
		m_renameRoomBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("rename_room_btn"));
		// m_addParticipantBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("add_participant_btn"));
		m_muteRoomBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("mute_room_btn"));
		m_renameRoomInput = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("rename_room_input"));

		m_messagesList = TextListboxWidget.Cast( layoutRoot.FindAnyWidget("messages_list"));
		m_sendMsgInput = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("send_msg_input"));
		m_sendMsgBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("send_msg_btn"));
				
		PluginPDArp pluginPDArp;
		Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
		
		if (!m_active) 
		{
			Man man = GetGame().GetPlayer();
			ref PlayerBase player = PlayerBase.Cast(man);
			auto pdas = pluginPDArp.GetWorkingPDAsOnPlayer(player);
			auto pda = pdas.Get(0);
			m_pdaId = pda.GetMemoryID();
			GetRPCManager().SendRPC( PDArpModPreffix, "GetDeviceMemory", new Param1<string>( m_pdaId ), true );
			m_yourIdTxt.SetText("#pda_loading");
		}
		ResetView();
		m_dirty = true;
		m_active = true;
        return layoutRoot;
    }
	
	void FillContactsList() {
		if (PDArpDebugMode) Print(PDArpModPreffix + "FillContactsList");
		
		PluginPDArp pluginPDArp;
		Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
		
        int itemId;
		m_chatRoomList.ClearItems();

		auto mem = pluginPDArp.m_devices.Get(m_pdaId);

		ChatRoom chatRoom;
		if (mem != null) {
			// TODO: Sort chatRooms by last message date desc
			foreach(auto chatRoomPref: mem.chatRooms) {
				chatRoom = pluginPDArp.m_rooms.Get(chatRoomPref.id);
				if (chatRoom) {
					string roomName;
					if (chatRoom.type == RoomType.DirectMessage) {
						string cId;
						foreach(auto _id: chatRoom.deviceIds) {
							if (_id != m_pdaId) {
								cId = _id;
							}
						}
						auto contact = mem.GetContact(cId);
						roomName = contact.name;
					} else {
						roomName = chatRoom.name;
					}
					
					if (chatRoomPref.unread > 0) {
						roomName = roomName + " (" + chatRoomPref.unread + ")";
					}
					
					itemId = m_chatRoomList.AddItem(roomName, NULL, 0);
					
					if (chatRoomPref.unread > 0) {
						m_chatRoomList.SetItemColor(itemId, 0, ARGB(255, 62, 189, 71));
					} else {
						m_chatRoomList.SetItemColor(itemId, 0, ARGB(255, 255, 255, 255));
					}
				}
			}
		}
		
		SelectConversation(m_lastSelectedChatIdx);
	}
	
	void SelectConversation(int id) {
		if (PDArpDebugMode) Print(PDArpModPreffix + "SelectConversation: " + id);

		PluginPDArp pluginPDArp;
		Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
		ref array<ref Param2<string, string>> confMessages = null;

		if ( id == -1 ) {
			if (PDArpDebugMode) Print(PDArpModPreffix + "No need to select conversation: " + id);
			m_messagesList.ClearItems();
			m_chatPanel.Show(false, true);
			return;
		} else {
			OpenChatMessages();
		}
		
		auto mem = pluginPDArp.m_devices.Get(m_pdaId);
		ChatPreferences selectedChat = mem.chatRooms.Get(id);
		
		m_messagesList.ClearItems();
		
		float chatWidth;
		float chatHeight;
		m_messagesList.GetScreenSize(chatWidth, chatHeight);
		int chatLineMaxSize = (int)(chatWidth * 0.85) - 50;
		string autor;	
		int color;	
		int itemId;
		int addedLinesCount = 0;
		bool isLineFinished;
		int i = 0;
		int q = 0;
		int textWidthCalibration;
		int textHeightCalibration; 


		int unread = selectedChat.unread;
		
		ref ChatRoom room = pluginPDArp.m_rooms.Get(selectedChat.id);
		int messagesRead = room.messages.Count() - unread;
		string roomName;
		if (room.deviceIds.Count() == 2) {
			string contactId;
			foreach(auto _id: room.deviceIds) {
				if (_id != m_pdaId) {
					contactId = _id;
					break;
				}
			}

			roomName = mem.GetContact(contactId).name;
		} else {
			roomName = selectedChat.name;
		}

		m_roomNameTxt.SetText(roomName);
		int msgIdx;
		for(msgIdx = 0; msgIdx < room.messages.Count(); msgIdx++) {
			auto message = room.messages.Get(msgIdx);
			if (messagesRead != -1 && msgIdx >= messagesRead) {
				itemId = m_messagesList.AddItem("--- New Messages ---", NULL, 1);
				m_messagesList.SetItemColor(itemId, 1, ARGB(255, 156, 156, 156));
				messagesRead = -1;
			}

			if (message.sender_id == m_pdaId) {
				autor = "Me";
				color = ARGBF(1, 0.2, 0.8, 0.2);
			}
			else {
				PDArpContact contact = mem.GetContact(message.sender_id);
				
				autor = contact.name;
				color = ARGBF(1, 0.976, 1, 0.298);
			}
						
			itemId = m_messagesList.AddItem(autor, NULL, 0);
			m_messagesList.SetItemColor(itemId, 0, color);
			
			m_messagesList.SetItem(itemId, message.message, NULL, 1);
		}
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Call(ScrollToLastMessage);
	}
	
	void ScrollToLastMessage() {
		auto count = m_messagesList.GetNumItems();
		m_messagesList.SelectRow(count - 1);
		m_messagesList.EnsureVisible(count - 1);
	}
	
	override void Update(float timeslice)
	{
		super.Update(timeslice);
		PluginPDArp pluginPDArp;
		Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
		ChatPreferences lastSelectedChat;

		if (m_externalSendEvent) {
			if (m_renameRoomInput.IsVisibleHierarchy()) {
				SendRenameEvent();
			} else if (m_sendMsgInput.IsVisibleHierarchy()) {
				SendMessageEvent();
			}
			m_externalSendEvent = false;
		}

		int currentlySelectedChat = m_chatRoomList.GetSelectedRow();

		if (m_lastSelectedChatIdx != -1 && currentlySelectedChat == -1) {
			m_chatRoomList.SelectRow(m_lastSelectedChatIdx);
			currentlySelectedChat = m_lastSelectedChatIdx;
		}

		if ( m_lastSelectedChatIdx != currentlySelectedChat) {
			lastSelectedChat = pluginPDArp.m_devices.Get(m_pdaId).chatRooms.Get(m_lastSelectedChatIdx);
			m_lastSelectedChatIdx = m_chatRoomList.GetSelectedRow();
			if (lastSelectedChat && lastSelectedChat.unread > 0) {
				lastSelectedChat.unread = 0;
				GetRPCManager().SendRPC( PDArpModPreffix, "MsgAck", new Param2<string, string>( m_pdaId, lastSelectedChat.id ), true );
				FillContactsList();
			} else {
				SelectConversation(m_lastSelectedChatIdx);
			}
		}
		
		if (m_dirty) {
			if (PDArpDebugMode) Print(PDArpModPreffix + "SelectConversation by dirty: X6");

			auto mem = pluginPDArp.m_devices.Get(m_pdaId);
			m_yourIdTxt.SetText("#pda_user_id " + m_pdaId);		
			
			if (mem != null) {
				FillContactsList();			
			}
			m_dirty = false;
		}
		
		if (!m_active) {
			if (m_lastSelectedChatIdx == currentlySelectedChat) {
				lastSelectedChat =pluginPDArp.m_devices.Get(m_pdaId).chatRooms.Get(m_lastSelectedChatIdx);
				if (lastSelectedChat && lastSelectedChat.unread > 0) {
					lastSelectedChat.unread = 0;
					GetRPCManager().SendRPC( PDArpModPreffix, "MsgAck", new Param2<string, string>( m_pdaId, lastSelectedChat.id ), true );
				}
			}
			GetGame().GetUIManager().Back();
		}
	}

	override void OnShow()
	{
		super.OnShow();

		PPEffects.SetBlurMenu(0.5);

		GetGame().GetInput().ChangeGameFocus(1);

		SetFocus( layoutRoot );
		
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		player.GetInputController().SetDisabled(true);
		player.GetActionManager().EnableActions(false);
	}

	override void OnHide()
	{
		super.OnHide();

		PPEffects.SetBlurMenu(0);

		GetGame().GetInput().ResetGameFocus();
		
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		player.GetInputController().SetDisabled(false);
		player.GetActionManager().EnableActions(true);
		
		Close();
	}

	override bool OnKeyPress(Widget w, int x, int y, int key) {
		if (PDArpDebugMode) Print(PDArpModPreffix + "OnKeyPress: " + w.GetName());
		
		string boxText;
		if (w.GetName() == m_addContactIdInput.GetName()) {
			boxText = m_addContactIdInput.GetText();
			if (boxText.LengthUtf8() >= m_contactMaxLength) {
				return true;
			}
		}
		
		if (w.GetName() == m_addContactNameInput.GetName()) {
			boxText = m_addContactNameInput.GetText();
			if (boxText.LengthUtf8() >= m_contactMaxLength) {
				return true;
			}
		}
		if (w.GetName() == m_sendMsgInput.GetName()) {
			boxText = m_sendMsgInput.GetText();
			if (boxText.LengthUtf8() >= m_messageMaxLength) {
				return true;
			}
		}
		return false;
	}

	bool SendMessageEvent() {
		PluginPDArp pluginPDArp;
		Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));

		auto mem = pluginPDArp.m_devices.Get(m_pdaId);
		
		string message = m_sendMsgInput.GetText();
		if (message.LengthUtf8() > 0) {
			if (message.LengthUtf8() > m_messageMaxLength) {
				message = message.Substring(0, m_messageMaxLength);
			}
			ChatPreferences roomPrefs = mem.chatRooms.Get(m_lastSelectedChatIdx);
			if (roomPrefs.unread > 0) {
				roomPrefs.unread = 0;
			}
			GetRPCManager().SendRPC( PDArpModPreffix, "SendMessage", new Param3<string, string, string>( m_pdaId, roomPrefs.id, message ), true );
			m_sendMsgInput.SetText("");
			return true;
		}
		
		return false;
	}
	
	bool SendRenameEvent() {
		PluginPDArp pluginPDArp;
		Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
		auto mem = pluginPDArp.m_devices.Get(m_pdaId);
		string newName = m_renameRoomInput.GetText();
		
		newName = newName.Trim();
		
		if (newName.LengthUtf8() > 0) {
			ChatPreferences roomPrefs = mem.chatRooms.Get(m_lastSelectedChatIdx);
			GetRPCManager().SendRPC(PDArpModPreffix, "RenameChat", new Param3<string, string, string>(m_pdaId, roomPrefs.id, newName));
			CloseRenameInput();
			return true;
		} else {
			ShowError("A name must not be empty");
			return false;
		}
	}
	
	void ResetTitleText() {
		if (m_pdaId) {
			m_yourIdTxt.SetText("#pda_user_id " + m_pdaId);
		} else {
			m_yourIdTxt.SetText("#pda_loading");
		}
		m_yourIdTxt.SetColor( ARGB(255, 255, 255, 255));
	}

	void ShowError(string txt) {
		m_yourIdTxt.SetText(txt);
		m_yourIdTxt.SetColor( ARGB(255, 255, 51, 51));
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ResetTitleText, 1500);
	}

	bool OnAddContactSubmit() {
		PluginPDArp pluginPDArp;
		Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));

		string contactId = m_addContactIdInput.GetText();

		if (contactId) {
			contactId = contactId.Trim();
		}

		if (!contactId || contactId == "" || contactId.Length() != 8) {
			ShowError("You must provide a valid contact ID");
			return false;
		}

		auto pdaMem = pluginPDArp.m_devices.Get(m_pdaId);
		
		auto contact = pdaMem.GetContact(contactId);

		if(contact) {
			ShowError("You already have a contact with this id - " + contact.name);
			return false;
		}

		string contactName = m_addContactNameInput.GetText();
		if (contactName) {
			contactName = contactName.Trim();
		}

		if (!contactName || contactName == "") {
			contactName = "Unknown";
		}

		contact = new PDArpContact(contactId, contactName);
		GetRPCManager().SendRPC( PDArpModPreffix, "AddContact", new Param2<string, PDArpContact>( m_pdaId, contact), true );
		return true;
	}
	
	override bool OnClick(Widget w, int x, int y, int button) {
		super.OnClick(w, x, y, button);
		
		if (PDArpDebugMode) Print(PDArpModPreffix + "OnClick: " + w.GetName());
		
		if (button == MouseState.LEFT)
		{
			int rowShift;
			int selectedRow;
			PluginPDArp pluginPDArp;
			Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
			
			if (w == m_addContactSubmitBtn) {
				return OnAddContactSubmit();
			}
			
			if (w == m_addContactCancelBtn) {
				ResetView();
				return true;
			}

			if (w == m_renameRoomBtn) {
				m_isRenaming = !m_isRenaming;
				if (m_isRenaming) {
					OpenRenameInput();
				} else {
					CloseRenameInput();
				}
				return true;
			}
			
			if (w == m_newContactBtn) {
				OpenAddContact();
				return true;
			}
			
			if (w == m_sendMsgBtn) {
				return SendMessageEvent();
			}

		}
		
		return false;
	}
	
	void OpenRenameInput() {
		m_renameRoomBtn.SetText("Cancel");
		m_roomNameTxt.Show(false, true);
		m_renameRoomInput.Show(true, true);
	}
	
	void CloseRenameInput() {
		m_renameRoomBtn.SetText("Rename");
		m_renameRoomInput.Show(false, false);
		m_roomNameTxt.Show(true, true);
	}
	
	void OpenAddContact() {
		m_addContactPanel.Show(true, true);
		m_chatRoomPanel.Show(false, true);
	}

	void OpenChatMessages() {
		m_chatPanel.Show(true, true);
	}
	
	void ResetView() {
		m_addContactPanel.Show(false, true);
		m_chatRoomPanel.Show(true, true);
		m_chatPanel.Show(false, true);
	}

	override bool OnChange(Widget w, int x, int y, bool finished) {
		super.OnChange(w, x, y, finished);
		
		if (PDArpDebugMode) Print(PDArpModPreffix + "OnChange: " + w.GetName());
		
		string boxText;
		if (w.GetName() == m_addContactIdInput.GetName()) {
			return true;
			// TODO: Handle validation error.
		}

		if (w.GetName() == m_addContactNameInput.GetName()) {
			return true;
		}
		
		if (w.GetName() == m_renameRoomInput.GetName()) {
			return true;
		}

		if (w.GetName() == m_sendMsgInput.GetName()) {
			boxText = m_sendMsgInput.GetText();
			if (boxText.LengthUtf8() > m_messageMaxLength) {
				m_sendMsgInput.SetText(boxText.Substring(0, m_messageMaxLength));
			}
			return true;
		}
		return false;
	}
	
	override bool OnItemSelected(Widget w, int x, int y, int row, int  column,	int  oldRow, int  oldColumn)
	{
		super.OnItemSelected(w, x, y, row, column, oldRow, oldColumn);
		
		if (PDArpDebugMode) Print(PDArpModPreffix + "OnItemSelected: " + w.GetName());
		
		if (w == m_chatRoomList)
		{
			return true;
		}
		
		if (w == m_messagesList)
		{
			return true;
		}
		
		return false;
	}
}