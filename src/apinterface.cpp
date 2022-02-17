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

#include "submodules/apclientpp/apclient.hpp"
#include "uuid.h"

extern "C" {
  #include "itemhandler.h"
}

#define GAME_NAME "Meritous"
#define DATAPACKAGE_CACHE "datapackage.json"

using nlohmann::json;

APClient *ap = NULL;
bool ap_sync_queued = false;
bool deathlink = false;
bool ap_connect_sent = false;
double deathtime = -1;

bool isEqual(double a, double b)
{
  return fabs(a - b) < std::numeric_limits<double>::epsilon() * fmax(fabs(a), fabs(b));
}

void connect_ap(const char *c_uri)
{
  std::string uri = c_uri;
  // read or generate uuid, required by AP
  std::string uuid = get_uuid();

  if (ap) delete ap;
  ap = nullptr;
  if (!uri.empty() && uri.find("ws://") != 0 && uri.find("wss://") != 0) uri = "ws://"+uri;

  printf("Connecting to AP...\n");
  if (uri.empty()) ap = new APClient(uuid, GAME_NAME);
  else ap = new APClient(uuid, GAME_NAME, uri);

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
          } catch (std::exception) { /* ignore */ }
      }
      free(buf);
      fclose(f);
  }

  // set state and callbacks
  ap_sync_queued = false;
  ap->set_socket_connected_handler([](){
    // TODO: what to do when we've connected to the server
  });
  ap->set_socket_disconnected_handler([](){
    // TODO: what to do when we've disconnected from the server
  });
  ap->set_room_info_handler([](){
    // TODO: not completely sure what to do here
    // compare seeds and error out if it's the wrong one, and then (try to) connect with games's slot
    // if (!game || game->get_seed().empty() || game->get_slot().empty())
    //     printf("Waiting for game ...\n");
    // else if (strncmp(game->get_seed().c_str(), ap->get_seed().c_str(), GAME::MAX_SEED_LENGTH) != 0)
    //     bad_seed(ap->get_seed(), game->get_seed());
    // else {
        std::list<std::string> tags;
        if (deathlink) tags.push_back("DeathLink");
        // ap->ConnectSlot(game->get_slot(), password, game->get_items_handling(), tags);
        ap_connect_sent = true; // TODO: move to APClient::State ?
    // }
  });
  ap->set_slot_connected_handler([](){
    // TODO: what to do when we've connected to our slot
  });
  ap->set_slot_disconnected_handler([](){
    // TODO: what to do when we've disconnected from our slot
    ap_connect_sent = false;
  });
  ap->set_slot_refused_handler([](const std::list<std::string>& errors){
    // TODO: what to do when connecting to our slot failed
    ap_connect_sent = false;
    if (std::find(errors.begin(), errors.end(), "InvalidSlot") != errors.end()) {
      //bad_slot(game?game->get_slot():"");
    } else {
      printf("AP: Connection refused:");
      for (const auto& error: errors) printf(" %s", error.c_str());
      printf("\n");
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
      // TODO: what to do when we receive an item
      std::string itemname = ap->get_item_name(item.item);
      std::string sender = ap->get_player_alias(item.player);
      std::string location = ap->get_location_name(item.location);
      printf("  #%d: %s (%" PRId64 ") from %s - %s\n",
              item.index, itemname.c_str(), item.item,
              sender.c_str(), location.c_str());
      //game->send_item(item.index, item.item, sender, location);
    }
  });
  ap->set_data_package_changed_handler([](const json& data) {
    // TODO: what to do when the data package changes (probably doesn't need to change this code)
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
      // TODO: what to do when DeathLink is received
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

