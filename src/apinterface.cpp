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

#include <fstream>
#include <map>

#include <SDL.h>
#include <zlib.h>

#include "submodules/apclientpp/apclient.hpp"
#include "uuid.h"
#include "apitemstore.hpp"

extern "C" {
  #include "itemhandler.h"
  #include "apinterface.h"
  #include "save.h"
}

#define HIDWORD(n) (int32_t)((n >> 8) & 0xffffffff)
#define LODWORD(n) (int32_t)(n & 0xffffffff)

#define GAME_NAME "Meritous"
#define DATAPACKAGE_CACHE "datapackage.json"
#define AP_OFFSET 593000

using nlohmann::json;

APClient *ap = NULL;
bool ap_sync_queued = false;
bool ap_connect_sent = false;
double deathtime = -1;

std::vector<ItemStore*> apStores;
std::map<int, std::set<int64_t>> recvCache;

std::string server;
std::string slotname;
std::string password;

int goal = 0;
bool deathlink = false;

const char *storeNames[] = {
  "Alpha cache",
  "Beta cache",
  "Gamma cache",
  "the chest",
  "somewhere special"
};

const char *specialStoreNames[] = {
  "the pedestal",
  "the pedestal",
  "the pedestal",
  "Meridian's soul",
  "Ataraxia's soul",
  "Merodach's soul",
  "this remote location",
  "this reclusive location"
};

void DestroyAPStores();

int64_t FRInt64() {
  return (int64_t)FRInt() << 8 | (int64_t)FRInt();
}
void FWInt64(int64_t val) {
  FWInt(HIDWORD(val));
  FWInt(LODWORD(val));
}

const char *IDToLocation(int locId) {
  if (locId >= AP_OFFSET) locId -= AP_OFFSET;
  if (locId < 96) return storeNames[locId / 24];
  else return specialStoreNames[locId - 96];
}

bool isEqual(double a, double b)
{
  return fabs(a - b) < std::numeric_limits<double>::epsilon() * fmax(fabs(a), fabs(b));
}

char ReadAPSettings() {
  std::ifstream i("meritous-ap.json");
  if (i.is_open()) {
    std::ostringstream fullserver;
    json settings;
    i >> settings;
    i.close();

    bool apEnabled = settings["ap-enable"].get<bool>();
    if (!apEnabled) return 0;

    fullserver
      << (settings["server"].is_string() ? settings["server"].get<std::string>() : (std::string)"localhost")
      << ":"
      << (settings["port"].is_number() ? settings["port"].get<int>() : 38281);
    server = fullserver.str();

    password = settings["password"].is_string() ? settings["password"].get<std::string>() : (std::string)"";
    slotname = settings["slotname"].get<std::string>();

    return 1;
  } else return 0;
}

void CreateAPStores()
{
  if (apStores.size() > 0) DestroyAPStores();
  apStores.push_back(new ItemStore(24, false));
  apStores.push_back(new ItemStore(24, false));
  apStores.push_back(new ItemStore(24, false));
  apStores.push_back(new ItemStore(24, true));
  apStores.push_back(new ItemStore(8, false));
  recvCache.clear();
}

void DestroyAPStores()
{
  while (apStores.size()) {
    ItemStore *delStore = apStores.back();
    apStores.pop_back();
    delete delStore;
  }
}

int GetAPCostFactor(t_itemStores store)
{
  if (!apStores.size() || store < IS_ALPHA || store > IS_GAMMA) return -1;
  return apStores[store]->GetCostFactor();
}

void CollectAPItem(t_itemStores store)
{
  if (!ap || !apStores.size()) return;
  int next = apStores[store]->BuyNextItem();
  if (next >= 0) {
    std::list<int64_t> check;
    check.push_back((store * 24) + next + AP_OFFSET);
    ap->LocationScouts(check);
    ap->LocationChecks(check);
  } else if (apStores[store]->HasCrystalFallback()) {
    ReceiveItem(MakeCrystals(), storeNames[store]);
  }
}

void CollectAPSpecialItem(t_specialStore index)
{
  if (!ap || !apStores.size()) return;
  apStores[IS_SPECIAL]->MarkCollected(index);

  std::list<int64_t> check;
  check.push_back(index + 96 + AP_OFFSET);
  ap->LocationScouts(check);
  ap->LocationChecks(check);
}

