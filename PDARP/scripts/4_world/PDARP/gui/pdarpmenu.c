class PDArpMenu extends UIScriptedMenu
{
	bool m_active = false;
	bool m_dirty = false;
	
	const int m_contactMaxLength = 32;
	const int m_messageMaxLength = 256;
		
	int m_lastSelectedContact = -1;
	
	bool m_externalSendEvent = false;
	bool m_sendFuncEnabled = true;

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
	ref TextWidget m_roomNameTxt;
	ref ButtonWidget m_renameRoomBtn;
	ref ButtonWidget m_muteRoomBtn;

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
		m_roomNameTxt = TextWidget.Cast(layoutRoot.FindAnyWidget("room_name_txt"));
		m_renameRoomBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("rename_room_btn"));
		// m_addParticipantBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("add_participant_btn"));
		m_muteRoomBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("mute_room_btn"));

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
			if (pda != null && pluginPDArp.m_devices.Get(m_pdaId) == null) {
				// Request PDA memory if not present in the client.
				GetRPCManager().SendRPC( PDArpModPreffix, "GetDeviceMemory", new Param1<string>( m_pdaId ), true );
			}
			m_yourIdTxt.SetText("#pda_loading");
			m_sendMsgInput.Enable(false);
			m_sendMsgBtn.Enable(false);
			m_sendFuncEnabled = false;
		}
		m_dirty = true;
		m_active = true;
        return layoutRoot;
    }
	
	void FillContactsList() {
		if (PDArpDebugMode) Print(PDArpModPreffix + "FillContactsList");
		
		PluginPDArp pluginPDArp;
		Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
		
        int itemId;
        int rowShift = 0;
		int selectedRow = m_lastSelectedContact;
		m_chatRoomList.ClearItems();

		auto mem = pluginPDArp.m_devices.Get(m_pdaId);

		ChatRoom chatRoom;
		if (mem != null) {
			foreach(auto chatRoomPref: mem.chatRooms) {
				chatRoom = pluginPDArp.m_rooms.Get(chatRoomPref.id);
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
				itemId = m_chatRoomList.AddItem(roomName, NULL, 0);

				// TODO: Not sure how I will deal with the unread stuff
				// m_chatRoomList.SetItem(itemId, "" + chatRoom.unreaded, NULL, 1);
				
				// TODO: Not sure how I will deal with the original ban & online users feature.
				// if (contact.m_IsBanned) {
				// 	m_chatRoomList.SetItemColor(itemId, 0, ARGBF(1, 0.8, 0.02, 0.02));
				// }
				// else if (pluginPDArp.m_onlineContacts.Find(contact.m_UID) != -1)
				// {
				// 	m_chatRoomList.SetItemColor(itemId, 0, ARGBF(1, 0.2, 0.8, 0.02));
				// }
				
				m_chatRoomList.SetItemColor(itemId, 0, ARGB(255, 255, 255, 255));

			}
		}
		
		if (selectedRow >= 0 && selectedRow < mem.chatRooms.Count() + rowShift) {
			if (PDArpDebugMode) Print(PDArpModPreffix + "SelectConversation: X1 " + selectedRow);
			m_lastSelectedContact = selectedRow;
			m_chatRoomList.SelectRow(selectedRow);
			SelectConversation(selectedRow);
		} else if (mem.chatRooms.Count() + rowShift > 0) {
			if (PDArpDebugMode) Print(PDArpModPreffix + "SelectConversation: X2 " + 0);
			m_lastSelectedContact = 0;
			m_chatRoomList.SelectRow(0);
			SelectConversation(0);
		} else {
			m_lastSelectedContact = -1;
			m_chatRoomList.SelectRow(-1);
			SelectConversation(-1);
		}
	}
	
	void SelectConversation(int id) {
		if (PDArpDebugMode) Print(PDArpModPreffix + "SelectConversation: " + id);

		int rowShift = 0;		
		PluginPDArp pluginPDArp;
		Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
		ref array<ref Param2<string, string>> confMessages = null;
		
		auto mem = pluginPDArp.m_devices.Get(m_pdaId);
		
		if ( (id < 0) || (id >= mem.chatRooms.Count() + rowShift) ) {
			if (PDArpDebugMode) Print(PDArpModPreffix + "No need to select conversation: " + id);
			m_messagesList.ClearItems();
			m_sendMsgInput.Enable(false);
			m_sendMsgBtn.Enable(false);
			m_sendFuncEnabled = false;
			return;
		}
		
		m_messagesList.ClearItems();
		
		float chatWidth;
		float chatHeight;
		m_messagesList.GetScreenSize(chatWidth, chatHeight);
		int chatLineMaxSize = (int)(chatWidth * 0.85) - 50;
		int chatLinesCount = -1;
		string autor;	
		int color;	
		int itemId;
		int addedLinesCount = 0;
		string chatMessage;
		bool isLineFinished;
		string chatLine;
		int i = 0;
		int q = 0;
		int textWidthCalibration;
		int textHeightCalibration; 


		string roomId = mem.chatRooms.Get(id - rowShift).id;
		ref ChatRoom room = pluginPDArp.m_rooms.Get(roomId);
	
		foreach (auto message: room.messages) {
			if (message.sender_id == m_pdaId) {
				autor = "Me";
				color = ARGBF(1, 0.2, 0.8, 0.2);
			}
			else {
				PDArpContact contact;
				
				foreach(auto c: mem.contacts) {
					if (c.id == message.sender_id) {
						contact = c;
						break;
					}
				}
				
				autor = contact.name;
				color = ARGBF(1, 0.976, 1, 0.298);
			}
						
			itemId = m_messagesList.AddItem(autor, NULL, 0);
			m_messagesList.SetItemColor(itemId, 0, color);
			
			m_messagesList.SetItem(itemId, chatLine, NULL, 1);
						
			addedLinesCount = 0;
			chatMessage = message.message;			

			chatLinesCount = chatLinesCount + 1;

	
			m_messagesList.SelectRow(chatLinesCount);
			m_messagesList.EnsureVisible(chatLinesCount);
			
			m_sendMsgInput.Enable(true);
			m_sendMsgBtn.Enable(true);
			m_sendFuncEnabled = true;
			
			// TODO: Mark unread messages are read.
		}
	}
	
	override void Update(float timeslice)
	{
		super.Update(timeslice);

		if (m_externalSendEvent) {
			SendMessageEvent();
			m_externalSendEvent = false;
		}
		
		if (m_lastSelectedContact != m_chatRoomList.GetSelectedRow()) {
			m_lastSelectedContact = m_chatRoomList.GetSelectedRow();
			SelectConversation(m_lastSelectedContact);
		}
		
		if (m_dirty) {
			if (PDArpDebugMode) Print(PDArpModPreffix + "SelectConversation by dirty: X6");
			
			PluginPDArp pluginPDArp;
			Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));

			auto mem = pluginPDArp.m_devices.Get(m_pdaId);
			m_yourIdTxt.SetText("#pda_user_id " + m_pdaId);		
			
			if (mem != null) {
				FillContactsList();			
			}
			m_dirty = false;
		}
		
		if (!m_active) {
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

		Man man = GetGame().GetPlayer();
		ref PlayerBase player = PlayerBase.Cast(man);
		auto pdas = pluginPDArp.GetWorkingPDAsOnPlayer(player);
		auto pda = pdas.Get(0);
		auto mem = pluginPDArp.m_devices.Get(pda.GetMemoryID());
		
		if (m_sendFuncEnabled)
		{
			int selectedRow = m_lastSelectedContact;
			string message = m_sendMsgInput.GetText();
			if (selectedRow >= 0 && message.LengthUtf8() > 0)
			{
				if (message.LengthUtf8() > m_messageMaxLength)
				{
					message = message.Substring(0, m_messageMaxLength);
				}
				
				int rowShift = 0;
				// TODO: More logic on global chat
				// if (pluginPDArp.m_enableGlobalChat)
				// {
				// 	if (selectedRow == rowShift)
				// 	{
				// 		m_sendMessageTimeout = 1.0;
				// 		m_sendMessageStatus = 1;
				// 		GetRPCManager().SendRPC( PDArpModPreffix, "SendGlobalMessage", new Param1<string>( message ), true );
				// 		m_message.SetText("");	
				// 		return true;				
				// 	}
					
				// 	rowShift = rowShift + 1;
				// }
				
				// TODO: More logic on online and banned contacts
				string roomId = mem.chatRooms.Get(selectedRow - rowShift).id;
				// ref PluginPDArp_Conversation msgContact = pluginPDArp.m_contacts[selectedRow - rowShift];
				// string target = msgContact.m_UID;					
				// if ( (pluginPDArp.m_onlineContacts.Find(target) != -1) && (!msgContact.m_IsBanned) )
				// {					
				GetRPCManager().SendRPC( PDArpModPreffix, "SendMessage", new Param3<string, string, string>( pda.GetMemoryID(), roomId, message ), true );
				m_sendMsgInput.SetText("");
				return true;
				// }
			}
		}
		
		return false;
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

	void OnAddContactSubmit() {
		PluginPDArp pluginPDArp;
		Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));

		string contactId = m_addContactIdInput.GetText();

		if (contactId) {
			contactId = contactId.Trim();
		}

		if (!contactId || contactId == "" || contactId.Length() != 8) {
			ShowError("You must provide a valid contact ID");
			return;
		}

		auto pdaMem = pluginPDArp.m_devices.Get(m_pdaId);
		
		auto contact = pdaMem.GetContact(contactId);

		if(contact) {
			ShowError("You already have a contact with this id - " + contact.name);
			return;
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
				OnAddContactSubmit();
				return true;
			}
			
			if (w == m_addContactCancelBtn) {
				ResetView();
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
	
	void OpenAddContact() {
		m_addContactPanel.Show(true, true);
		m_chatRoomPanel.Show(false, true);
	}
	
	void ResetView() {
		m_addContactPanel.Show(false, true);
		m_chatRoomPanel.Show(true, true);
	}
	
	// void UpdateMutedButton()
	// {
	// 	PluginPDArp pluginPDArp;
	// 	Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
		
	// 	if (pluginPDArp.m_options.m_Muted)
	// 	{
	// 		m_mute_btn.SetText("#pda_unmute");
	// 	}
	// 	else
	// 	{
	// 		m_mute_btn.SetText("#pda_mute");
	// 	}
	// }
	
	// void UpdateHideIdButton()
	// {
	// 	PluginPDArp pluginPDArp;
	// 	Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
		
	// 	if (pluginPDArp.m_options.m_HideId)
	// 	{
	// 		m_hideId_btn.SetText("#pda_unhideId");
	// 		m_yourIdTxt.Show(false);
	// 	}
	// 	else
	// 	{
	// 		m_hideId_btn.SetText("#pda_hideId");
	// 		m_yourIdTxt.Show(true);
	// 	}
	// }	

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