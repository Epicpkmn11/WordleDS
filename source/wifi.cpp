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
		res.status = atoi(res.content.c_str() + 9);
		res.content = res.content.substr(res.content.find("\r\n\r\n") + 4);
		return res;
	} else {
		return {0, "Request failed"};
	}

}

void getWords(int day) {
	game->drawBgBottom("Connecting to Wi-Fi", 0);
	Gfx::fadeIn(FADE_SLOW, FADE_TOP | FADE_BOTTOM);;
	if(!Wifi_CheckInit()) {
		if(Wifi_InitDefault(WFC_CONNECT)) {
			game->drawBgBottom("Connected to Wi-Fi!", 240);
		} else {
			game->drawBgBottom("Failed to connect to Wi-Fi", 240);
			return;
		}
	} else {
		Wifi_EnableWifi();
	}

	game->drawBgBottom("Updating words...", 240);
	HttpResponse res = httpGet("wordle.xn--rck9c.xn--tckwe", "/words.php?date=2023-02-24&include=id");
	if(res.status == 200) {
		Json words(res.content.c_str(), false);
		FILE *file = fopen("/test.json", "wb");
		if(file) {
			std::string str = words.dump();
			fwrite(str.c_str(), 1, str.size(), file);
			fclose(file);
		}
		game->drawBgBottom("Update successful!", 240);
	} else {
		game->drawBgBottom("Update failed!!", 240);
	}

	Wifi_DisableWifi();
}
