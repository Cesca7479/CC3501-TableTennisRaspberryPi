#include "thingspeak.h"
#include "scorestate.h"
#include <nlohmann/json.hpp>
#include "board.h"



size_t http_callback_upload(void *buffer, size_t sz, size_t nmemb, void *userp) {
    size_t size = sz * nmemb;

    if (size > 0) {
        fwrite(buffer, sz, nmemb, stdout);
        printf("\r\n");
    } else {
        printf("No response\r\n");
    }
    return size;
}

void upload_state(int player_1_score, int player_2_score, int player_1_session_wins, int player_2_session_wins, int player_1_total_wins, int player_2_total_wins) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        printf("Failed to init curl\r\n");
    }

    std::string API_KEY = "8P1QJKWD5PXTRYW9";
    std::string URL = "https://api.thingspeak.com/update?api_key=" + API_KEY + 
        "&field1=" + std::to_string(player_1_score) +
        "&field2=" + std::to_string(player_2_score) +
        "&field3=" + std::to_string(player_1_session_wins) +
        "&field4=" + std::to_string(player_2_session_wins) +
        "&field5=" + std::to_string(player_1_total_wins) +
        "&field6=" + std::to_string(player_2_total_wins);
    printf("URL: %s\r\n", URL.c_str());

  

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_callback_upload);
    curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
}

size_t http_callback_read(void *buffer, size_t sz, size_t nmemb, void *userp) {
    ((std::string*)userp)->append(
        (char*)buffer,
        sz * nmemb);

    return sz * nmemb;
}

void read_total_wins() 
{
    std::string response;

    CURL *curl = curl_easy_init();
    if (!curl) {
        printf("Failed to init curl\r\n");
    }
    // https://api.thingspeak.com/channels/3443442/feeds/last.json?api_key=3C28J6WWZIYXYVPF
    std::string API_KEY = "3C28J6WWZIYXYVPF";
    std::string URL = "https://api.thingspeak.com/channels/3443442/feeds/last.json?api_key=" + API_KEY;
    printf("URL: %s\r\n", URL.c_str());

  

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_callback_read);
    curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);

    auto json = nlohmann::json::parse(response);

    total_wins[PLAYER_1] = std::stoi(json["field5"].get<std::string>());
    total_wins[PLAYER_2] = std::stoi(json["field6"].get<std::string>());
    
    printf("Player1 Wins: %d\r\nPlayer2 Wins: %d\r\n", total_wins[PLAYER_1], total_wins[PLAYER_2]);
}