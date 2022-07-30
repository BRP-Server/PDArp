class PDARPMenu extends UIScriptedMenu
{
	bool m_active = false;
	bool m_dirty = false;
	
    ref TextListboxWidget m_contactList;
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
	
	float m_addContactTimeout = 0;
	bool m_addContactStatus = 0;
	
	float m_sendMessageTimeout = 0;
	bool m_sendMessageStatus = 0;
	int m_lastSelectedContact = -1;
	
	bool m_externalSendEvent = false;
	bool m_sendFuncEnabled = true;
	
	void PDARPMenu()
	{				
		if (PDARPDebugMode) Print(PDARPModPreffix + "PDARPMenu construct");
	}	
	
	void ~PDARPMenu()
	{
		if (PDARPDebugMode) Print(PDARPModPreffix + "PDARPMenu destruct");
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		player.GetInputController().SetDisabled(false);
	}

    override Widget Init()
    {
		if (PDARPDebugMode) Print(PDARPModPreffix + "PDARPMenu init");
		layoutRoot = GetGame().GetWorkspace().CreateWidgets( "PDARP/scripts/layouts/PDARPMenu.layout" );

        m_contactList = TextListboxWidget.Cast( layoutRoot.FindAnyWidget( "contact_list" ) );
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
		UpdateMutedButton();
		UpdateHideIdButton();
				
		PluginPDARP pluginPDARP;
		Class.CastTo(pluginPDARP, GetPlugin(PluginPDARP));
				
		ref array<string> request = new array<string>();
		for (int i = 0; i < pluginPDARP.m_contacts.Count(); i++)
		{
			request.Insert(pluginPDARP.m_contacts[i].m_UID);
		}
		
		if (!m_active) 
		{
			GetRPCManager().SendRPC( PDARPModPreffix, "CheckContacts", new Param1< ref array<string> >( request ), true );
			m_yourIdText.SetText("#pda_loading");
			m_message.Enable(false);
			m_send.Enable(false);
			m_sendFuncEnabled = false;
		}
		
		m_active = true;		
        return layoutRoot;
    }
	
	void FillContactsList()
	{
		if (PDARPDebugMode) Print(PDARPModPreffix + "FillContactsList");
		
		PluginPDARP pluginPDARP;
		Class.CastTo(pluginPDARP, GetPlugin(PluginPDARP));
		
        int itemId;
        int rowShift = 0;
		int selectedRow = m_lastSelectedContact;
		m_contactList.ClearItems();
			
        if (pluginPDARP.m_enableGlobalChat)
		{
			rowShift = rowShift + 1;
			itemId = m_contactList.AddItem("#pda_global_chat", NULL, 0);
			m_contactList.SetItem(itemId, "" + pluginPDARP.m_globalChatUnreaded, NULL, 1);
			m_contactList.SetItemColor(itemId, 0, ARGBF(1, 0.0, 0.529, 0.858));
		}
            
		for (int i = 0; i < pluginPDARP.m_contacts.Count(); i++)
		{
			ref PluginPDARP_Conversation contact = pluginPDARP.m_contacts[i];
			itemId = m_contactList.AddItem(contact.m_Name, NULL, 0);
			m_contactList.SetItem(itemId, "" + contact.m_Unreaded, NULL, 1);
			
			if (contact.m_IsBanned)
			{
				m_contactList.SetItemColor(itemId, 0, ARGBF(1, 0.8, 0.02, 0.02));
			}
			else if (pluginPDARP.m_onlineContacts.Find(contact.m_UID) != -1)
			{
				m_contactList.SetItemColor(itemId, 0, ARGBF(1, 0.2, 0.8, 0.02));
			}
			else
			{
				m_contactList.SetItemColor(itemId, 0, ARGBF(1, 0.7, 0.7, 0.7));
			}			
		}
		
		if (selectedRow >= 0 && selectedRow < pluginPDARP.m_contacts.Count() + rowShift)
		{
			if (PDARPDebugMode) Print(PDARPModPreffix + "SelectConversation: X1 " + selectedRow);
			m_lastSelectedContact = selectedRow;
			m_contactList.SelectRow(selectedRow);
			SelectConversation(selectedRow);
		}
		else if (pluginPDARP.m_contacts.Count() + rowShift > 0)
		{
			if (PDARPDebugMode) Print(PDARPModPreffix + "SelectConversation: X2 " + 0);
			m_lastSelectedContact = 0;
			m_contactList.SelectRow(0);
			SelectConversation(0);
		}
		else
		{
			m_lastSelectedContact = -1;
			m_contactList.SelectRow(-1);
			SelectConversation(-1);
		}
	}
	
	void SelectConversation(int id)
	{
		if (PDARPDebugMode) Print(PDARPModPreffix + "SelectConversation: " + id);
		
		int rowShift = 0;		
		PluginPDARP pluginPDARP;
		Class.CastTo(pluginPDARP, GetPlugin(PluginPDARP));
		ref array<ref Param2<string, string>> confMessages = null;
		
		if (pluginPDARP.m_enableGlobalChat)
		{
			if (id == rowShift)
			{
				confMessages = pluginPDARP.m_globalMessages;
				pluginPDARP.m_globalChatUnreaded = 0;
				m_contactList.SetItem(id, "0", NULL, 1);
			}
			
			rowShift = rowShift + 1;
		}
		
		if ( (id < 0) || (id >= pluginPDARP.m_contacts.Count() + rowShift) )
		{
			if (PDARPDebugMode) Print(PDARPModPreffix + "No need to select conversation: " + id);
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
		else
		{
			ref PluginPDARP_Conversation contact = pluginPDARP.m_contacts[id - rowShift];		
			ref array<ref PluginPDARP_Comment> comments = pluginPDARP.GetComments(contact.m_UID);

			for (i = 0; i < comments.Count(); i++)
			{
				ref PluginPDARP_Comment comment = comments[i];	
				if (comment.m_UID == GetGame().GetPlayer().GetIdentity().GetId())
				{
					autor = "Me";
					color = ARGBF(1, 0.2, 0.8, 0.2);
				}
				else
				{
					autor = contact.m_Name;
					color = ARGBF(1, 0.976, 1, 0.298);
				}
							
				itemId = m_chat.AddItem(autor, NULL, 0);
				m_chat.SetItemColor(itemId, 0, color);
							
				addedLinesCount = 0;
				chatMessage = comment.m_Message;			
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
			
			if ( (pluginPDARP.m_onlineContacts.Find(contact.m_UID) == -1) || (contact.m_IsBanned) )
			{
				m_message.Enable(false);
				m_send.Enable(false);
				m_sendFuncEnabled = false;
			}
			else
			{
				m_message.Enable(true);
				m_send.Enable(true);
				m_sendFuncEnabled = true;
			}
			
			if (contact.m_Unreaded > 0)
			{
				contact.m_Unreaded = 0;
				m_contactList.SetItem(id, "" + contact.m_Unreaded, NULL, 1);
				pluginPDARP.SaveContactsConfig();
			}
		}
	}
	
	override void Update(float timeslice)
	{
		super.Update(timeslice);

		if (m_externalSendEvent)
		{
			SendMessageEvent();
			m_externalSendEvent = false;
		}
		
		if (m_addContactStatus == 2)
		{
			if (PDARPDebugMode) Print(PDARPModPreffix + "SelectConversation: X5");
			FillContactsList();
			m_addContactStatus = 0;
		}
		
		if (m_sendMessageStatus == 2)
		{
			if (PDARPDebugMode) Print(PDARPModPreffix + "SelectConversation: X3 " + m_lastSelectedContact);
			FillContactsList();
			m_sendMessageStatus = 0;
		}
		
		if (m_lastSelectedContact != m_contactList.GetSelectedRow())
		{
			m_lastSelectedContact = m_contactList.GetSelectedRow();
			SelectConversation(m_lastSelectedContact);
		}
		
		if (m_addContactTimeout > 0 || m_addContactStatus != 0)
		{
			m_addContactTimeout = m_addContactTimeout - timeslice;
			m_addContactBtn.Enable(false);
		}
		else
		{
			m_addContactBtn.Enable(true);
		}
		
		if (m_sendMessageTimeout > 0 || m_sendMessageStatus != 0)
		{
			m_sendMessageTimeout = m_sendMessageTimeout - timeslice;
			m_send.Enable(false);
		}
		else
		{
			m_send.Enable(true);
		}
		
		if (m_dirty)
		{
			if (PDARPDebugMode) Print(PDARPModPreffix + "SelectConversation by dirty: X6");
			
			PluginPDARP pluginPDARP;
			Class.CastTo(pluginPDARP, GetPlugin(PluginPDARP));
			m_yourIdText.SetText("#pda_user_id " + pluginPDARP.m_steamId);		
			
			FillContactsList();
			m_dirty = false;
		}
		
		if (!m_active)
		{
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
		if (PDARPDebugMode) Print(PDARPModPreffix + "OnKeyPress: " + w.GetName());
		
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
		PluginPDARP pluginPDARP;
		Class.CastTo(pluginPDARP, GetPlugin(PluginPDARP));
		
		if (m_sendFuncEnabled && m_sendMessageTimeout <= 0 && m_sendMessageStatus == 0)
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
				if (pluginPDARP.m_enableGlobalChat)
				{
					if (selectedRow == rowShift)
					{
						m_sendMessageTimeout = 1.0;
						m_sendMessageStatus = 1;
						GetRPCManager().SendRPC( PDARPModPreffix, "SendGlobalMessage", new Param1<string>( message ), true );
						m_message.SetText("");	
						return true;				
					}
					
					rowShift = rowShift + 1;
				}
				
				ref PluginPDARP_Conversation msgContact = pluginPDARP.m_contacts[selectedRow - rowShift];
				string target = msgContact.m_UID;					
				if ( (pluginPDARP.m_onlineContacts.Find(target) != -1) && (!msgContact.m_IsBanned) )
				{					
					m_sendMessageTimeout = 0.25;
					m_sendMessageStatus = 1;
					GetRPCManager().SendRPC( PDARPModPreffix, "SendMessage", new Param2<string, string>( target, message ), true );
					m_message.SetText("");
					return true;
				}
			}
		}
		
		return false;
	}
	
	override bool OnClick(Widget w, int x, int y, int button) {
		super.OnClick(w, x, y, button);
		
		if (PDARPDebugMode) Print(PDARPModPreffix + "OnClick: " + w.GetName());
		
		if (button == MouseState.LEFT)
		{
			int rowShift;
			int selectedRow;
			PluginPDARP pluginPDARP;
			Class.CastTo(pluginPDARP, GetPlugin(PluginPDARP));
			
			if (w == m_addContactBtn)
			{
				if (m_addContactTimeout <= 0 && m_addContactStatus == 0)
				{
					string contactId = m_addContactTxt.GetText();
					if (contactId.LengthUtf8() <= m_contactMaxLength && contactId.LengthUtf8() > 0) {
						m_addContactStatus = 1;
						m_addContactTimeout = 1;
						GetRPCManager().SendRPC( PDARPModPreffix, "AddContact", new Param1<string>( contactId ), true );
						return true;
					}	
				}
			}
			
			if (w == m_send)
			{
				return SendMessageEvent();
			}
			
			if (w == m_deleteContactBtn)
			{
				selectedRow = m_lastSelectedContact;
				rowShift = 0;
				if (pluginPDARP.m_enableGlobalChat)
				{				
					if (selectedRow == rowShift)
					{
						return true;
					}
					
					rowShift = rowShift + 1;
				}
				
				if (selectedRow >= 0 && selectedRow < pluginPDARP.m_contacts.Count() + rowShift)
				{
					string contactToDeleteUid = pluginPDARP.m_contacts[selectedRow - rowShift].m_UID;
					pluginPDARP.RemoveContact(contactToDeleteUid);
					m_dirty = true;
					return true;
				}
			}
			
			if (w == m_mute_btn)
			{
				pluginPDARP.m_options.m_Muted = !pluginPDARP.m_options.m_Muted;
				UpdateMutedButton();
				return true;
			}
			
			if (w == m_hideId_btn)
			{
				pluginPDARP.m_options.m_HideId = !pluginPDARP.m_options.m_HideId;
				UpdateHideIdButton();
				return true;
			}
			
			if (w == m_rename_btn)
			{
				selectedRow = m_lastSelectedContact;
				rowShift = 0;
				if (pluginPDARP.m_enableGlobalChat)
				{				
					if (selectedRow == rowShift)
					{
						return true;
					}
					
					rowShift = rowShift + 1;
				}
				
				if (selectedRow >= 0 && selectedRow < pluginPDARP.m_contacts.Count() + rowShift)
				{
					string newContactName = m_addContactTxt.GetText();
					if (newContactName.LengthUtf8() <= m_contactMaxLength && newContactName.LengthUtf8() > 0)
					{
						string contactToRenameUid = pluginPDARP.m_contacts[selectedRow - rowShift].m_UID;
						pluginPDARP.RenameContact(contactToRenameUid, newContactName);
						m_dirty = true;
						return true;
					}
				}
			}
			
			if (w == m_ban_btn)
			{
				selectedRow = m_lastSelectedContact;
				rowShift = 0;
				if (pluginPDARP.m_enableGlobalChat)
				{				
					if (selectedRow == rowShift)
					{
						return true;
					}
					
					rowShift = rowShift + 1;
				}
				
				if (selectedRow >= 0 && selectedRow < pluginPDARP.m_contacts.Count() + rowShift)
				{
					string contactToBanUid = pluginPDARP.m_contacts[selectedRow - rowShift].m_UID;
					pluginPDARP.BanUnbanContact(contactToBanUid);
					m_dirty = true;
					return true;
				}
			}
		}
		
		return false;
	}
	
	void UpdateMutedButton()
	{
		PluginPDARP pluginPDARP;
		Class.CastTo(pluginPDARP, GetPlugin(PluginPDARP));
		
		if (pluginPDARP.m_options.m_Muted)
		{
			m_mute_btn.SetText("#pda_unmute");
		}
		else
		{
			m_mute_btn.SetText("#pda_mute");
		}
	}
	
	void UpdateHideIdButton()
	{
		PluginPDARP pluginPDARP;
		Class.CastTo(pluginPDARP, GetPlugin(PluginPDARP));
		
		if (pluginPDARP.m_options.m_HideId)
		{
			m_hideId_btn.SetText("#pda_unhideId");
			m_yourIdText.Show(false);
		}
		else
		{
			m_hideId_btn.SetText("#pda_hideId");
			m_yourIdText.Show(true);
		}
	}	

	override bool OnChange(Widget w, int x, int y, bool finished) {
		super.OnChange(w, x, y, finished);
		
		if (PDARPDebugMode) Print(PDARPModPreffix + "OnChange: " + w.GetName());
		
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
		
		if (PDARPDebugMode) Print(PDARPModPreffix + "OnItemSelected: " + w.GetName());
		
		if (w == m_contactList)
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