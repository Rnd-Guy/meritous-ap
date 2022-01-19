// itemstore.c
//
// Copyright 2007, 2008 Lancer-X/ASCEAI
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

#include <stdio.h>
#include <stdlib.h>

typedef enum itemTypes {
  T_NOTHING,
  T_REFLECT_SHIELD,
  T_CIRCUIT_CHARGE,
  T_CIRCUIT_REFILL,
  T_MAP,
  T_SHIELD_BOOST,
  T_CRYSTAL_EFFICIENCY,
  T_CIRCUIT_BOOSTER,
  T_METABOLISM,
  T_DODGE_ENHANCER,
  T_ETHEREAL_MONOCLE,
  T_CRYSTAL_GATHERER,
  T_PSI_KEY_1,
  T_PSI_KEY_2,
  T_PSI_KEY_3,
  T_CURSED_SEAL,
  T_AGATE_KNIFE,
  T_EVOLUTION_TRAP,
  T_CRYSTALS_500,
  T_CRYSTALS_1000,
  T_CRYSTALS_2000
} t_itemTypes;

typedef enum itemStores {
  IS_ALPHA,
  IS_BETA,
  IS_GAMMA,
  IS_CHESTS,
  IS_SPECIAL
} t_itemStores;

typedef enum specialStore {
  SS_PSI_KEY_1,
  SS_PSI_KEY_2,
  SS_PSI_KEY_3,
  SS_BOSS_PRIZE_1,
  SS_BOSS_PRIZE_2,
  SS_BOSS_PRIZE_3,
  SS_CURSED_SEAL,
  SS_AGATE_KNIFE
};

typedef enum genFlags {
  GF_INCLUDE_TRAPS
} t_genFlags;

struct itemStore
{
  t_itemTypes items[24];
  char collected[24];
  int length;
  char crystalFallback;
};

struct itemStore** stores = NULL;

void CreateItemStores() {
  if (stores != NULL) free(stores);
  stores = calloc(5, sizeof(struct itemStore));
}

t_itemTypes GetNextItem(t_itemStores store, char collect) {
  if (stores == NULL) return T_NOTHING;

  struct itemStore* thisStore = stores[(int)store];
  if (thisStore == NULL) return T_NOTHING;

  for (int x = 0; x < thisStore->length; x++) {
    if (thisStore->collected[x] == 0) {
      if (collect != 0) thisStore->collected[x] = 1;
      return thisStore->items[x];
    }
  }

  if (thisStore->crystalFallback != 0) {
    int randCrystals = rand() % 32;
    if (randCrystals < 24) return T_CRYSTALS_500;
    if (randCrystals < 31) return T_CRYSTALS_1000;
    return T_CRYSTALS_2000;
  }
  return T_NOTHING;
}
// t_itemTypes GetNextItem(t_itemStores store) {
//   return GetNextItem(store, 1);
// }

char HasNextItem(t_itemStores store) {
  if (stores == NULL) return 0;

  struct itemStore* thisStore = stores[(int)store];
  if (thisStore == NULL) return 0;

  for (int x = 0; x < thisStore->length; x++) {
    if (thisStore->collected[x] != 0) return 1;
  }

  return 0;
}

t_itemTypes GetItemByIndex(t_itemStores store, int index, char collect) {
  if (stores == NULL) return T_NOTHING;

  struct itemStore* thisStore = stores[(int)store];
  if (thisStore == NULL) return T_NOTHING;

  if (index < 0 || index >= thisStore->length) return T_NOTHING;
  if (thisStore->collected[index] != 0) return T_NOTHING;

  if (collect != 0) thisStore->collected[index] = 1;
  return thisStore->items[index];
}
// t_itemTypes GetItemByIndex(t_itemStores store, int index) {
//   return GetItemByIndex(store, index, 1);
// }

char HasItemByIndex(t_itemStores store, int index) {
  if (stores == NULL) return 0;

  struct itemStore *thisStore = stores[(int)store];
  if (thisStore == NULL) return 0;

  if (index < 0 || index >= thisStore->length) return 0;
  if (thisStore->collected[index] != 0) return 0;
  return 1;
}

