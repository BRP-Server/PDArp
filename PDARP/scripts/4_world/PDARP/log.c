enum PDArpLogLevel {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4
}

string logLevels[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR"};

class PDArpLog {
	
	PDArpLogLevel logLevel;
	
	void PDArpLog(PDArpLogLevel level) {
		logLevel = level;
	}

    private void Log(PDArpLogLevel level, string msg) {
        if (logLevel <= level) {
            Print(PDArpModPreffix + " [" + logLevels[level] + "] " + msg);
        }
    }

    void SetLogLevel(PDArpLogLevel level) {
        logLevel = level;
    }

    void Trace(string msg) {
        Log(PDArpLogLevel.Trace, msg);
    }

    void Debug(string msg) {
        Log(PDArpLogLevel.Debug, msg);
    }

    void Info(string msg) {
        Log(PDArpLogLevel.Info, msg);
    }

    void Warn(string msg) {
        Log(PDArpLogLevel.Warn, msg);
    }

    void Error(string msg) {
        Log(PDArpLogLevel.Error, msg);
    }
}


ref PDArpLog g_PDArpLog;
ref PDArpLog GetPDArpLog() {
	if (!g_PDArpLog) {
		g_PDArpLog = new ref PDArpLog(PDArpLogLevel.Info)
	}
	return g_PDArpLog;
}