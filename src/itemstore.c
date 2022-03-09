// itemstore.c
//
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
#include <string.h>
#include <SDL.h>

#include "itemdefs.h"
#include "itemhandler.h"
#include "levelblit.h"
#include "save.h"

int localCostScale[] = {80, 5, 4};

typedef struct itemStore
{
  t_itemTypes items[24];
  char collected[24];
  int length;
  char crystalFallback;
} t_itemStore;

t_itemStore *stores = NULL;

void CreateItemStores() {
  if (stores != NULL) free(stores);
  stores = malloc(sizeof(t_itemStore) * IS_MAX);
  memset(stores, 0, sizeof(t_itemStore) * IS_MAX);
}

void DestroyItemStores() {
  if (stores != NULL) {
    free(stores);
    stores = NULL;
  }
}

t_itemTypes GetNextItem(t_itemStores store, char collect) {
  if (stores == NULL) return T_NOTHING;

  t_itemStore *thisStore = &stores[(int)store];

  for (int x = 0; x < thisStore->length; x++) {
    if (thisStore->collected[x] == 0) {
      if (collect != 0) thisStore->collected[x] = 1;
      return thisStore->items[x];
    }
  }

  if (thisStore->crystalFallback != 0) return MakeCrystals();
  return T_NOTHING;
}
// t_itemTypes GetNextItem(t_itemStores store) {
//   return GetNextItem(store, 1);
// }

char HasNextItem(t_itemStores store) {
  if (stores == NULL) return 0;

  t_itemStore *thisStore = &stores[(int)store];

  for (int x = 0; x < thisStore->length; x++) {
    if (thisStore->collected[x] == 0) return 1;
  }

  return 0;
}

t_itemTypes GetItemByIndex(t_itemStores store, int index, char collect) {
  if (stores == NULL) return T_NOTHING;

  t_itemStore *thisStore = &stores[(int)store];

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

  t_itemStore *thisStore = &stores[(int)store];

  if (index < 0 || index >= thisStore->length) return 0;
  // if (thisStore->collected[index] != 0) return 0;
  // return 1;
  return (thisStore->collected[index] != 0);
}

int EmptySlotsInStore(t_itemStores store) {
  if (stores == NULL) return -1;

  t_itemStore *thisStore = &stores[(int)store];

  int retval = 0;
  for (int x = 0; x < thisStore->length; x++)
    if (thisStore->items[x] == T_NOTHING) retval++;

  return retval;
}

size_t GetNextIndexInStore(t_itemStores store) {
  if (stores == NULL) return -1;

  t_itemStore *thisStore = &stores[(size_t)store];

  for (int x = 0; x < thisStore->length; x++) {
    if (thisStore->collected[x] == 0) return x;
  }

  return -1;
}

int GetLocalUpgradeCost(t_itemStores store, char training) {
  int costFactor = GetNextIndexInStore(store);
  if (costFactor < 0) return 9999999;
  else return (localCostScale[0] - training * (int)((float)localCostScale[0] / 2.f))
               * costFactor + (localCostScale[1] << costFactor)
               * (localCostScale[2] - training * (int)((float)localCostScale[2] / 2.f));
}


