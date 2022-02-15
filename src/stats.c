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

#include <memory.h>
#include <SDL.h>

#include "stats.h"
#include "save.h"

enum { FLOATCOUNT = STAT_FLOATMAX - STAT_INTMAX };

int istats[STAT_INTMAX] = {0};
float fstats[FLOATCOUNT] = {0};

void init_stats() {
  memset(istats, 0, sizeof(int) * STAT_INTMAX);
  memset(fstats, 0.f, sizeof(float) * FLOATCOUNT);
}

void set_int_stat(t_statType stat, int newval) {
  if (stat < 0 || stat >= STAT_INTMAX) return;
  istats[stat] = newval;
}

void set_float_stat(t_statType stat, float newval) {
  if (stat < STAT_INTMAX || stat >= STAT_FLOATMAX) return;
  fstats[stat - STAT_INTMAX] = newval;
}

void add_int_stat(t_statType stat, int addval) {
  if (stat < 0 || stat >= STAT_INTMAX) return;
  istats[stat] += addval;
}

void add_float_stat(t_statType stat, float addval) {
  if (stat < STAT_INTMAX || stat >= STAT_FLOATMAX) return;
  fstats[stat - STAT_INTMAX] += addval;
}

int get_int_stat(t_statType stat) {
  if (stat < 0 || stat >= STAT_INTMAX) return;
  return istats[stat];
}

float get_float_stat(t_statType stat) {
  if (stat < STAT_INTMAX || stat >= STAT_FLOATMAX) return;
  return fstats[stat - STAT_INTMAX];
}


void WriteStatsData() {
  for (int x = 0; x < STAT_INTMAX; x++) FWInt(istats[x]);
  for (int x = 0; x < FLOATCOUNT; x++) FWFloat(fstats[x]);
}

void ReadStatsData() {
  for (int x = 0; x < STAT_INTMAX; x++) istats[x] = FRInt();
  for (int x = 0; x < FLOATCOUNT; x++) fstats[x] = FRFloat();
}