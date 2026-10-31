#include "ServerMinerLauncher.h"

#include "FastMiner.h"
#include "ServerPlatformService.h"
#include "config/ServerConfigImpl.h"
#include "config/StaticGlobalConfigHost.h"
#include "core/ChainTask.h"
#include "utils/McUtils.h"

#include <mc/world/item/enchanting/EnchantUtils.h>

#include <memory>
#include <optional>
#include <utility>

namespace fm::server {

/**
 * @brief 服务器完成策略：经济扣费 + 结算提示。
 * @note 编译期注入 ChainTask<ServerChainFinisher>。
 */
struct ServerChainFinisher {
    template <typename Task>
    void operator()(Task const& task, long long cpuTime) const {
        if (task.count_ <= 0) {
            return;
        }

        auto& platform = FastMiner::getInstance().getPlatformService().as<ServerPlatformService>();
        auto& rtConfig = *task.blockConfig();
        auto  cost     = rtConfig.rawConfig.cost * task.count_;
        auto& economy  = rtConfig.isPlayerBlock ? platform.getPlayerBlockEconomy() : platform.getEconomy();
        economy.reduce(task.player_.getUuid(), cost);
        mc_utils::sendText(
            task.player_,
            "本次连锁了 {} 个方块, 消耗了 {} 点耐久, 花费 {} 点经济, 总耗时 {}ms",
            task.count_,
            task.deductDamage(),
            cost,
            cpuTime
        );
    }
};

bool ServerMinerLauncher::isMinerEnabled(Player& player, const std::string& blockType) {
    if (!player.isSurvival()) {
        FM_TRACE("player not survival");
        return false;
    }

    auto& inst = StaticGlobalConfigHost::getInstance().as<ServerConfigImpl>();
    auto& uuid = player.getUuid();
    if (!inst.isEnabled(uuid, ServerConfigImpl::KEY_ENABLE.data())) {
        FM_TRACE("player miner disabled");
        return false;
    }

    bool sneakingRequired = inst.isEnabled(uuid, ServerConfigImpl::KEY_SNEAK.data());
    bool sneaking         = mc_utils::isSneaking(player);
    if (sneakingRequired && !sneaking) {
        FM_TRACE("sneaking required but player is not sneaking");
        return false;
    }
    return inst.isEnabled(uuid, blockType) || inst.hasPlayerBlock(uuid, blockType);
}

RuntimeSingleBlockConfigPtr ServerMinerLauncher::loadRuntimeSingleBlockConfig(
    std::string const& blockType,
    Player&            player
) {
    // 先走全局方块配置（原版 FastMiner 行为）
    if (auto global = MinerLauncher::loadRuntimeSingleBlockConfig(blockType); global) {
        return global;
    }

    // 再看是不是该玩家自己添加的方块
    auto& inst = StaticGlobalConfigHost::getInstance().as<ServerConfigImpl>();
    auto& uuid = player.getUuid();
    if (!inst.hasPlayerBlock(uuid, blockType)) {
        return nullptr;
    }

    // 玩家自定义方块：6 面相邻、无工具限制、不限制精准采集、上限固定 64
    // playerDestroy 由 ChainTask 调用，因此时运/精准采集附魔会正常生效
    // 经济：默认每连锁 1 格扣 1 点；手持精准采集工具时每格扣 3 点（可在 Config.json 调整）
    auto const& ecoCfg   = ServerConfigImpl::model.playerBlockEconomy;
    bool        silkTouch = EnchantUtils::hasEnchant(Enchant::Type::SilkTouch, player.getSelectedItem());

    SingleBlockConfig single{};
    single.name          = inst.getPlayerBlockName(uuid, blockType);
    single.cost          = ecoCfg.enabled ? (silkTouch ? ecoCfg.silkTouchCost : ecoCfg.cost) : 0;
    single.limit         = ServerConfigImpl::PlayerBlockChainLimit;
    single.destroyMode   = DestroyMode::Default;
    single.silkTouchMode = SilkTouchMode::Unlimited;

    auto rtConfig            = inst.buildRuntimeSingleBlockConfig(std::move(single));
    rtConfig->isPlayerBlock = true;
    return rtConfig;
}

bool ServerMinerLauncher::canDestroyBlockWithConfig(Player& player, const RuntimeSingleBlockConfigPtr& rtConfig) {
    const auto& config = rtConfig->rawConfig;
    const auto& item   = player.getSelectedItem();

    if (!config.tools.empty() && !config.tools.contains(item.getTypeName())) {
        return false;
    }

    const bool hasSilkTouch = EnchantUtils::hasEnchant(Enchant::Type::SilkTouch, item);
    switch (config.silkTouchMode) {
    case SilkTouchMode::Unlimited:
        return true;
    case SilkTouchMode::Forbid:
        return !hasSilkTouch;
    case SilkTouchMode::Need:
        return hasSilkTouch;
    }
    return false;
}

int ServerMinerLauncher::calculateLimit(const ChainTaskContext& ctx) {
    int limit = MinerLauncher::calculateLimit(ctx);
    auto& platform = FastMiner::getInstance().getPlatformService().as<ServerPlatformService>();

    if (ctx.rtConfig->isPlayerBlock) {
        // 玩家自定义方块：按玩家自定义经济余额限制，避免扣成负数
        int unitCost = ctx.rtConfig->rawConfig.cost;
        if (ServerConfigImpl::model.playerBlockEconomy.enabled && unitCost > 0) {
            limit = std::min(
                limit,
                static_cast<int>(platform.getPlayerBlockEconomy().get(ctx.player.getUuid()) / unitCost)
            );
        }
    } else if (ServerConfigImpl::model.economy.enabled && ctx.rtConfig->rawConfig.cost > 0) {
        // 全局方块：原有逻辑
        limit = std::min(
            limit,
            static_cast<int>(
                platform.getEconomy().get(ctx.player.getUuid()) / ctx.rtConfig->rawConfig.cost
            )
        );
    }
    return limit;
}

void ServerMinerLauncher::launchChainTask(
    ChainTaskContext             ctx,
    TaskDispatcher&              dispatcher,
    std::optional<PreSearchData> preSearch
) {
    auto task = std::make_shared<ChainTask<ServerChainFinisher>>(std::move(ctx), dispatcher, std::move(preSearch));
    dispatcher.launch(task);
}

} // namespace fm::server