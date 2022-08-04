class PDArpMenu extends UIScriptedMenu
{
	bool m_active = false;
	bool m_dirty = false;
	
    ref TextListboxWidget m_chatRooms;
	ref TextWidget m_yourIdText;
	ref EditBoxWidget m_addContactTxt;
	ref ButtonWidget m_addContactBtn;
	ref ButtonWidget m_deleteContactBtn;
	ref TextListboxWidget m_chat;
	ref EditBoxWidget m_message;
	ref ButtonWidget m_send;
	ref TextWidget m_callibrationText;
	ref ButtonWidget m_mute_btn;
	ref ButtonWidget m_hideId_btn;
	ref ButtonWidget m_rename_btn;
	ref ButtonWidget m_ban_btn;
	
	const int m_contactMaxLength = 32;
	const int m_messageMaxLength = 256;
		
	int m_lastSelectedContact = -1;
	
	bool m_externalSendEvent = false;
	bool m_sendFuncEnabled = true;

	string m_pdaId;
	
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
		layoutRoot = GetGame().GetWorkspace().CreateWidgets( "PDArp/scripts/layouts/PDArpMenu.layout" );

        m_chatRooms = TextListboxWidget.Cast( layoutRoot.FindAnyWidget( "contact_list" ) );
		m_yourIdText = TextWidget.Cast( layoutRoot.FindAnyWidget( "your_id_text" ) );
		m_addContactTxt = EditBoxWidget.Cast( layoutRoot.FindAnyWidget( "add_contact_txt" ) );
		m_addContactBtn = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "add_contact_btn" ) );
		m_deleteContactBtn = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "delete_contact_btn" ) );
		m_chat = TextListboxWidget.Cast( layoutRoot.FindAnyWidget( "messages_txt" ) );
		m_message = EditBoxWidget.Cast( layoutRoot.FindAnyWidget( "send_msg_txt" ) );
		m_send = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "semd_msg_btn" ) );
		m_callibrationText = TextWidget.Cast( layoutRoot.FindAnyWidget( "callibration_txt" ) );
		m_mute_btn = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "mute_btn" ) );
		m_hideId_btn = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "hideid_btn" ) );
		m_rename_btn = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "rename_contact_btn" ) );
		m_ban_btn = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "ban_contact_btn" ) );
		// UpdateMutedButton();
		// UpdateHideIdButton();
				
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
			m_yourIdText.SetText("#pda_loading");
			m_message.Enable(false);
			m_send.Enable(false);
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
		m_chatRooms.ClearItems();

		auto mem = pluginPDArp.m_devices.Get(m_pdaId);

		ChatRoom chatRoom;
		if (mem != null) {
			foreach(auto chatRoomPref: mem.chatRooms) {
				chatRoom = pluginPDArp.m_rooms.Get(chatRoomPref.id);
				itemId = m_chatRooms.AddItem(chatRoom.name, NULL, 0);

				// TODO: Not sure how I will deal with the unread stuff
				// m_chatRooms.SetItem(itemId, "" + chatRoom.unreaded, NULL, 1);
				
				// TODO: Not sure how I will deal with the original ban & online users feature.
				// if (contact.m_IsBanned) {
				// 	m_chatRooms.SetItemColor(itemId, 0, ARGBF(1, 0.8, 0.02, 0.02));
				// }
				// else if (pluginPDArp.m_onlineContacts.Find(contact.m_UID) != -1)
				// {
				// 	m_chatRooms.SetItemColor(itemId, 0, ARGBF(1, 0.2, 0.8, 0.02));
				// }
				
				m_chatRooms.SetItemColor(itemId, 0, ARGBF(1, 0.7, 0.7, 0.7));

			}
		}
		
		if (selectedRow >= 0 && selectedRow < mem.chatRooms.Count() + rowShift) {
			if (PDArpDebugMode) Print(PDArpModPreffix + "SelectConversation: X1 " + selectedRow);
			m_lastSelectedContact = selectedRow;
			m_chatRooms.SelectRow(selectedRow);
			SelectConversation(selectedRow);
		} else if (mem.chatRooms.Count() + rowShift > 0) {
			if (PDArpDebugMode) Print(PDArpModPreffix + "SelectConversation: X2 " + 0);
			m_lastSelectedContact = 0;
			m_chatRooms.SelectRow(0);
			SelectConversation(0);
		} else {
			m_lastSelectedContact = -1;
			m_chatRooms.SelectRow(-1);
			SelectConversation(-1);
		}
	}
	
	void SelectConversation(int id) {
		if (PDArpDebugMode) Print(PDArpModPreffix + "SelectConversation: " + id);

		int rowShift = 0;		
		PluginPDArp pluginPDArp;
		Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
		ref array<ref Param2<string, string>> confMessages = null;
		
		// TODO: The PDA should be passed in or be a module attribute of the menu. (Probably the last one)
		auto mem = pluginPDArp.m_devices.Get(m_pdaId);
		
		if ( (id < 0) || (id >= mem.chatRooms.Count() + rowShift) ) {
			if (PDArpDebugMode) Print(PDArpModPreffix + "No need to select conversation: " + id);
			m_chat.ClearItems();
			m_message.Enable(false);
			m_send.Enable(false);
			m_sendFuncEnabled = false;
			return;
		}
		
		m_chat.ClearItems();
		
		float chatWidth;
		float chatHeight;
		m_chat.GetScreenSize(chatWidth, chatHeight);
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

		// TODO: What are this confMessages?
		if (confMessages)
		{
			for (i = 0; i < confMessages.Count(); i++)
			{
				ref Param2<string, string> commentGlob = confMessages[i];
				autor = commentGlob.param1;							
				itemId = m_chat.AddItem(autor, NULL, 0);							
				addedLinesCount = 0;
				chatMessage = commentGlob.param2;			
				while (chatMessage.LengthUtf8() > 0)
				{		
					isLineFinished = false;
					chatLine = "";		
					for (q = 0; q < chatMessage.LengthUtf8() && !isLineFinished; q++)
					{
						chatLine = chatLine + chatMessage.SubstringUtf8(q, 1);						
						m_callibrationText.SetText(chatLine);
						m_callibrationText.GetTextSize(textWidthCalibration, textHeightCalibration);
						
						if (textWidthCalibration >= chatLineMaxSize)
						{						
							isLineFinished = true;
						}
					}
					
					if (addedLinesCount == 0)
					{
						m_chat.SetItem(itemId, chatLine, NULL, 1);
					} 
					else
					{
						m_chat.AddItem(chatLine, NULL, 1);
					}
					
					chatMessage = chatMessage.SubstringUtf8(chatLine.LengthUtf8(), chatMessage.LengthUtf8() - chatLine.LengthUtf8());
					addedLinesCount = addedLinesCount + 1;
					chatLinesCount = chatLinesCount + 1;
				}
			}
			
			m_callibrationText.SetText("");
			m_chat.SelectRow(chatLinesCount);
			m_chat.EnsureVisible(chatLinesCount);
			
			m_message.Enable(true);
			m_send.Enable(true);
			m_sendFuncEnabled = true;
		}
		else{
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
							
				itemId = m_chat.AddItem(autor, NULL, 0);
				m_chat.SetItemColor(itemId, 0, color);
							
				addedLinesCount = 0;
				chatMessage = message.message;			
				while (chatMessage.LengthUtf8() > 0)
				{		
					isLineFinished = false;
					chatLine = "";		
					for (q = 0; q < chatMessage.LengthUtf8() && !isLineFinished; q++)
					{
						chatLine = chatLine + chatMessage.SubstringUtf8(q, 1);
						m_callibrationText.SetText(chatLine);
						m_callibrationText.GetTextSize(textWidthCalibration, textHeightCalibration);
						
						if (textWidthCalibration >= chatLineMaxSize)
						{						
							isLineFinished = true;
						}
					}
					
					if (addedLinesCount == 0)
					{
						m_chat.SetItem(itemId, chatLine, NULL, 1);
					} 
					else
					{
						m_chat.AddItem(chatLine, NULL, 1);
					}
					
					chatMessage = chatMessage.SubstringUtf8(chatLine.LengthUtf8(), chatMessage.LengthUtf8() - chatLine.LengthUtf8());
					addedLinesCount = addedLinesCount + 1;
					chatLinesCount = chatLinesCount + 1;
				}
			}
			
			m_callibrationText.SetText("");
			m_chat.SelectRow(chatLinesCount);
			m_chat.EnsureVisible(chatLinesCount);
			
			// TODO: more bits of the online functionality
			// if ( (pluginPDArp.m_onlineContacts.Find(contact.m_UID) == -1) || (contact.m_IsBanned) )
			// {
			// 	m_message.Enable(false);
			// 	m_send.Enable(false);
			// 	m_sendFuncEnabled = false;
			// }
			// else
			// {
			m_message.Enable(true);
			m_send.Enable(true);
			m_sendFuncEnabled = true;
			// }
			
			// TODO: More bits of the unread functionality
			// if (contact.m_Unreaded > 0)
			// {
			// 	contact.m_Unreaded = 0;
			// 	m_chatRooms.SetItem(id, "" + contact.m_Unreaded, NULL, 1);
			// 	pluginPDArp.SaveContactsConfig();
			// }
		}
	}
	
	override void Update(float timeslice)
	{
		super.Update(timeslice);

		if (m_externalSendEvent) {
			SendMessageEvent();
			m_externalSendEvent = false;
		}
		
		if (m_lastSelectedContact != m_chatRooms.GetSelectedRow()) {
			m_lastSelectedContact = m_chatRooms.GetSelectedRow();
			SelectConversation(m_lastSelectedContact);
		}
		
		if (m_dirty) {
			if (PDArpDebugMode) Print(PDArpModPreffix + "SelectConversation by dirty: X6");
			
			PluginPDArp pluginPDArp;
			Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));

			auto mem = pluginPDArp.m_devices.Get(m_pdaId);
			m_yourIdText.SetText("#pda_user_id " + m_pdaId);		
			
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
		if (w.GetName() == m_addContactTxt.GetName()) {
			boxText = m_addContactTxt.GetText();
			if (boxText.LengthUtf8() >= m_contactMaxLength) {
				return true;
			}
		}
		if (w.GetName() == m_message.GetName()) {
			boxText = m_message.GetText();
			if (boxText.LengthUtf8() >= m_messageMaxLength) {
				return true;
			}
		}
		return false;
	}

	bool SendMessageEvent()
	{
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
			string message = m_message.GetText();
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
				m_message.SetText("");
				return true;
				// }
			}
		}
		
		return false;
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
			
			if (w == m_addContactBtn) {
				string contactId = m_addContactTxt.GetText();
				if (contactId.LengthUtf8() <= m_contactMaxLength && contactId.LengthUtf8() > 0) {
					auto contact = new PDArpContact(contactId, "Unknown");
					GetRPCManager().SendRPC( PDArpModPreffix, "AddContact", new Param2<string, PDArpContact>( m_pdaId, contact), true );
					return true;
				}	
			}
			
			if (w == m_send) {
				return SendMessageEvent();
			}
			
			// TODO Implement delete contact functionality
			// if (w == m_deleteContactBtn)
			// {
			// 	selectedRow = m_lastSelectedContact;
			// 	rowShift = 0;
			// 	// TODO: More logic on global chat, probably non relevant for our implementation
			// 	if (pluginPDArp.m_enableGlobalChat)
			// 	{				
			// 		if (selectedRow == rowShift)
			// 		{
			// 			return true;
			// 		}
					
			// 		rowShift = rowShift + 1;
			// 	}
				
			// 	TODO: Implement delete contact button
			// 	if (selectedRow >= 0 && selectedRow < pluginPDArp.m_contacts.Count() + rowShift)
			// 	{
			// 		string contactToDeleteUid = pluginPDArp.m_contacts[selectedRow - rowShift].m_UID;
			// 		pluginPDArp.RemoveContact(contactToDeleteUid);
			// 		m_dirty = true;
			// 		return true;
			// 	}

			// 	return true;
			// }
			
			// TODO Implement mute functionality
			// if (w == m_mute_btn) {
			// 	pluginPDArp.m_options.m_Muted = !pluginPDArp.m_options.m_Muted;
			// 	UpdateMutedButton();
			// 	return true;
			// }
			
			// TODO Implement hide id functionality (is this even needed?)
			// if (w == m_hideId_btn) {
			// 	pluginPDArp.m_options.m_HideId = !pluginPDArp.m_options.m_HideId;
			// 	UpdateHideIdButton();
			// 	return true;
			// }
			
			// TODO: Implement rename functionality.
			// if (w == m_rename_btn) {
			// 	selectedRow = m_lastSelectedContact;
			// 	rowShift = 0;
			// 	if (pluginPDArp.m_enableGlobalChat)
			// 	{				
			// 		if (selectedRow == rowShift)
			// 		{
			// 			return true;
			// 		}
					
			// 		rowShift = rowShift + 1;
			// 	}
				
			// 	if (selectedRow >= 0 && selectedRow < pluginPDArp.m_contacts.Count() + rowShift)
			// 	{
			// 		string newContactName = m_addContactTxt.GetText();
			// 		if (newContactName.LengthUtf8() <= m_contactMaxLength && newContactName.LengthUtf8() > 0)
			// 		{
			// 			string contactToRenameUid = pluginPDArp.m_contacts[selectedRow - rowShift].m_UID;
			// 			pluginPDArp.RenameContact(contactToRenameUid, newContactName);
			// 			m_dirty = true;
			// 			return true;
			// 		}
			// 	}
			// }
			
			// TODO: Implement ban functionality
			// if (w == m_ban_btn)
			// {
			// 	selectedRow = m_lastSelectedContact;
			// 	rowShift = 0;
			// 	if (pluginPDArp.m_enableGlobalChat)
			// 	{				
			// 		if (selectedRow == rowShift)
			// 		{
			// 			return true;
			// 		}
					
			// 		rowShift = rowShift + 1;
			// 	}
				
			// 	if (selectedRow >= 0 && selectedRow < pluginPDArp.m_contacts.Count() + rowShift)
			// 	{
			// 		string contactToBanUid = pluginPDArp.m_contacts[selectedRow - rowShift].m_UID;
			// 		pluginPDArp.BanUnbanContact(contactToBanUid);
			// 		m_dirty = true;
			// 		return true;
			// 	}
			// }
		}
		
		return false;
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
	// 		m_yourIdText.Show(false);
	// 	}
	// 	else
	// 	{
	// 		m_hideId_btn.SetText("#pda_hideId");
	// 		m_yourIdText.Show(true);
	// 	}
	// }	

	override bool OnChange(Widget w, int x, int y, bool finished) {
		super.OnChange(w, x, y, finished);
		
		if (PDArpDebugMode) Print(PDArpModPreffix + "OnChange: " + w.GetName());
		
		string boxText;
		if (w.GetName() == m_addContactTxt.GetName()) {
			boxText = m_addContactTxt.GetText();
			if (boxText.LengthUtf8() > m_contactMaxLength) {
				m_addContactTxt.SetText(boxText.Substring(0, m_contactMaxLength));
			}
			return true;
		}
		if (w.GetName() == m_message.GetName()) {
			boxText = m_message.GetText();
			if (boxText.LengthUtf8() > m_messageMaxLength) {
				m_message.SetText(boxText.Substring(0, m_messageMaxLength));
			}
			return true;
		}
		return false;
	}
	
	override bool OnItemSelected(Widget w, int x, int y, int row, int  column,	int  oldRow, int  oldColumn)
	{
		super.OnItemSelected(w, x, y, row, column, oldRow, oldColumn);
		
		if (PDArpDebugMode) Print(PDArpModPreffix + "OnItemSelected: " + w.GetName());
		
		if (w == m_chatRooms)
		{
			return true;
		}
		
		if (w == m_chat)
		{
			return true;
		}
		
		return false;
	}
}