int GetEmptyPosInStore(t_itemStores store, int index) {
  if (stores == NULL) return -2;

  t_itemStore *thisStore = &stores[(int)store];

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
  printf("Creating item store containers\n");
  CreateItemStores();
  int leftToAdd[] = {25, 23, 22, 9, 0, 3};
  int leftToFill[] = {24, 24, 24, 24, 3};
  t_itemStore *specialStore = &stores[(int)IS_SPECIAL];

  printf("Determining flags\n");
  if (flags & GF_INCLUDE_TRAPS) {
    leftToFill[4] += 3;
    leftToAdd[4] += 3;
  }
  else {
    printf("Pre-placing evolution traps on bosses\n");
    for (int x = 3; x < 6; x++)
      specialStore->items[x] = T_EVOLUTION_TRAP;
  }

  {
    int totalToAdd = 0, totalToFill = 0;
    for (int x = 0; x < 6; x++) totalToAdd += leftToAdd[x];
    for (int x = 0; x < 5; x++) totalToFill += leftToFill[x];
    printf("%d to add, %d to fill\n", totalToAdd, totalToFill);
  }

  printf("Placing guaranteed items\n");
  specialStore->items[(int)SS_CURSED_SEAL] = T_CURSED_SEAL;
  specialStore->items[(int)SS_AGATE_KNIFE] = T_AGATE_KNIFE;

  printf("Defining stores\n");
  stores[(int)IS_ALPHA].length = 24;
  stores[(int)IS_BETA].length = 24;
  stores[(int)IS_GAMMA].length = 24;
  stores[(int)IS_CHESTS].length = 24;
  specialStore->length = 8;

  stores[(int)IS_CHESTS].crystalFallback = 1;

  // put down the keys first with this logic:
  // - psi keys can be placed at regular psi key locations (is this even randomized?)
  // - psi keys can be placed in the first half of store/chest drops
  // - psi keys *can* be placed in the second half of store/chest drops, but that may result in a longer game
  // - psi keys can NEVER be placed as a boss prize
  // - psi keys can NEVER be placed in the cursed seal or agate knife location
  printf("Placing Psi Keys\n");
  while (leftToAdd[5] > 0) {
    int randStore = rand() % 13 / 3;
    if (randStore >= 5) {
      printf("ERROR: Store %d is out of bounds.", randStore);
      exit(1);
    }
    int offset = (randStore == (int)IS_SPECIAL) ? 0 : rand() % (stores[randStore].length / 2);
    stores[randStore].items[GetEmptyPosInStore(randStore, offset)] = (t_itemTypes)((int)T_PSI_KEY_1 + --leftToAdd[5]);
    leftToFill[randStore]--;
  }

  int totalLeftToFill = 0;
  for (int x = 0; x < 5; x++) totalLeftToFill += leftToFill[x];

  printf("Placing other items\n");
  for (int x = 0; x < IS_MAX; x++) {
    t_itemTypes item = x + 1;
    if (x == 4) item = T_EVOLUTION_TRAP;

    while (leftToAdd[x] > 0) {
      if (x == 3) {
        item = T_MAP + leftToAdd[x] - 1;
      }

      int randLoc = rand() % totalLeftToFill;
      int store = 0;
      for (int y = 0; y < 5; y++) {
        if (randLoc < leftToFill[y]) break;
        randLoc -= leftToFill[y];
        store++;
      }

      int emptyPos = GetEmptyPosInStore(store, randLoc);
      if (emptyPos < 0) {
        printf("ERROR: Tried to get empty position %d of store %d, which does not exist.", randLoc, store);
        exit(1);
      }
      if (emptyPos >= stores[store].length) {
        printf("ERROR: Store %d position %d is out of bounds (length %d).", store, emptyPos, stores[store].length);
        exit(1);
      }
      if (stores[store].items[emptyPos] != T_NOTHING) {
        printf("ERROR: Store %d position %d is not empty and contains %d.", store, emptyPos, stores[store].items[emptyPos]);
        exit(1);
      }
      stores[store].items[emptyPos] = item;
      totalLeftToFill--;
      leftToFill[store]--;
      leftToAdd[x]--;
    }
  }

  printf("Filling with random crystals\n");
  int crystalsPlaced = 0;
  for (int x = 0; x < 5; x++) {
    while (EmptySlotsInStore(x) > 0) {
      stores[x].items[GetEmptyPosInStore(x, 0)] = MakeCrystals();
      crystalsPlaced++;
    }
  }
  
  if (crystalsPlaced != totalLeftToFill) {
    printf("ERROR: mismatch between locations left to fill (%d) and crystal drops placed (%d).", totalLeftToFill, crystalsPlaced);
    exit(1);
  }

  printf("Done.\n");
}

char VerifyItemStores() {
  if (stores == NULL) {
    printf("ERROR: Stores not initialized.");
    return 1;
  }

  int counts[T_MAX] = {0};
  int correctCounts[T_MAX - 3] = {0, 25, 23, 22, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3};

  for (int x = 0; x < IS_MAX; x++) {
    t_itemStore *thisStore = &stores[x];
    for (int y = 0; y < thisStore->length; y++)
      counts[thisStore->items[y]]++;
  }

  char retval = 0;
  for (int x = 0; x < T_MAX - 3; x++)
    if (counts[x] != correctCounts[x]) {
      printf("ERROR: Item %d has %d instances (expected %d)", x, counts[x], correctCounts[x]);
      retval = 1;
    }

  return retval;
}

void SaveStores() {
  if (stores == NULL) return;
  for (int x = 0; x < IS_MAX; x++) {
    t_itemStore *thisStore = &stores[x];
    FWInt(thisStore->length);
    for (int y = 0; y < thisStore->length; y++) {
      FWInt(thisStore->items[y]);
      FWChar(thisStore->collected[y]);
    }
    FWChar(thisStore->crystalFallback);
    SavingScreen(4, (float)(x + 1) / IS_MAX);
  }
}

void LoadStores() {
  CreateItemStores();
  for (int x = 0; x < IS_MAX; x++) {
    t_itemStore *thisStore = &stores[x];
    thisStore->length = FRInt();
    for (int y = 0; y < thisStore->length; y++) {
      thisStore->items[y] = FRInt();
      thisStore->collected[y] = FRChar();
    }
    thisStore->crystalFallback = FRChar();
    LoadingScreen(5, (float)(x + 1) / IS_MAX);
  }
}