#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string>

using namespace std;

int main() {
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(client_fd);
        return 1;
    }
    
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_fd);
        return 1;
    }
    
    cout << "서버에 연결되었습니다!" << endl;
    
    char buffer[1024];
    int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        cout << buffer;
    }
    
    string message;
    while (true) {
        cout << "메시지 입력: ";
        getline(cin, message);
        
        if (message.empty()) {
            continue;
        }
        
        message += "\n";
        send(client_fd, message.c_str(), message.length(), 0);
        
        if (message == "quit\n") {
            memset(buffer, 0, sizeof(buffer));
            bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                cout << buffer;
            }
            break;
        }
        
        memset(buffer, 0, sizeof(buffer));
        bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            cout << buffer;
        }
    }
    
    close(client_fd);
    cout << "클라이언트 종료" << endl;
    return 0;
}
