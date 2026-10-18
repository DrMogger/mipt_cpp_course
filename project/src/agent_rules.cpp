#include "agent_rules.h"
#include "fields.h"

namespace nano_edr {

bool ImageEndsWith(const Event& event, const std::string& suffix) {
    const auto& image = GetRequiredField(event, "image");
    return NormalizePath(image).ends_with(NormalizePath(suffix));
}



bool ScriptHostFromTemp(const Event& event) {
    if (!IsProcessStart(event)) return false;
    return (ImageEndsWith(event, "cscript.exe") || ImageEndsWith(event, "wscript.exe")) && (CommandLineContains(event, "\\appdata\\local\\temp\\") || CommandLineContains(event, "\\windows\\temp\\"));
}

bool LolBinDownload(const Event& event) {
    if (!IsProcessStart(event)) return false;
    return (ImageEndsWith(event, "certutil.exe") || ImageEndsWith(event, "bitsadmin.exe")) && (CommandLineContains(event, "transfer") || CommandLineContains(event, "urlcache") || CommandLineContains(event, "http:") || CommandLineContains(event, "https:"));
}

bool HiddenPowershell(const Event& event) {
    if (!IsProcessStart(event)) return false;
    return (ImageEndsWith(event, "powershell.exe") || ImageEndsWith(event, "pwsh.exe")) && (CommandLineContains(event, "-windowstyle hidden") || CommandLineContains(event, "-enc") || CommandLineContains(event, "-encodedcommand") || CommandLineContains(event, "-w hidden"));
}

bool AutostartWrite(const Event& event) {
    if (!IsFileWrite(event) && event.type() != "file_create" && event.type() != "file_move") {
        return false;
    }
    const auto* path = FindField(event, event.type() == "file_move" ? "to" : "path");
    if (!path) {
        return false;
    }
    return NormalizePath(*path).find("\\start menu\\programs\\startup\\") != std::string::npos;
}

bool RansomExtension(const Event& event) {
    if (!IsFileWrite(event) && event.type() != "file_create" && event.type() != "file_move") {
        return false;
    }
    const auto* path = FindField(event, event.type() == "file_move" ? "to" : "path");
    if (!path) {
        return false;
    }
    return NormalizePath(*path).ends_with(".locked");
}

constexpr Rule kRules[] = {
    {"script_host_from_temp", ScriptHostFromTemp, Severity::kHigh},
    {"lolbin_download", LolBinDownload, Severity::kHigh},
    {"hidden_powershell", HiddenPowershell, Severity::kMedium},
    {"autostart_write", AutostartWrite, Severity::kHigh},
    {"ransom_extension", RansomExtension, Severity::kCritical},
};

const Rule* AgentRules() {
    return kRules;
}
size_t AgentRuleCount() {
    return sizeof(kRules) / sizeof(kRules[0]);
}

}
