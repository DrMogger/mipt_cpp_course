#include "agent.h"

#include <print>
#include <agent.h>
#include <fields.h>

namespace nano_edr {

void Agent::PrintContext() const {
    if (prev_){
        nano_edr::EventNode *tail = prev_->next;
        int i = tail ? 2 : 1;
        auto* current = prev_;
        while (current) {
            std::print("[CTX] -{}: ts={} type={} pid={}\n", i, current->event.ts(), current->event.type(), current->event.pid());
            current = current->next;
            i--;
        }
    }
}

void Agent::HandleEvent(const Event& event) {
    const EventNode* tail = window_.back();
    int matched = 0;
    ++lines_;
    ++types_[event.type()];

    matched += CheckRules(event, AgentRules(), AgentRuleCount());
    if(matched && !quiet_)   PrintSummary();
    prev_ = tail;

    window_.PushBack(event);

    if (window_.capacity() == 1)   prev_ = nullptr;
}


static constexpr Rule kRules[] = {
    {"script_host_from_temp", ScriptHostFromTemp, Severity::kHigh},
    {"lolbin_download", LolBinDownload, Severity::kHigh},
    {"hidden_powershell", HiddenPowershell, Severity::kMedium},
    {"autostart_write", AutostartWrite, Severity::kHigh},
    {"ransom_extension", RansomExtension, Severity::kCritical},
};


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
    const auto* path = FindField(event, "path");
    if (!path) {
        path = FindField(event, "to");
        if (!path) {
            return false;
        }
    }
    if (NormalizePath(*path).find("\\start menu\\programs\\startup\\") != std::string::npos) {
        if (IsProcessStart(event) || IsFileWrite(event) || (event.type() == "file_move") || (event.type() == "file_create")) {
            return true;
        }
    }
    return false;
}

bool RansomExtension(const Event& event) {
    const auto* path = FindField(event, "path");
    if (!path) {
        path = FindField(event, "to");
        if (!path) {
            path = FindField(event, "from");
            if (!path) {
                return false;
            }
        }
    }
    return (NormalizePath(*path).ends_with(".locked")) && (IsProcessStart(event) || IsFileWrite(event) || (event.type() == "file_move"));
}

}