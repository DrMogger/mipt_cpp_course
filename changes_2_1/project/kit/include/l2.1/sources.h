#ifndef NANO_EDR_SOURCES_H
#define NANO_EDR_SOURCES_H

#include <fstream>
#include <string>

#include "agent.h"

namespace nano_edr {

class FileSource {
 public:
    explicit FileSource(const std::string& path);
    void Run(Agent* agent);

 private:
    std::string path_;
    std::ifstream file_;
};

}  // namespace nano_edr

#endif