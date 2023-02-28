#include "game.hpp"
#include "json.hpp"

// #include <nds.h>
#include <dswifi9.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

std::string httpGet(const char *host, const char *path) {
	const char request[256];
	snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: Wordle DS\r\n\r\n", path, host);

	hostent *host = gethostbyname(host);
	int socket = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in sain;
	sain.sin_family = AF_INET;
	sain.sin_port = htons(80);
	sain.sin_addr.s_addr = *(u32 *)(host->h_addr_list[0]);
	connect(socket, (sockaddr *)&sain, sizeof(sain));

	send(socket, request, strlen(request), 0);

	int len;
	char buffer[256];
	std::string response;

	while((len = recv(socket, buffer, 255, 0)) != 0) {
		buffer[len] = '\0';
		response += buffer;
	}

	shutdown(socket, 0);
	closesocket(socket);

	return response;
}

void getWords(int day) {
	if(!Wifi_CheckInit()) {
		Wifi_InitDefault();
	} else {
		Wifi_EnableWifi();
	}

	std::string jsonStr = httpGet("wordle.xn--rck9c.xn--tckwe", "/word.php?day=2023-02-24");
	game->drawBgBottom(jsonStr, 240);

	Wifi_DisableWifi();
}
