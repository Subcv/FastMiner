#pragma once
#include <mc/platform/UUID.h>

#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fm::server {

class IServerPlayerConfig {
public:
    virtual ~IServerPlayerConfig() = default;

    inline static constexpr std::string_view PlayerConfigFileName = "PlayerSetting.json";

    /** 玩家自定义方块单独存一个文件，避免改动 PlayerSetting.json 的旧格式 */
    inline static constexpr std::string_view PlayerBlocksFileName = "PlayerBlocks.json";

    inline static constexpr std::string_view KEY_ENABLE           = "enable";
    inline static constexpr std::string_view KEY_SNEAK            = "sneak";

    /** 玩家自己添加的方块，单次连锁上限（需求写死 64，不读取全局方块配置） */
    inline static constexpr int PlayerBlockChainLimit = 64;

    using BlockState       = std::unordered_map<std::string, bool>;
    using PlayerBlockState = std::unordered_map<mce::UUID, BlockState>;

    /** 方块类型 -> 显示名称 */
    using PlayerBlockList = std::unordered_map<std::string, std::string>;
    using PlayerBlocks    = std::unordered_map<mce::UUID, PlayerBlockList>;

    virtual bool isEnabled(mce::UUID const& uuid, std::string const& key) = 0;

    virtual void setEnabled(mce::UUID const& uuid, std::string const& key, bool enabled) = 0;

    virtual void enable(mce::UUID const& uuid, std::string const& key) = 0;

    virtual void disable(mce::UUID const& uuid, std::string const& key) = 0;

    virtual bool hasPlayer(mce::UUID const& uuid)                                = 0;
    virtual bool hasBlock(mce::UUID const& uuid, std::string const& key)    = 0;
    virtual void removeBlock(mce::UUID const& uuid, std::string const& key) = 0;
    virtual void ensurePlayerBlockConfig()                                       = 0;

    /* 玩家自己添加的连锁方块 */
    virtual bool hasPlayerBlock(mce::UUID const& uuid, std::string const& key) = 0;
    virtual bool
    addPlayerBlock(mce::UUID const& uuid, std::string const& key, std::string const& name) = 0;
    virtual bool removePlayerBlock(mce::UUID const& uuid, std::string const& key)         = 0;
    virtual std::string getPlayerBlockName(mce::UUID const& uuid, std::string const& key) = 0;
    virtual std::vector<std::pair<std::string, std::string>> listPlayerBlocks(mce::UUID const& uuid) = 0;
};

} // namespace fm::server