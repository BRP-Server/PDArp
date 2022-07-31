class PluginPDArp extends PluginBase
{
	string m_configDir;
	
	ref PDArpMenu m_PDArpMenu;
	
	ref array<ref PluginPDArp_Conversation> m_contacts;
	
	ref array<string> m_onlineContacts;
	
	ref map<string, ref array<ref PluginPDArp_Comment>> m_comments;
	
	string m_steamId;
	
	const string contactsJsonFilename = "contacts.json";
	
	const string optionsJsonFilename = "options.json";
	
	ref PluginPDArp_Options m_options;
    
    bool m_enableGlobalChat = false;
    
    bool m_enableGlobalChatSound = false;
    
    ref array<ref Param2<string, string>> m_globalMessages;
    
    int m_globalChatUnreaded = 0;
	
	void PluginPDArp()
	{
		if (PDArpDebugMode) Print(PDArpModPreffix + "PluginPDArp construct.");
		m_contacts = new array<ref PluginPDArp_Conversation>();
		m_comments = new map<string, ref array<ref PluginPDArp_Comment>>();
        m_globalMessages = new array<ref Param2<string, string>>;
		m_onlineContacts = new array<string>();
		m_options = new PluginPDArp_Options();
		m_configDir = "$profile:\\PDArp\\";
	}
	
	void ~PluginPDArp()
	{
		SaveOptionsConfig();
		if (PDArpDebugMode) Print(PDArpModPreffix + "PluginPDArp destruct.");
	}
	
	override void OnInit()
	{
		MakeDirectory(m_configDir);
		
		if (!FileExist(m_configDir + contactsJsonFilename)) {
			SaveOptionsConfig();
            SaveContactsConfig();
        } else {
			LoadOptionsConfig();
            LoadContactsConfig();
        }
	}
	
	void SaveOptionsConfig()
	{
		if (PDArpDebugMode) Print(PDArpModPreffix + "SaveOptionsConfig.");
		JsonFileLoader<ref PluginPDArp_Options>.JsonSaveFile(m_configDir + optionsJsonFilename, m_options);
	}
	
	void LoadOptionsConfig()
	{
		if (PDArpDebugMode) Print(PDArpModPreffix + "LoadOptionsConfig.");
		JsonFileLoader<ref PluginPDArp_Options>.JsonLoadFile(m_configDir + optionsJsonFilename, m_options);
	}
	
	void SaveContactsConfig()
	{
		if (PDArpDebugMode) Print(PDArpModPreffix + "SaveContactsConfig.");
		JsonFileLoader<ref array<ref PluginPDArp_Conversation>>.JsonSaveFile(m_configDir + contactsJsonFilename, m_contacts);
	}
	
	void LoadContactsConfig()
	{
		if (PDArpDebugMode) Print(PDArpModPreffix + "LoadContactsConfig.");
		JsonFileLoader<ref array<ref PluginPDArp_Conversation>>.JsonLoadFile(m_configDir + contactsJsonFilename, m_contacts);
	}
	
	void SaveCommentsConfig(string uid)
	{
		if (PDArpDebugMode) Print(PDArpModPreffix + "SaveCommentsConfig: " + uid);
		
		if (m_comments.Contains(uid))
		{
			ref array<ref PluginPDArp_Comment> comments = m_comments[uid];
			
			while (comments.Count() > 100)
			{
				comments.RemoveOrdered(0);
			}
			
			JsonFileLoader<ref array<ref PluginPDArp_Comment>>.JsonSaveFile(m_configDir + uid + ".json", comments);
		}
	}
	
	void LoadCommentsConfig(string uid)
	{
		if (PDArpDebugMode) Print(PDArpModPreffix + "LoadCommentsConfig: " + uid);
		
		string fileName = m_configDir + uid + ".json";
		if (FileExist(fileName))
		{
			ref array<ref PluginPDArp_Comment> result = new array<ref PluginPDArp_Comment>(); 
			JsonFileLoader<ref array<ref PluginPDArp_Comment>>.JsonLoadFile(fileName, result);
			m_comments[uid] = result;
		}
	}
	
	ref array<ref PluginPDArp_Comment> GetComments(string uid)
	{
		if (!m_comments.Contains(uid))
		{
			LoadCommentsConfig(uid);
		}

		if (!m_comments.Contains(uid))
		{
			m_comments[uid] = new array<ref PluginPDArp_Comment>();
		}
		
		return m_comments[uid];
	}
	
	void AddComment(string contactId, string senderId, string message)
	{
		if (contactId.Length() > 0)
		{
			ref array<ref PluginPDArp_Comment> comments = GetComments(contactId);
			ref PluginPDArp_Comment comment = new PluginPDArp_Comment();
			comment.m_UID = senderId;
			comment.m_Message = message;
			comments.Insert(comment);
			
			SaveCommentsConfig(contactId);
			
			ref PluginPDArp_Conversation contact = FindContact(contactId);
			if (contact != null)
			{
				contact.m_Unreaded = contact.m_Unreaded + 1;
				SaveContactsConfig();
			}
			
			if (IsOpen())
			{
				m_PDArpMenu.m_sendMessageStatus = 2;
			}
		}
		else
		{
			if (IsOpen())
			{
				m_PDArpMenu.m_sendMessageStatus = 0;
			}
				
			ref array<string> request = new array<string>();
			for (int i = 0; i < m_contacts.Count(); i++)
			{
				request.Insert(m_contacts[i].m_UID);
			}
			GetRPCManager().SendRPC( PDArpModPreffix, "CheckContacts", new Param1< ref array<string> >( request ), true );
		}
	}
	
	bool IsOpen()
	{
		return m_PDArpMenu && m_PDArpMenu.m_active;
	}
	
	void Open()
	{
		if (IsOpen())
		{
			Close();
		}
		
		if (GetGame().GetUIManager().GetMenu() != NULL)
		{
			if (PDArpDebugMode) Print(PDArpModPreffix + "OpenRecipesBookAction ActionCondition blocking by external menu: " + GetGame().GetUIManager().GetMenu());
			return;
		}
		
		if (PDArpDebugMode) Print(PDArpModPreffix + "PluginPDArp prepare open menu");
		m_PDArpMenu = new PDArpMenu;
		m_PDArpMenu.Init();
		GetGame().GetUIManager().ShowScriptedMenu( m_PDArpMenu, NULL );
		if (PDArpDebugMode) Print(PDArpModPreffix + "PluginPDArp post open menu: " + m_PDArpMenu);
	}
	
	ref PluginPDArp_Conversation FindContact(string uid)
	{
		if (PDArpDebugMode) Print(PDArpModPreffix + "FindContact: " + uid);
		
		for (int i = 0; i < m_contacts.Count(); i++)
		{
			ref PluginPDArp_Conversation item = m_contacts[i];
			if (item.m_UID == uid)
			{
				return item;
			}
		}
		
		return null;
	}
	
	void AddContact(string uid, string name)
	{
		if (PDArpDebugMode) Print(PDArpModPreffix + "AddContact X1: " + uid + "; " + name);
		if (uid.Length() > 0 && name.Length() > 0)
		{
			if (PDArpDebugMode) Print(PDArpModPreffix + "AddContact X2: " + uid + "; " + name);
			if (FindContact(uid) == null)
			{
				if (PDArpDebugMode) Print(PDArpModPreffix + "AddContact X3: " + uid + "; " + name);
				ref PluginPDArp_Conversation conv = new PluginPDArp_Conversation();
				conv.m_UID = uid;
				conv.m_Name = name;
				conv.m_Unreaded = 0;
				conv.m_IsBanned = false;
				m_contacts.Insert(conv);
				m_onlineContacts.Insert(uid);
				SaveContactsConfig();
			}
		}
		
		if (IsOpen())
		{
			m_PDArpMenu.m_addContactStatus = 2;
		}
	}
	
	void RemoveContact(string uid)
	{
		if (PDArpDebugMode) Print(PDArpModPreffix + "RemoveContact: " + uid);
		
		ref PluginPDArp_Conversation contactToDelete = FindContact(uid);
		if (contactToDelete == null)
		{
			return;
		}
		
		m_contacts.RemoveItem(contactToDelete);
		SaveContactsConfig();
		
		if (m_comments.Contains(uid))
		{
			m_comments.Remove(uid);
		}
		
		string fileName = m_configDir + uid + ".json";
		if (FileExist(fileName))
		{
			DeleteFile(fileName);
		}
	}
	
	void RenameContact(string uid, string newName)
	{
		if (PDArpDebugMode) Print(PDArpModPreffix + "RenameContact: " + uid);
		
		ref PluginPDArp_Conversation contactToRename = FindContact(uid);
		if (contactToRename == null)
		{
			return;
		}
		
		contactToRename.m_Name = newName;
		SaveContactsConfig();
	}
	
	void BanUnbanContact(string uid)
	{
		if (PDArpDebugMode) Print(PDArpModPreffix + "BanContact: " + uid);
		
		ref PluginPDArp_Conversation contactToRename = FindContact(uid);
		if (contactToRename == null)
		{
			return;
		}
		
		contactToRename.m_IsBanned = !contactToRename.m_IsBanned;
		SaveContactsConfig();
	}
	
	void Close()
	{
		if (m_PDArpMenu)
		{
			m_PDArpMenu.m_active = false;
		}

		if (PDArpDebugMode) Print(PDArpModPreffix + "PluginPDArp close menu: " + m_PDArpMenu);
	}
};

class PluginPDArp_Conversation
{
	string m_UID;
	string m_Name;
	int m_Unreaded;
	bool m_IsBanned;

	void PluginPDArp_Conversation()
	{
	
	}
};

class PluginPDArp_Comment
{
	string m_UID;
	string m_Message;
	
	void PluginPDArp_Comment()
	{
	
	}
};

class PluginPDArp_Options
{
	bool m_Muted;
	bool m_HideId;
	
	void PluginPDArp_Options()
	{
	
	}
};