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

#ifndef ITEMDEFS_H
#define ITEMDEFS_H

typedef enum artifacts {
  AF_MAP,
  AF_SHIELD_BOOST,
  AF_CRYSTAL_EFFICIENCY,
  AF_CIRCUIT_BOOSTER,
  AF_METABOLISM,
  AF_DODGE_ENHANCER,
  AF_ETHEREAL_MONOCLE,
  AF_CRYSTAL_GATHERER,
  AF_PORTABLE_COMPASS,
  AF_PSI_KEY_1,
  AF_PSI_KEY_2,
  AF_PSI_KEY_3,
  AF_CURSED_SEAL,

  AF_MAXX,
  AF_MAXX_NOKEYS = AF_PSI_KEY_1
} t_artifacts;

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
  T_PORTABLE_COMPASS,
  T_PSI_KEY_1,
  T_PSI_KEY_2,
  T_PSI_KEY_3,
  T_CURSED_SEAL,
  T_AGATE_KNIFE,
  T_EVOLUTION_TRAP,
  T_CRYSTALS_500,
  T_CRYSTALS_1000,
  T_CRYSTALS_2000,
  T_1UP,

  T_MAX
} t_itemTypes;

typedef enum itemStores {
  IS_ALPHA,
  IS_BETA,
  IS_GAMMA,
  IS_CHESTS,
  IS_SPECIAL,

  IS_MAX
} t_itemStores;

typedef enum specialStore {
  SS_PSI_KEY_1,
  SS_PSI_KEY_2,
  SS_PSI_KEY_3,
  SS_BOSS_PRIZE_1,
  SS_BOSS_PRIZE_2,
  SS_BOSS_PRIZE_3,
  SS_CURSED_SEAL,
  SS_AGATE_KNIFE,

  SS_MAX
} t_specialStore;

typedef enum genFlags {
  GF_INCLUDE_TRAPS = 0x01
} t_genFlags;

typedef enum apSignal {
  APSIG_NONE,
  APSIG_RELEASE,
  APSIG_COLLECT
} t_apSignal;

#endif