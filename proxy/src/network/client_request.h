#include <nlohmann/json.hpp>
#include <vector>
using json = nlohmann::json;

namespace network {

struct HarpoonFile {
    std::string file;
    std::vector<std::string> functions;
};

struct Content {
    std::string prompt;
    std::optional<std::vector<HarpoonFile>> harpoonFiles;
    std::optional<std::string>  gitDiff;
    std::optional<std::string> fileStructure;
};

struct ClientRequest {
    std::string request_type;
    std::string project_id;
    std::optional<Content> content;
};

void from_json(const json &j, HarpoonFile &harpoon);
void from_json(const json &j, Content &content);
void from_json(const json &j, ClientRequest &request);

} // namespace network