void ConnectAP()
{
  std::string uri = server;
  // read or generate uuid, required by AP
  std::string uuid = get_uuid();

  if (ap) delete ap;
  ap = nullptr;
  if (!uri.empty() && uri.find("ws://") != 0 && uri.find("wss://") != 0) uri = "ws://"+uri;

  printf("Connecting to AP, server %s\n", uri.c_str());
  if (uri.empty()) ap = new APClient(uuid, GAME_NAME);
  else ap = new APClient(uuid, GAME_NAME, uri);
  SetAPStatus("Connecting", 1);

  // clear game's cache. read below on socket_connected_handler
  //if (game) game->clear_cache();

  // load DataPackage cache
  FILE* f = fopen(DATAPACKAGE_CACHE, "rb");
  if (f) {
    char* buf = nullptr;
    size_t len = (size_t)0;
    if ((0 == fseek(f, 0, SEEK_END)) &&
        ((len = ftell(f)) > 0) &&
        ((buf = (char*)malloc(len+1))) &&
        (0 == fseek(f, 0, SEEK_SET)) &&
        (len == fread(buf, 1, len, f)))
    {
      buf[len] = 0;
      try {
        ap->set_data_package(json::parse(buf));
      } catch (std::exception&) { /* ignore */ }
    }
    free(buf);
    fclose(f);
  }

  // set state and callbacks
  ap_sync_queued = false;
  ap->set_socket_connected_handler([](){
    // TODO: what to do when we've connected to the server
    printf("Connected, authenticating\n");
    SetAPStatus("Authenticating", 1);
  });
  ap->set_socket_disconnected_handler([](){
    // TODO: what to do when we've disconnected from the server
    printf("Disconnected\n");
    SetAPStatus("Disconnected", 1);
  });
  ap->set_room_info_handler([](){
    printf("Room info received\n");
    std::list<std::string> tags;
    if (deathlink) tags.push_back("DeathLink");
    ap->ConnectSlot(slotname, password, 0b111, tags, {0,2,5});
    ap_connect_sent = true; // TODO: move to APClient::State ?
  });
  ap->set_slot_connected_handler([](const json& data){
    deathlink = data["death_link"].is_boolean() ? data["death_link"].get<bool>() : false;
    goal = data["goal"].is_number_integer() ? data["goal"].get<int>() : 0;

    if (deathlink) ap->ConnectUpdate(false, 0b111, true, {"DeathLink"});
    ap->StatusUpdate(APClient::ClientStatus::PLAYING);
    printf("Connected and ready to go as %s\n", ap->get_player_alias(ap->get_player_number()).c_str());
    SetAPStatus("Connected", 0);
  });
  ap->set_slot_disconnected_handler([](){
    printf("Disconnected\n");
    SetAPStatus("Disconnected", 1);
    ap_connect_sent = false;
  });
  ap->set_slot_refused_handler([](const std::list<std::string>& errors){
    ap_connect_sent = false;
    if (std::find(errors.begin(), errors.end(), "InvalidSlot") != errors.end()) {
      //bad_slot(game?game->get_slot():"");
    } else {
      printf("AP: Connection refused:");
      for (const auto& error: errors) printf(" %s", error.c_str());
      printf("\n");
      SetAPStatus(errors.front().c_str(), 1);
    }
  });
  ap->set_items_received_handler([](const std::list<APClient::NetworkItem>& items) {
    if (!ap->is_data_package_valid()) {
      // NOTE: this should not happen since we ask for data package before connecting
      if (!ap_sync_queued) ap->Sync();
      ap_sync_queued = true;
      return;
    }
    for (const auto& item: items) {
      if (item.player > 0) {
        if (recvCache.find(item.player) == recvCache.end())
          recvCache.insert(std::pair<int, std::set<int64_t>>(item.player, std::set<int64_t>()));
        if (recvCache[item.player].find(item.location) != recvCache[item.player].end()) return;
        recvCache[item.player].insert(item.location);
      }

      auto itemname = ap->get_item_name(item.item);
      auto sender = item.player ? (ap->get_player_alias(item.player) + "'s world") : "out of nowhere";
      auto location = ap->get_location_name(item.location);

      if (item.player == ap->get_player_number()) {
        location = IDToLocation(item.location);
      }
      
      printf("  #%d: %s (%" PRId64 ") from %s - %s\n",
              item.index, itemname.c_str(), item.item,
              sender.c_str(), location.c_str());
      ReceiveItem((t_itemTypes)(item.item - AP_OFFSET),
        item.player == ap->get_player_number()
        ? location.c_str()
        : sender.c_str());
    }
  });
  ap->set_location_info_handler([](const std::list<APClient::NetworkItem> &items) {
    auto me = ap->get_player_number();
    for (auto item: items) {
      if (item.player != me) {
        auto itemname = ap->get_item_name(item.item);
        auto recipient = ap->get_player_alias(item.player);

        ReportSentItem(IDToLocation(item.location), recipient.c_str(), itemname.c_str());
      }
    }
  });
  ap->set_data_package_changed_handler([](const json& data) {
    // NOTE: what to do when the data package changes (probably doesn't need to change this code)
    FILE* f = fopen(DATAPACKAGE_CACHE, "wb");
    if (f) {
      std::string s = data.dump();
      fwrite(s.c_str(), 1, s.length(), f);
      fclose(f);
    }
  });
  ap->set_print_handler([](const std::string& msg) {
    printf("%s\n", msg.c_str());
  });
  ap->set_print_json_handler([](const std::list<APClient::TextNode>& msg) {
    printf("%s\n", ap->render_json(msg, APClient::RenderFormat::ANSI).c_str());
  });
  ap->set_bounced_handler([](const json& cmd) {
    if (deathlink) {
      auto tagsIt = cmd.find("tags");
      auto dataIt = cmd.find("data");
      if (tagsIt != cmd.end() && tagsIt->is_array()
          && std::find(tagsIt->begin(), tagsIt->end(), "DeathLink") != tagsIt->end())
      {
        if (dataIt != cmd.end() && dataIt->is_object()) {
          json data = *dataIt;
          if (data["time"].is_number() && isEqual(data["time"].get<double>(), deathtime)) {
            deathtime = -1;
          } else {
            KillPlayer(data["source"].is_string() ? data["source"].get<std::string>().c_str() : "???");
          }
        } else {
          printf("Bad deathlink packet!\n");
        }
      }
    }
  });
}

