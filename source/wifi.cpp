#include "wifi.hpp"
#include "game.hpp"
#include "gfx.hpp"
#include "json.hpp"

#include <dswifi9.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <vector>

struct HttpResponse {
	int status;
	std::string content;
};

HttpResponse httpGet(const char *host, const char *path) {
	char request[256];
	snprintf(request, sizeof(request), "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: Wordle DS\r\n\r\n", path, host);

	hostent *hostEnt = gethostbyname(host);
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in sain;
	sain.sin_family = AF_INET;
	sain.sin_port = htons(80);
	sain.sin_addr.s_addr = *(u32 *)(hostEnt->h_addr_list[0]);
	connect(sock, (sockaddr *)&sain, sizeof(sain));

	send(sock, request, strlen(request), 0);

	HttpResponse res;
	while(true) {
		char buffer[256];
		int len = recv(sock, buffer, 255, 0);
		if(len > 0) {
			buffer[len] = '\0';
			res.content += buffer;

			if(len < 255)
				break;
		}
	}

	shutdown(sock, 0);
	closesocket(sock);

	if(res.content.compare(0, 9, "HTTP/1.1 ") == 0) {
		res.status = atoi(res.content.c_str() + 9); // Get status code number
		res.content = res.content.substr(res.content.find("\r\n\r\n") + 4); // Get content (after the headers)
		return res;
	} else {
		return {0, "Request failed"};
	}

}

void WiFi::getWords(const char *url) {
	Gfx::showPopup("Connecting to Wi-Fi");
	if(!Wifi_CheckInit()) {
		if(Wifi_InitDefault(WFC_CONNECT)) {
			Gfx::showPopup("Connected to Wi-Fi!");
		} else {
			Gfx::showPopup("Failed to connect to Wi-Fi", 120);
			return;
		}
	} else {
		Wifi_EnableWifi();
	}

	// Parse out the host and request path
	char host[256], path[256];
	if(memcmp(url, "http://", 7) != 0)
		return; // Invalid URL, must be HTTP not HTTPS

	// The host is from after the 'http://' until the '/path/to/file.json'
	snprintf(host, sizeof(host), "%s", url + 7);
	char *slash = strchr(host, '/');
	if(!slash)
		return;
	*slash = '\0';

	// The path should include a date since we only want new IDs, so strftime that in
	time_t epoch = (game->data().firstDay() + game->data().choiceOrder().size()) * 24 * 60 * 60;
	tm *time = localtime(&epoch);
	strftime(path, sizeof(path), url + 7 + (slash - host), time);

	// Try get the page
	Gfx::showPopup("Updating words...");
	HttpResponse res = httpGet(host, path);
	Wifi_DisableWifi();

	if(res.status == 200) {
		// We should now have a JSON array of new IDs
		Json json(res.content.c_str(), false);
		if(json.isObject() && json.contains("status") && strcmp(json["status"].get()->valuestring, "ERROR") == 0) {
			Gfx::showPopup(json["message"].get()->valuestring, 120);
		} else if(json.isArray()) {
			// Validate results
			int choiceCount = game->data().choices().size();
			std::vector<int> ids;
			for(const Json id : json) {
				if(id.isNumber() && id.get()->valueint >= 1 && id.get()->valueint <= choiceCount) {
					ids.push_back(id.get()->valueint);
				} else {
					Gfx::showPopup("Error loading IDs", 120);
					return;
				}
			}

			// Save to mod.json
			Gfx::showPopup("Saving...");
			game->data().appendChoiceOrder(ids);

			Gfx::showPopup("Update successful!", 120);
		} else {
			Gfx::showPopup("Invalid JSON!", 120);
		}
	} else {
		Gfx::showPopup("Update failed!!", 120);
	}
}
