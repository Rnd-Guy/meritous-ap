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
  int costFactor;
  bool crystalFallback;

public:
  ItemStore(int length, bool crystalFallbackIn) {
    for (int x = 0; x < length; x++) checks.push_back(false);
    costFactor = 0;
    crystalFallback = crystalFallbackIn;
  }

  void MarkCollected(int index) {
    if (index < 0 || index >= checks.size()) return;
    checks[index] = true;
  }

  int BuyNextItem() {
    for (int x = 0; x < checks.size(); x++) {
      if (!checks[x]) {
        costFactor++;
        checks[x] = true;
        return x;
      }
    }
    return -1;
  }

  int GetCostFactor() {
    return costFactor;
  }

  bool HasCrystalFallback() {
    return crystalFallback;
  }

  bool operator[](int index) {
    if (index < 0 || index >= checks.size()) return false;
    return checks[index];
  }
};

#endif