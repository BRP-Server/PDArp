
class PDArpLog {

    static void Trace(string msg) {
        Print(PDArpModPreffix + "[TRACE] " + msg);
    }

    static void Debug(string msg) {
        if (PDArpDebugMode) {
            Print(PDArpModPreffix + "[DEBUG] " + msg);
        }
    }

    static void Info(string msg) {
        Print(PDArpModPreffix + "[INFO] " + msg);
    }

    static void Warn(string msg) {
        Print(PDArpModPreffix + "[WARN] " + msg);
    }

    static void Error(string msg) {
        Print(PDArpModPreffix + "[ERROR] " + msg);
    }
}