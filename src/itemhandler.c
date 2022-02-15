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
#include "stats.h"

const char *itemNames[] = {
  "Nothing",
  "Reflect Shield upgrade",
  "Circuit Charge upgrade",
  "Circuit Refill upgrade",
  "Map",
  "Shield Boost",
  "Crystal Efficiency",
  "Circuit Booster",
  "Metabolism",
  "Dodge Enhancer",
  "Ethereal Monocle",
  "Crystal Gatherer",
  "", // get these from artifact_names
  "",
  "",
  "",
  "Agate Knife",
  "Evolution Trap",
  "Some crystals",
  "More crystals",
  "Many crystals",
  "Extra Life"
};

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
  // TODO: Make these weights configurable
  int randCrystals = rand() % 32;
  if (randCrystals < 24) return T_CRYSTALS_500;
  if (randCrystals < 31) return T_CRYSTALS_1000;
  return T_CRYSTALS_2000;
}

char HasItem(t_itemTypes item) {
  switch (item) {
    case T_NOTHING:
    case T_CRYSTALS_500:
    case T_CRYSTALS_1000:
    case T_CRYSTALS_2000:
      return 0;
    case T_REFLECT_SHIELD:
      return player_shield >= 25;
    case T_CIRCUIT_CHARGE:
      return circuit_fillrate >= 25;
    case T_CIRCUIT_REFILL:
      return circuit_recoverrate >= 25;
    case T_AGATE_KNIFE:
      return player_shield == 30;
    case T_EVOLUTION_TRAP:
      return enemy_evolutions >= 3;
    default:
      return artifacts[item - T_MAP];
  }
}

void ProcessItem(t_itemTypes item, char *source, char isForfeit) {
  char noise = 0;
  int basemod = 500;

  if (HasItem(item)) item = MakeCrystals();

  // Process the item effect
  switch (item) {
    case T_NOTHING:
      break;
    case T_REFLECT_SHIELD:
      player_shield++;
      break;
    case T_CIRCUIT_CHARGE:
      circuit_fillrate++;
      break;
    case T_CIRCUIT_REFILL:
      circuit_recoverrate++;
      break;
    case T_AGATE_KNIFE:
      add_int_stat(STAT_TIME_KNIFE, expired_ms);
      player_shield = 30;
      circuit_fillrate = 30;
      circuit_recoverrate = 30;
      player_hp = 6;
      break;
    case T_EVOLUTION_TRAP:
      SoupUpEnemies();
      break;
    case T_CRYSTALS_500:
    case T_CRYSTALS_1000:
    case T_CRYSTALS_2000:
      if (item >= T_CRYSTALS_1000) basemod *= 2;
      if (item == T_CRYSTALS_2000) basemod *= 2;
      int collect = rand()%((1 << (explored / 300)) * basemod);
      add_int_stat(STAT_GEMS_COLLECTED, collect);
      player_gems += collect;
      break;
    case T_1UP:
      add_int_stat(STAT_LIVES_GAINED, 1);
      player_lives++;
      player_hp = 3 + (player_shield == 30)*3;
      break;
    default:
      artifacts[item - T_MAP] = 1;
      if (item == T_CURSED_SEAL) Curse();
      break;
  }

  // Show the appropriate message
  if (isForfeit) {
    char kelka = item >= T_PSI_KEY_1 && item <= T_CURSED_SEAL;
    PostMessage(71, 30, 2, source, kelka ? artifact_names[item - T_PSI_KEY_1] : itemNames[item]);
  } else switch (item) {
    case T_NOTHING:
      PostMessage(60, 30, 1, source);
      break;
    case T_REFLECT_SHIELD:
      PostMessage(10, 30, 1, source);
      noise = 1;
      break;
    case T_CIRCUIT_CHARGE:
      PostMessage(11, 30, 1, source);
      noise = 1;
      break;
    case T_CIRCUIT_REFILL:
      PostMessage(12, 30, 1, source);
      noise = 1;
      break;
    case T_AGATE_KNIFE:
      PostMessage(50, 150, 0);
      noise = 2;
      break;
    case T_EVOLUTION_TRAP:
      PostMessage(25, 120, 1, source);
      break;
    case T_CRYSTALS_500:
    case T_CRYSTALS_1000:
    case T_CRYSTALS_2000:
      PostMessage(20, 30, 1, source);
      noise = 1;
      break;
    case T_1UP:
      PostMessage(61, 30, 1, source);
      noise = 3;
      break;
    case T_CURSED_SEAL:
      PostMessage(33, 120, 0);
      break;
    default:
      if (item >= T_PSI_KEY_1) PostMessage(34, 120, 2, source, artifact_names[item - T_PSI_KEY_1]);
      else PostMessage(item - T_MAP + 1, 30, 1, source);
      noise = 2;
      break;
  }

  if (noise == 1) SND_Pos("dat/a/tone.wav", 128, 0);
  else if (noise == 2) SND_Pos("dat/a/crystal.wav", 128, 0);
  else if (noise == 3) SND_Pos("dat/a/crystal2.wav", 100, 0);
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
  char source[12] = {0};

  switch (store) {
    case IS_ALPHA: sprintf(source, "Alpha store"); break;
    case IS_BETA: sprintf(source, "Beta store"); break;
    case IS_GAMMA: sprintf(source, "Gamma store"); break;
    case IS_CHESTS: sprintf(source, "the chest"); break;
    default: sprintf(source, "nowhere"); break;
  }
  ProcessItem(item, source, 0);
}

void CollectSpecialItem(t_specialStore itemIndex) {
  t_itemTypes item = GetItemByIndex(IS_SPECIAL, itemIndex, 1);
  char source[16] = {0};

  switch (itemIndex) {
    case SS_PSI_KEY_1: case SS_PSI_KEY_2: case SS_PSI_KEY_3:
      sprintf(source, "the pedestal"); break;

    case SS_BOSS_PRIZE_1: sprintf(source, "Meridian's soul"); break;
    case SS_BOSS_PRIZE_2: sprintf(source, "Ataraxia's soul"); break;
    case SS_BOSS_PRIZE_3: sprintf(source, "Merodach's soul"); break;

    default: sprintf(source, "somewhere");
  }

  ProcessItem(item, source, 0);
}

char IsArchipelago() {
  // TODO: when we implement the AP client, this will return whether the current game is AP or local rando
  return 1;
}

void ForceCollectItem(char *player, char *itemName, char *waswere) {
  PostMessage(72, 30, 2, itemName, waswere, player);
}

void WriteStoreData() {
  SaveStores();
}

void ReadStoreData() {
  LoadStores();
}