int EmptySlotsInStore(t_itemStores store) {
  if (stores == NULL) return -1;

  struct itemStore* thisStore = stores[(int)store];
  if (thisStore == NULL) return -1;

  int retval = 0;
  for (int x = 0; x < thisStore->length; x++)
    if (thisStore->items[x] == T_NOTHING) retval++;

  return retval;
}

int GetEmptyPosInStore(t_itemStores store, int index) {
  if (stores == NULL) return -2;

  struct itemStore* thisStore = stores[(int)store];
  if (thisStore == NULL) return -2;

  if (index < 0 || index >= thisStore->length) return -1;

  int emptySlots = 0;
  for (int x = 0; x < thisStore->length; x++) {
    if (thisStore->items[x] == T_NOTHING) {
      if (emptySlots++ == index) return x;
    }
  }

  return -1;
}

void LocalGenerateItemStores(int flags) {
  CreateItemStores();
  int leftToAdd[] = {25, 23, 22, 8, 0, 3};
  int leftToFill[] = {24, 24, 24, 20, 3};

  if (flags & GF_INCLUDE_TRAPS) {
    leftToFill[4] += 3;
    leftToAdd[4] += 3;
  }
  else for (int x = 3; x < 6; x++)
    stores[(int)IS_SPECIAL]->items[x] = T_EVOLUTION_TRAP;

  struct itemStore *specialStore = stores[(int)IS_SPECIAL];

  specialStore->items[(int)SS_CURSED_SEAL] = T_CURSED_SEAL;
  specialStore->items[(int)SS_AGATE_KNIFE] = T_AGATE_KNIFE;

  stores[(int)IS_ALPHA]->length = 24;
  stores[(int)IS_BETA]->length = 24;
  stores[(int)IS_GAMMA]->length = 24;
  stores[(int)IS_CHESTS]->length = 20;
  specialStore->length = 8;

  stores[(int)IS_CHESTS]->crystalFallback = 1;

  // put down the keys first with this logic:
  // - psi keys can be placed at regular psi key locations (is this even randomized?)
  // - psi keys can be placed in the first half of store/chest drops
  // - psi keys *can* be placed in the second half of store/chest drops, but that may result in a longer game
  // - psi keys can NEVER be placed as a boss prize
  // - psi keys can NEVER be placed in the cursed seal or agate knife location
  while (leftToAdd[5] > 0) {
    int randStore = rand() % 13 / 3;
    if (randStore == 4) {
      for (int x = 0; x < 3; x++)
        if (specialStore->items[x] == T_NOTHING) {
          specialStore->items[x] = (t_itemTypes)((int)T_PSI_KEY_1 + --leftToAdd[5]);
          leftToFill[randStore]--;
        }
    }
    else {
      int offset = rand() % (stores[randStore]->length / 2);
      for (int x = 0; x < stores[randStore]->length / 2; x++)
        if (specialStore->items[x+offset] == T_NOTHING) {
          specialStore->items[x+offset] = (t_itemTypes)((int)T_PSI_KEY_1 + --leftToAdd[5]);
          leftToFill[randStore]--;
        }
    }
  }

  int totalLeftToFill = 0;
  for (int x = 0; x < 5; x++) totalLeftToFill += leftToFill[x];

  for (int x = 0; x < 5; x++) {
    t_itemTypes item = x + 1;
    if (x == 4) item = T_EVOLUTION_TRAP;

    while (leftToAdd[x] > 0) {
      if (x == 3)
        item = T_MAP + leftToAdd[x] - 1;

      int randLoc = rand() % totalLeftToFill;
      int store = 0;
      for (int y = 0; y < 5; y++) {
        if (randLoc < leftToFill[y]) break;
        randLoc -= leftToFill[y];
        store++;
      }

      stores[store]->items[GetEmptyPosInStore(store, randLoc)] = item;
      leftToFill[store]--;
      leftToAdd[x]--;
    }
  }

  for (int x = 0; x < 5; x++) {
    while (EmptySlotsInStore(x) > 0) {
      int randCrystals = rand() % 32;
      t_itemTypes putCrystals;

      if (randCrystals < 24) putCrystals = T_CRYSTALS_500;
      else if (randCrystals < 31) putCrystals = T_CRYSTALS_1000;
      else putCrystals = T_CRYSTALS_2000;

      stores[x]->items[GetEmptyPosInStore(x, 0)] = putCrystals;
    }
  }
}