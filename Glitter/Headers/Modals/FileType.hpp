#include <string_view>

enum class FileType : uint8_t {
    CharacterType,
    ModelType,
    BlendSpaceType,
    StateMachineType,
    AnimationType,
    MetaJSONType,
    Unknown
};

inline constexpr std::string_view toString(FileType type) {
    switch (type) {
        case FileType::CharacterType:    return "character";
        case FileType::ModelType:        return "model";
        case FileType::BlendSpaceType:   return "blendspace";
        case FileType::StateMachineType: return "statemachine";
        case FileType::AnimationType:    return "animation";
        case FileType::MetaJSONType:    return "meta.json";
        default:                     return "unknown";
    }
}

inline constexpr FileType fromString(std::string_view s) {
    if (s == "character")    return FileType::CharacterType;
    if (s == "model")        return FileType::ModelType;
    if (s == "blendspace")   return FileType::BlendSpaceType;
    if (s == "statemachine") return FileType::StateMachineType;
    if (s == "animation")    return FileType::AnimationType;
    if (s == "meta.json")    return FileType::MetaJSONType;
    return FileType::Unknown;
}