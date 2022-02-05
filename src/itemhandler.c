/**
 * Copyright (C) 2022 FelicitusNeko
 * 
 * This file is part of Meritous-AP.
 * 
 * Meritous-AP is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Meritous-AP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Meritous-AP.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <SDL.h>

#include "audio.h"
#include "boss.h"
#include "demon.h"
#include "itemstore.h"
#include "levelblit.h"

void InitStores() {
  LocalGenerateItemStores(0);
}

void DestroyStores() {
  DestroyItemStores();
}

int CostFactor(t_itemStores store) {
  if (store >= IS_ALPHA && store <= IS_GAMMA) {
    return GetNextIndexInStore(store);
  }
  else return -1;
}

t_itemTypes MakeCrystals() {
  int randCrystals = rand() % 32;
  if (randCrystals < 24) return T_CRYSTALS_500;
  if (randCrystals < 31) return T_CRYSTALS_1000;
  return T_CRYSTALS_2000;
}

char HasItem(t_itemTypes item) {
  switch (item) {
    case T_NOTHING:
      return 1;
    case T_REFLECT_SHIELD:
      return player_shield >= 25;
    case T_CIRCUIT_CHARGE:
      return circuit_fillrate >= 25;
    case T_CIRCUIT_REFILL:
      return circuit_recoverrate >= 25;
    case T_AGATE_KNIFE:
      return player_shield == 30;
    case T_CRYSTALS_500:
    case T_CRYSTALS_1000:
    case T_CRYSTALS_2000:
      return 0;
    case T_EVOLUTION_TRAP:
      return enemy_evolutions >= 3;
    default:
      return artifacts[item - T_MAP];
  }
}

void ProcessItem(t_itemTypes item) {
  char noise = 0;
  int basemod = 500;

  if (HasItem(item)) item = MakeCrystals();

  switch (item) {
    case T_NOTHING:
      break;
    case T_REFLECT_SHIELD:
      specialmessage = 10;
      specialmessagetimer = 30;
      player_shield++;
      noise = 1;
      break;
    case T_CIRCUIT_CHARGE:
      specialmessage = 11;
      specialmessagetimer = 30;
      circuit_fillrate++;
      noise = 1;
      break;
    case T_CIRCUIT_REFILL:
      specialmessage = 12;
      specialmessagetimer = 30;
      circuit_recoverrate++;
      noise = 1;
      break;
    case T_AGATE_KNIFE:
      specialmessage = 50;
      specialmessagetimer = 150;
      noise = 2;

      player_shield = 30;
      circuit_fillrate = 30;
      circuit_recoverrate = 30;
      player_hp = 6;
      break;
    case T_EVOLUTION_TRAP:
      specialmessage = 25;
      specialmessagetimer = 120;
      SoupUpEnemies();
      break;
    case T_CRYSTALS_500:
    case T_CRYSTALS_1000:
    case T_CRYSTALS_2000:
      if (item >= T_CRYSTALS_1000) basemod *= 2;
      if (item == T_CRYSTALS_2000) basemod *= 2;
      player_gems += rand()%((1 << (explored / 300)) * basemod);
      specialmessage = 20;
      specialmessagetimer = 30;
      noise = 1;
      break;
    default:
      if (item >= T_PSI_KEY_1) {
        specialmessage = 30 + (item - T_PSI_KEY_1);
        specialmessagetimer = 120;
      } else {
        specialmessage = item - T_MAP + 1;
        specialmessagetimer = 30;
      }
      artifacts[item - T_MAP] = 1;

      if (item == T_CURSED_SEAL) Curse();
      else noise = 2;
      break;
  }

  if (noise == 1) SND_Pos("dat/a/tone.wav", 128, 0);
  else if (noise == 2) SND_Pos("dat/a/crystal.wav", 128, 0);
}

void KillPlayer() {
  if (player_dying <= 0 && player_hp > 0) {
    player_hp = 0;
    player_dying = 1;
    SND_Pos("dat/a/playerhurt.wav", 128, 0);
  }
}

void AnnounceDeath() {
  // doesn't do anything for now; this is for AP DeathLink
}

void AnnounceVictory(char isFullVictory) {
  // doesn't do anything for now; this is for AP
}

void CollectItem(t_itemStores store) {
  t_itemTypes item = GetNextItem(store, 1);
  if (item != T_NOTHING) ProcessItem(item);
}

void CollectSpecialItem(t_specialStore itemIndex) {
  t_itemTypes item = GetItemByIndex(IS_SPECIAL, itemIndex, 1);
  if (item != T_NOTHING) ProcessItem(item);
}

void WriteStoreData() {
  SaveStores();
}

void ReadStoreData() {
  LoadStores();
}