enum PDArpLogLevel {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4
}

string logLevels[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR"};

class PDArpLog {

    static private void Log(PDArpLogLevel level, string msg) {
        PluginPDArp pluginPDArp;
		Class.CastTo(pluginPDArp, GetPlugin(PluginPDArp));
        if (pluginPDArp.m_settings.logLevel <= level) {
            Print(PDArpModPreffix + " [" + logLevels[level] + "] " + msg);
        }
    }

    static void Trace(string msg) {
        Log(PDArpLogLevel.Trace, msg);
    }

    static void Debug(string msg) {
        Log(PDArpLogLevel.Debug, msg);
    }

    static void Info(string msg) {
        Log(PDArpLogLevel.Info, msg);
    }

    static void Warn(string msg) {
        Log(PDArpLogLevel.Warn, msg);
    }

    static void Error(string msg) {
        Log(PDArpLogLevel.Error, msg);
    }
}