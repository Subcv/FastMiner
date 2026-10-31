#include "ServerPlatformService.h"

#include "command/FastMinerCommand.h"
#include "config/ServerConfigImpl.h"
#include "config/ServerConfigModel.h"

#include "econbridge/detail/LegacyMoneyEconomy.h"
#include "econbridge/detail/NullEconomy.h"
#include "econbridge/detail/ScoreboardEconomy.h"

namespace fm {
namespace server {

struct ServerPlatformService::Impl {
    std::unique_ptr<econbridge::IEconomy> mEconomy{nullptr};
    std::unique_ptr<econbridge::IEconomy> mPlayerBlockEconomy{nullptr};

    static std::unique_ptr<econbridge::IEconomy> createEconomy(bool enabled) {
        if (!enabled) {
            return std::make_unique<econbridge::detail::NullEconomy>();
        }
        switch (ServerConfigImpl::model.economy.kit) {
        case ConfigModel::EconomyConfig::EconomyKit::LegacyMoney:
            return std::make_unique<econbridge::detail::LegacyMoneyEconomy>();
        case ConfigModel::EconomyConfig::EconomyKit::ScoreBoard:
            return std::make_unique<econbridge::detail::ScoreboardEconomy>(
                ServerConfigImpl::model.economy.scoreboardName
            );
        }
        return std::make_unique<econbridge::detail::NullEconomy>();
    }

    void initEconomy() {
        mEconomy            = createEconomy(ServerConfigImpl::model.economy.enabled);
        mPlayerBlockEconomy = createEconomy(ServerConfigImpl::model.playerBlockEconomy.enabled);
    }
};

ServerPlatformService::ServerPlatformService() : impl(std::make_unique<Impl>()) {}

ServerPlatformService::~ServerPlatformService() = default;

bool ServerPlatformService::init() {
    FastMinerCommand::setup();
    impl->initEconomy();
    StaticGlobalConfigHost::getInstance().buildRuntimeMap();
    return true;
}

bool ServerPlatformService::destroy() {
    impl->mEconomy.reset();
    impl->mPlayerBlockEconomy.reset();
    return true;
}

econbridge::IEconomy& ServerPlatformService::getEconomy() const { return *impl->mEconomy; }

econbridge::IEconomy& ServerPlatformService::getPlayerBlockEconomy() const {
    return *impl->mPlayerBlockEconomy;
}


} // namespace server
} // namespace fm