// itemstore.h
//
// Copyright 2022 FelicitusNeko
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

#ifndef ITEMSTORE_H
#define ITEMSTORE_H

#include "itemdefs.h"

void CreateItemStores();
void DestroyItemStores();

t_itemTypes GetNextItem(t_itemStores store, char collect);
// enum itemTypes GetNextItem(t_itemStores store);
char HasNextItem(t_itemStores store);

t_itemTypes GetItemByIndex(t_itemStores store, int index, char collect);
// enum itemTypes GetItemByIndex(t_itemStores store, int index);
char HasItemByIndex(t_itemStores store, int index);

int GetNextIndexInStore(t_itemStores store);

void LocalGenerateItemStores(int flags);

char VerifyItemStores();

void SaveStores();
void LoadStores();

#endif