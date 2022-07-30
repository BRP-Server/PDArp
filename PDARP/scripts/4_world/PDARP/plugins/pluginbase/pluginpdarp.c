class PluginPDARP extends PluginBase
{
	string m_configDir;
	
	ref PDARPMenu m_PDARPMenu;
	
	ref array<ref PluginPDARP_Conversation> m_contacts;
	
	ref array<string> m_onlineContacts;
	
	ref map<string, ref array<ref PluginPDARP_Comment>> m_comments;
	
	string m_steamId;
	
	const string contactsJsonFilename = "contacts.json";
	
	const string optionsJsonFilename = "options.json";
	
	ref PluginPDARP_Options m_options;
    
    bool m_enableGlobalChat = false;
    
    bool m_enableGlobalChatSound = false;
    
    ref array<ref Param2<string, string>> m_globalMessages;
    
    int m_globalChatUnreaded = 0;
	
	void PluginPDARP()
	{
		if (PDARPDebugMode) Print(PDARPModPreffix + "PluginPDARP construct.");
		m_contacts = new array<ref PluginPDARP_Conversation>();
		m_comments = new map<string, ref array<ref PluginPDARP_Comment>>();
        m_globalMessages = new array<ref Param2<string, string>>;
		m_onlineContacts = new array<string>();
		m_options = new PluginPDARP_Options();
		m_configDir = "$profile:\\PDARP\\";
	}
	
	void ~PluginPDARP()
	{
		SaveOptionsConfig();
		if (PDARPDebugMode) Print(PDARPModPreffix + "PluginPDARP destruct.");
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
		if (PDARPDebugMode) Print(PDARPModPreffix + "SaveOptionsConfig.");
		JsonFileLoader<ref PluginPDARP_Options>.JsonSaveFile(m_configDir + optionsJsonFilename, m_options);
	}
	
	void LoadOptionsConfig()
	{
		if (PDARPDebugMode) Print(PDARPModPreffix + "LoadOptionsConfig.");
		JsonFileLoader<ref PluginPDARP_Options>.JsonLoadFile(m_configDir + optionsJsonFilename, m_options);
	}
	
	void SaveContactsConfig()
	{
		if (PDARPDebugMode) Print(PDARPModPreffix + "SaveContactsConfig.");
		JsonFileLoader<ref array<ref PluginPDARP_Conversation>>.JsonSaveFile(m_configDir + contactsJsonFilename, m_contacts);
	}
	
	void LoadContactsConfig()
	{
		if (PDARPDebugMode) Print(PDARPModPreffix + "LoadContactsConfig.");
		JsonFileLoader<ref array<ref PluginPDARP_Conversation>>.JsonLoadFile(m_configDir + contactsJsonFilename, m_contacts);
	}
	
	void SaveCommentsConfig(string uid)
	{
		if (PDARPDebugMode) Print(PDARPModPreffix + "SaveCommentsConfig: " + uid);
		
		if (m_comments.Contains(uid))
		{
			ref array<ref PluginPDARP_Comment> comments = m_comments[uid];
			
			while (comments.Count() > 100)
			{
				comments.RemoveOrdered(0);
			}
			
			JsonFileLoader<ref array<ref PluginPDARP_Comment>>.JsonSaveFile(m_configDir + uid + ".json", comments);
		}
	}
	
	void LoadCommentsConfig(string uid)
	{
		if (PDARPDebugMode) Print(PDARPModPreffix + "LoadCommentsConfig: " + uid);
		
		string fileName = m_configDir + uid + ".json";
		if (FileExist(fileName))
		{
			ref array<ref PluginPDARP_Comment> result = new array<ref PluginPDARP_Comment>(); 
			JsonFileLoader<ref array<ref PluginPDARP_Comment>>.JsonLoadFile(fileName, result);
			m_comments[uid] = result;
		}
	}
	
	ref array<ref PluginPDARP_Comment> GetComments(string uid)
	{
		if (!m_comments.Contains(uid))
		{
			LoadCommentsConfig(uid);
		}

		if (!m_comments.Contains(uid))
		{
			m_comments[uid] = new array<ref PluginPDARP_Comment>();
		}
		
		return m_comments[uid];
	}
	
	void AddComment(string contactId, string senderId, string message)
	{
		if (contactId.Length() > 0)
		{
			ref array<ref PluginPDARP_Comment> comments = GetComments(contactId);
			ref PluginPDARP_Comment comment = new PluginPDARP_Comment();
			comment.m_UID = senderId;
			comment.m_Message = message;
			comments.Insert(comment);
			
			SaveCommentsConfig(contactId);
			
			ref PluginPDARP_Conversation contact = FindContact(contactId);
			if (contact != null)
			{
				contact.m_Unreaded = contact.m_Unreaded + 1;
				SaveContactsConfig();
			}
			
			if (IsOpen())
			{
				m_PDARPMenu.m_sendMessageStatus = 2;
			}
		}
		else
		{
			if (IsOpen())
			{
				m_PDARPMenu.m_sendMessageStatus = 0;
			}
				
			ref array<string> request = new array<string>();
			for (int i = 0; i < m_contacts.Count(); i++)
			{
				request.Insert(m_contacts[i].m_UID);
			}
			GetRPCManager().SendRPC( PDARPModPreffix, "CheckContacts", new Param1< ref array<string> >( request ), true );
		}
	}
	
	bool IsOpen()
	{
		return m_PDARPMenu && m_PDARPMenu.m_active;
	}
	
	void Open()
	{
		if (IsOpen())
		{
			Close();
		}
		
		if (GetGame().GetUIManager().GetMenu() != NULL)
		{
			if (PDARPDebugMode) Print(PDARPModPreffix + "OpenRecipesBookAction ActionCondition blocking by external menu: " + GetGame().GetUIManager().GetMenu());
			return;
		}
		
		if (PDARPDebugMode) Print(PDARPModPreffix + "PluginPDARP prepare open menu");
		m_PDARPMenu = new PDARPMenu;
		m_PDARPMenu.Init();
		GetGame().GetUIManager().ShowScriptedMenu( m_PDARPMenu, NULL );
		if (PDARPDebugMode) Print(PDARPModPreffix + "PluginPDARP post open menu: " + m_PDARPMenu);
	}
	
	ref PluginPDARP_Conversation FindContact(string uid)
	{
		if (PDARPDebugMode) Print(PDARPModPreffix + "FindContact: " + uid);
		
		for (int i = 0; i < m_contacts.Count(); i++)
		{
			ref PluginPDARP_Conversation item = m_contacts[i];
			if (item.m_UID == uid)
			{
				return item;
			}
		}
		
		return null;
	}
	
	void AddContact(string uid, string name)
	{
		if (PDARPDebugMode) Print(PDARPModPreffix + "AddContact X1: " + uid + "; " + name);
		if (uid.Length() > 0 && name.Length() > 0)
		{
			if (PDARPDebugMode) Print(PDARPModPreffix + "AddContact X2: " + uid + "; " + name);
			if (FindContact(uid) == null)
			{
				if (PDARPDebugMode) Print(PDARPModPreffix + "AddContact X3: " + uid + "; " + name);
				ref PluginPDARP_Conversation conv = new PluginPDARP_Conversation();
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
			m_PDARPMenu.m_addContactStatus = 2;
		}
	}
	
	void RemoveContact(string uid)
	{
		if (PDARPDebugMode) Print(PDARPModPreffix + "RemoveContact: " + uid);
		
		ref PluginPDARP_Conversation contactToDelete = FindContact(uid);
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
		if (PDARPDebugMode) Print(PDARPModPreffix + "RenameContact: " + uid);
		
		ref PluginPDARP_Conversation contactToRename = FindContact(uid);
		if (contactToRename == null)
		{
			return;
		}
		
		contactToRename.m_Name = newName;
		SaveContactsConfig();
	}
	
	void BanUnbanContact(string uid)
	{
		if (PDARPDebugMode) Print(PDARPModPreffix + "BanContact: " + uid);
		
		ref PluginPDARP_Conversation contactToRename = FindContact(uid);
		if (contactToRename == null)
		{
			return;
		}
		
		contactToRename.m_IsBanned = !contactToRename.m_IsBanned;
		SaveContactsConfig();
	}
	
	void Close()
	{
		if (m_PDARPMenu)
		{
			m_PDARPMenu.m_active = false;
		}

		if (PDARPDebugMode) Print(PDARPModPreffix + "PluginPDARP close menu: " + m_PDARPMenu);
	}
};

class PluginPDARP_Conversation
{
	string m_UID;
	string m_Name;
	int m_Unreaded;
	bool m_IsBanned;

	void PluginPDARP_Conversation()
	{
	
	}
};

class PluginPDARP_Comment
{
	string m_UID;
	string m_Message;
	
	void PluginPDARP_Comment()
	{
	
	}
};

class PluginPDARP_Options
{
	bool m_Muted;
	bool m_HideId;
	
	void PluginPDARP_Options()
	{
	
	}
};