void DisconnectAP()
{
  if (ap) {
    delete ap;
    ap = NULL;
  }
}

void WriteAPState()
{
  int progress = 0;
  FWInt(recvCache.size());
  for (const auto& [key, locList]: recvCache) {
    FWInt(key);
    FWInt(locList.size());
    for (const auto location: locList) FWInt64(location);
  }
  for (const auto store: apStores) {
    FWInt(store->GetCostFactor());
    for (size_t x = 0; x < store->GetLength(); x++) {
      FWChar((unsigned char)(*store)[x]);
    }
    UpdateSavingScreen((float)(++progress + 1) / (IS_MAX + 1));
  }
}

void ReadAPState()
{
  CreateAPStores();
  recvCache.clear();

  int progress = 0;
  for (int x = FRInt(); x > 0; x--) {
    auto player = FRInt();
    recvCache.insert(std::pair<int, std::set<int64_t>>(player, std::set<int64_t>()));
    for (int y = FRInt(); y > 0; y--) recvCache[player].insert(FRInt64());
  }
  for (auto store: apStores) {
    store->SetCostFactor(FRInt());
    for (size_t x = 0; x < store->GetLength(); x++) {
      auto collected = FRChar();
      if (collected) store->MarkCollected(x);
    }
    UpdateLoadingScreen((float)(++progress + 1) / (IS_MAX + 1));
  }
}

uint32_t RetrieveAPSeed()
{
  if (ap && ap->get_state() >= APClient::State::ROOM_INFO) {
    auto crc = crc32(0L, NULL, 0);
    crc = crc32(crc, (const Bytef*)ap->get_seed().c_str(), ap->get_seed().length());
    return HIDWORD(crc) ^ LODWORD(crc);
  }
  else return time(NULL);
}

void SendAPSignalMsg(t_apSignal signal)
{
  if (!ap) return;
  switch (signal) {
    case APSIG_COLLECT: ap->Say("!collect"); break;
    case APSIG_FORFEIT: ap->Say("!forfeit"); break;
    default: break;
  }
}

void PollServer()
{
  if (ap) ap->poll();
}

char isDeathLink()
{
  return (char)deathlink;
}

void SendDeathLink()
{
  if (!ap || !deathlink) return;
  deathtime = ap->get_server_time();
  json data{
    {"time", deathtime},
    {"cause", "PSI of defeat"}, // TODO: see about adding more granular reasons
    {"source", ap->get_slot()}
  };
  ap->Bounce(data, {}, {}, {"DeathLink"});
}

char AnnounceAPVictory(char winState)
{
  char retval = 0;

  if (!ap || ap->get_state() != APClient::State::SLOT_CONNECTED) return 0;
  if (winState >= goal) {
    retval = 1;
    ap->StatusUpdate(APClient::ClientStatus::GOAL);
  }

  return retval;
}