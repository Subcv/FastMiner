#pragma once
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/block/Block.h"

namespace fm::server::gui {

void __sendEditBlockTools(Player& player, std::string const& typeName);

void __sendEditSimilarBlock(Player& player, std::string const& typeName);

void _sendEditBlockConfig(Player& player, std::string const& typeName);

void _addHandheldItemBlock(Player& player);

void _sendBlockViewer(Player& player, std::string const& typeName);

void sendOpBlockManager(Player& player);

void sendPlayerConfigGUI(Player& player);

/* 玩家自定义连锁方块（所有玩家可用，单次上限 64） */
void addPlayerTargetBlock(Player& player);

void removePlayerTargetBlock(Player& player);

void sendPlayerBlocksGUI(Player& player);


} // namespace fm::server::gui