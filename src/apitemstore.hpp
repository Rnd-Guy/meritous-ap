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

#ifndef APITEMSTORE_H
#define APITEMSTORE_H

#include <vector>

class ItemStore {
private:
  std::vector<bool> checks;
  std::vector<std::optional<uint64_t>> items;
  int costFactor;
  bool crystalFallback;

public:
  ItemStore(int length, bool crystalFallbackIn) {
    for (int x = 0; x < length; x++) {
      checks.push_back(false);
      items.push_back(std::nullopt);
    }
    costFactor = 0;
    crystalFallback = crystalFallbackIn;
  }

  void MarkCollected(int index) {
    if (index < 0 || index >= (int)checks.size()) return;
    checks[index] = true;
  }

  size_t GetNextItemIndex() {
    for (size_t x = 0; x < checks.size(); x++)
      if (!checks[x]) return x;
    return -1;
  }

  size_t BuyNextItem() {
    auto nextItem = GetNextItemIndex();
    if (nextItem != (size_t)-1) {
      costFactor++;
      checks[nextItem] = true;
    }
    return nextItem;
  }

  bool StoreItem(size_t index, uint64_t item) {
    if (index >= items.size()) return false;
    items[index] = item;
    return true;
  }

  bool HasItemStored(size_t index) {
    if (index >= items.size()) return false;
    return items[index].has_value();
  }

  uint64_t GetStoredItem(size_t index) {
    if (index >= items.size()) return -1;
    return items[index].value_or(-1);
  }

  void SetCostFactor(int newFactor) {
    if (costFactor == 0) costFactor = newFactor;
  }

  size_t GetLength() {
    return checks.size();
  }

  int GetCostFactor() {
    return costFactor;
  }

  bool HasCrystalFallback() {
    return crystalFallback;
  }

  bool operator[](size_t index) {
    if (index >= checks.size()) return false;
    return checks[index];
  }
};

#endif