// Copyright (C) 2022 FelicitusNeko
// 
// This file is part of Meritous-AP.
// 
// Meritous-AP is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Meritous-AP is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Meritous-AP.  If not, see <http://www.gnu.org/licenses/>.

#ifndef ITEMHANDLER_H
#define ITEMHANDLER_H

#include "itemdefs.h"

void InitStores();
void DestroyStores();

t_itemTypes MakeCrystals();

int CostFactor(t_itemStores store);
void CollectItem(t_itemStores store);
void CollectSpecialItem(t_specialStore itemIndex);

char isArchipelago();
void PostCollectNotice(const char *player, const char *itemName, const char *waswere);

void PollAPClient();
void ReceiveItem(t_itemTypes item, const char *source);
void SendAPSignal(t_apSignal signal);
void KillPlayer(const char *from);
void AnnounceDeath();
void AnnounceVictory(char isFullVictory);

void WriteStoreData();
void ReadStoreData();

#endif