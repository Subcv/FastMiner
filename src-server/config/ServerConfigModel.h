#pragma once
#include "core/DispatcherConfig.h"

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace fm::server {

enum class SilkTouchMode : int {
    Unlimited = 0, // 无限制
    Forbid    = 1, // 禁止精准附魔
    Need      = 2  // 需要精准附魔
};

using MinerTools   = std::unordered_set<std::string>;
using SimilarBlock = std::unordered_set<std::string>;

struct BlockConfig {
    std::string   name;
    int           cost{0};
    int           limit{256};
    DestroyMode   destroyMode{DestroyMode::Default};
    SilkTouchMode silkTouchMode{SilkTouchMode::Unlimited};
    MinerTools    tools{};
    SimilarBlock  similarBlock{};
};

using Blocks = std::unordered_map<std::string, BlockConfig>;

struct ServerConfigModel {
    static constexpr int SchemaVersion = 9;

    int version = SchemaVersion;

    DispatcherConfig dispatcher;

    bool telemetry{true};

    struct EconomyConfig {
        enum class EconomyKit { LegacyMoney, ScoreBoard };

        bool        enabled        = false;
        EconomyKit  kit            = EconomyKit::LegacyMoney;
        std::string scoreboardName = "Scoreboard";
    } economy;

    /**
     * @brief 玩家自己添加的连锁方块的经济扣费。
     * @note 独立于 economy.enabled：即使全局经济关闭，玩家自定义方块也可以单独收费。
     */
    struct PlayerBlockEconomyConfig {
        bool enabled{true};
        int  cost{1};          // 普通工具每连锁 1 格扣多少
        int  silkTouchCost{3}; // 精准采集工具每连锁 1 格扣多少
    } playerBlockEconomy;

    Blocks blocks;
};

} // namespace fm::server
