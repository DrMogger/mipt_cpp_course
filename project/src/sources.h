#ifndef NANO_EDR_SOURCES_H
#define NANO_EDR_SOURCES_H

#include <fstream>
#include <string>

#include "event_source.h"
#include "os_handle.h"

namespace nano_edr {

class FileSource : public IEventSource {
 public:
    explicit FileSource(const std::string& path);
    void Run(IEventHandler* handler) override;

 private:
    std::string path_;
    std::ifstream file_;
};

class OsSource : public IEventSource {
 public:
    explicit OsSource(const std::string& config_path);
    void Run(IEventHandler* handler) override;

 private:
    static void Trampoline(const os_event* ev, void* ctx) noexcept;

    OsHandle handle_;
};

}  // namespace nano_edr

#endif  // NANO_EDR_SOURCES_H
