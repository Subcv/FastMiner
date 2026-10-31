#pragma once
#include "IPlatformService.h"

#include <econbridge/IEconomy.h>

#include <memory>

namespace fm {
namespace server {

class ServerPlatformService final : public IPlatformService {
    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    ServerPlatformService();
    ~ServerPlatformService() override;

    bool init() override;
    bool destroy() override;

    econbridge::IEconomy& getEconomy() const;

    /** 玩家自定义方块专用经济（独立于全局 economy.enabled） */
    econbridge::IEconomy& getPlayerBlockEconomy() const;
};

} // namespace server
} // namespace fm
