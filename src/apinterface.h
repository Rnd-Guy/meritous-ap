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

#ifndef APINTERFACE_H
#define APINTERFACE_H

// #ifdef __cplusplus
// #define EXTERNC extern "C"
// #else
// #define EXTERNC
// #endif

char ReadAPSettings();

void CreateAPStores();
void DestroyAPStores();

int GetAPCostFactor(t_itemStores store);
void CollectAPItem(t_itemStores store);
void CollectAPSpecialItem(t_specialStore index);

void ConnectAP();
void DisconnectAP();

void WriteAPState();
void ReadAPState();

void UpdateSavingScreen(float value);
void UpdateLoadingScreen(float value);

void SendAPSignalMsg(t_apSignal signal);
void SendCheck(int locationId);
void PollServer();

char isDeathLink();
void SendDeathLink();
void AnnounceAPVictory(char isFullVictory);

#endif