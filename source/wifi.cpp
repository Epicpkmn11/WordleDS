#include "wifi.hpp"
#include "game.hpp"
#include "gfx.hpp"
#include "json.hpp"

#include <dswifi9.h>
#include <nds/interrupts.h>
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
	snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: Wordle DS\r\n\r\n", path, host);

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

		buffer[len] = '\0';
		res.content += buffer;

		// Seems dumb, but waiting for the socket to timeout takes
		// much longer than waiting a few vblanks
		// (and if you don't add a bit of delay, it doesn't always
		// have 255 ready)
		// TODO: something better
		swiWaitForVBlank();

		if(len < 255)
			break;
	}

	shutdown(sock, 0);
	closesocket(sock);

	if(res.content.compare(0, 9, "HTTP/1.1 ") == 0) {
		res.status = atoi(res.content.c_str() + 9); // Get status code number
		res.content = res.content.substr(res.content.find("\r\n\r\n") + 4); // Get content (after the headers)

		// Parse chunked transfer
		std::string parsed;
		while(true) {
			uint len = strtol(res.content.c_str(), NULL, 16);
			if(len == 0)
				break;

			if(len > res.content.size())
				return {0, "Request failed"};

			parsed += res.content.substr(res.content.find("\r\n") + 2, len);
			res.content = res.content.substr(res.content.find("\r\n") + 2 + len + 2);
		}
		res.content = parsed;

		return res;
	} else {
		return {0, "Request failed"};
	}
}

void WiFi::getWords(const char *url) {
	Gfx::showPopup("Connecting to Wi-Fi");
	if(Wifi_InitDefault(WFC_CONNECT)) {
		Gfx::showPopup("Connected to Wi-Fi!");
	} else {
		Gfx::showPopup("Failed to connect to Wi-Fi", 120);
		return;
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
	Wifi_DisconnectAP();

	if(res.status == 200) {
		// We should now have a JSON array of new IDs
		Json json(res.content.c_str(), false);
		if(json.isObject() && json.contains("status") && strcmp(json["status"].get()->valuestring, "ERROR") == 0) {
			Gfx::showPopup(json["message"].get()->valuestring, 120);
		} else if(json.isArray()) {
			// Validate results
			const int choiceCount = game->data().choices().size();
			const int guessCount = -game->data().guesses().size(); // Guess list IDs are negative
			std::vector<int> ids;
			for(const Json id : json) {
				if(id.isNumber()) {
					int val = id.get()->valueint;
					if((val >= 1 && val <= choiceCount) || (val <= -1 && val >= guessCount))
						ids.push_back(id.get()->valueint);
					else
						Gfx::showPopup("Error: Invalid ID", 120);
				} else {
					Gfx::showPopup("Error: Non-numeric ID", 120);
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
