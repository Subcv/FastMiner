#pragma once
#include "core/MinerLauncher.h"

namespace fm::server {

class ServerMinerLauncher final : public MinerLauncher {
public:
    bool isMinerEnabled(Player& player, const std::string& blockType) override;
    bool canDestroyBlockWithConfig(Player& player, const RuntimeSingleBlockConfigPtr& rtConfig) override;
    int  calculateLimit(ChainTaskContext const& ctx) override;

    RuntimeSingleBlockConfigPtr
    loadRuntimeSingleBlockConfig(std::string const& blockType, Player& player) override;

protected:
    void
    launchChainTask(ChainTaskContext ctx, TaskDispatcher& dispatcher, std::optional<PreSearchData> preSearch) override;
};

} // namespace fm::server