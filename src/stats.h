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

#ifndef STATS_H
#define STATS_H

typedef enum statType {
  // integer types
  STAT_BURSTS, // ✔️
  STAT_HITS, // ✔️
  STAT_KILLS, // ✔️
  STAT_RESISTS, // ✔️
  STAT_WHIFFS, // ✔️
  STAT_BULLETS_CANCELLED,

  STAT_HEARTS_GATHERED, // ✔️
  STAT_SHIELD_HITS, // ✔️
  STAT_DAMAGE_TAKEN, // ✔️
  STAT_LIVES_GAINED, // ✔️
  STAT_LIVES_LOST, // ✔️

  STAT_GEMS_COLLECTED, // ✔️
  STAT_GEMS_SPENT, // ✔️
  STAT_GEMS_LOST_ON_DEATH, // ✔️
  STAT_PURCHASES, // ✔️
  STAT_CHESTS, // ✔️
  STAT_ROOM_TRANSITIONS, // ✔️
  STAT_CHECKPOINT_WARPS, // ✔️
  STAT_SAVES, // ✔️

  STAT_TRIES_BOSS1, // ✔️
  STAT_TRIES_BOSS2, // ✔️
  STAT_TRIES_BOSS3, // ✔️
  STAT_TRIES_BOSS4, // ✔️

  STAT_TIMESPENT_BOSS1, // ✔️
  STAT_TIMESPENT_BOSS2, // ✔️
  STAT_TIMESPENT_BOSS3, // ✔️
  STAT_TIMESPENT_BOSS4, // ✔️

  STAT_TIME_BOSS1, // ✔️
  STAT_TIME_BOSS2, // ✔️
  STAT_TIME_BOSS3, // ✔️
  STAT_TIME_BOSS4, // ✔️
  STAT_TIME_KNIFE, // ✔️

  // TODO: per-enemy kill stats
  
  STAT_INTMAX,

  // float types
  STAT_CIRCUIT_VALUE = STAT_INTMAX, // ✔️

  STAT_FLOATMAX
} t_statType;

void init_stats();
void set_int_stat(t_statType stat, int newval);
void set_float_stat(t_statType stat, float newval);
void add_int_stat(t_statType stat, int newval);
void add_float_stat(t_statType stat, float newval);
int get_int_stat(t_statType stat);
float get_float_stat(t_statType stat);

void WriteStatsData();
void ReadStatsData();

#endif