#include "agent_rules.h"

#include <memory>
#include <utility>
#include "combinators.h"
#include "make_rule.h"

namespace nano_edr {
namespace {



std::unique_ptr<AnyOf> EndsWithAny(const std::string& key,
                                  const std::vector<std::string>& suffixes) {
    auto condition = std::make_unique<AnyOf>();
    for (const std::string& suffix : suffixes) {
        condition->Add(std::make_unique<FieldEndsWith>(key, suffix));
    }
    return condition;
}

std::unique_ptr<AnyOf> ContainsAny(const std::string& key,
                                  const std::vector<std::string>& needles) {
    auto condition = std::make_unique<AnyOf>();
    for (const std::string& needle : needles) {
        condition->Add(std::make_unique<FieldContains>(key, needle));
    }
    return condition;
}

std::unique_ptr<AllOf> ScriptHostFromTemp() {
    auto condition = std::make_unique<AllOf>();
    condition->Add(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kProcessStart}));
    condition->Add(EndsWithAny("image", {"wscript.exe", "cscript.exe"}));
    condition->Add(ContainsAny("cmdline", {
        "\\appdata\\local\\temp\\", "/appdata/local/temp/",
        "\\windows\\temp\\", "/windows/temp/", "%temp%", "%tmp%"}));
    return condition;
}

std::unique_ptr<AnyOf> FilePaths(std::unique_ptr<ICondition> by_path,
                                std::unique_ptr<ICondition> by_to) {
    auto writes = std::make_unique<AllOf>();
    writes->Add(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kFileCreate, EventType::kFileWrite}));
    writes->Add(std::move(by_path));

    auto moves = std::make_unique<AllOf>();
    moves->Add(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kFileMove}));
    moves->Add(std::move(by_to));

    auto paths = std::make_unique<AnyOf>();
    paths->Add(std::move(writes));
    paths->Add(std::move(moves));
    return paths;
}

}  // namespace

void RegisterAgentRules(RuleEngine* engine) {
    auto script_host = std::make_unique<MatchRule>("script_host_from_temp", Severity::kHigh);
    script_host->AddCondition(ScriptHostFromTemp());
    script_host->AddAction("kill_process");
    engine->AddRule(std::move(script_host));

    auto lolbin = std::make_unique<MatchRule>("lolbin_download", Severity::kHigh);
    lolbin->AddCondition(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kProcessStart}));
    lolbin->AddCondition(EndsWithAny("image", {"certutil.exe", "bitsadmin.exe"}));
    lolbin->AddCondition(ContainsAny("cmdline", {"transfer", "urlcache", "http:", "https:"}));
    lolbin->AddAction("kill_process");
    engine->AddRule(std::move(lolbin));

    auto powershell = std::make_unique<MatchRule>("hidden_powershell", Severity::kMedium);
    powershell->AddCondition(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kProcessStart}));
    powershell->AddCondition(EndsWithAny("image", {"powershell.exe", "pwsh.exe"}));
    powershell->AddCondition(ContainsAny("cmdline", {
        "-windowstyle hidden", "-enc", "-encodedcommand", "-w hidden"}));
    powershell->AddAction("kill_process");
    engine->AddRule(std::move(powershell));

    const std::vector<std::string> startup_paths = {
        "\\start menu\\programs\\startup\\", "/start menu/programs/startup/"};

    auto autostart = std::make_unique<MatchRule>("autostart_write", Severity::kHigh);
    autostart->AddCondition(FilePaths(
        ContainsAny("path", startup_paths), ContainsAny("to", startup_paths)));
    autostart->AddAction("quarantine_file");
    engine->AddRule(std::move(autostart));

    auto ransom = std::make_unique<MatchRule>("ransom_extension", Severity::kCritical);
    ransom->AddCondition(FilePaths(
        std::make_unique<FieldEndsWith>("path", ".locked"),
        std::make_unique<FieldEndsWith>("to", ".locked")));
    ransom->AddAction("quarantine_file");
    engine->AddRule(std::move(ransom));

    auto beacon = std::make_unique<SequenceRule>("script_host_beacon", Severity::kCritical, 10000);
    beacon->SetFirst(ScriptHostFromTemp());
    beacon->SetSecond(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kNetConnect}));
    beacon->AddAction("kill_process");
    engine->AddRule(std::move(beacon));

    auto mass_write = MakeRule<ThresholdRule<128>>("ransomware_mass_write", Severity::kCritical, size_t(20), uint64_t(10000), std::string("pid"));
    mass_write->SetCondition(MakeCondition(
        AnyOfStatic(
            AllOfStatic(
                EventTypeIs({EventType::kFileCreate, EventType::kFileWrite}),
                AnyOfStatic(
                    FieldContains("path", "\\documents\\"),
                    FieldContains("path", "/documents")
                )
            )
            AllOfStatic(
                EventTypeIs({EventType::kFileMove})),
                AnyOfStatic(
                    FieldContains("to", "\\documents\\"),
                    FieldContains("to", "/documents/")
                )
            )
        ));
    mass_write->AddAction("kill_process");
    mass_write->AddAction("quarantine_file");
}

}  // namespace nano_edr
