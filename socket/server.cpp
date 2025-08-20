#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

int main() { 
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); 
    /*
        int socket(int domain, int type, int protocol)
        domain:주소 페밀리 AF_INET(IPv4), AF_INET6(IPv6) , http는 TCP 기반
        type:소켓 타입 SOCK_STREAM(TCP), SOCK_DGRAM(UDP)
        protocol: 특수한 경우 외에는 0
        mlx_init 같은 함수 
    */
    if (server_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }
    
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        /*
            기본적으로 소켓을 재사용하기 위해, 기본적으로 서버가 종료되면 운영체제가 일정시간 포트를 점유함
            이때 바로 같은 포트로 서버를 재시작하면 에러가 발생하는데
            SO_REUSEADDR를 설정하면 종류 직후에도 사용 가능하다

            int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
            sockfd : 옵션을 설정할 소켓의 파일 디스크립터(예: socket 함수로 생성한 값)
            level : 옵션의 종류를 지정 (예: SOL_SOCKET은 소켓 레벨 옵션)
            optname : 설정할 옵션 이름 (예: SO_REUSEADDR은 포트 재사용)
            optval : 옵션에 설정할 값의 포인터 (예: int형 변수의 주소)
            optlen : 옵션 값의 크기 (예: sizeof(int))

            level (옵션 종류)
                SOL_SOCKET : 소켓 레벨 옵션
                IPPROTO_TCP : TCP 프로토콜 관련 옵션
                IPPROTO_IP : IP 프로토콜 관련 옵션
            optname (옵션 이름)
                SO_REUSEADDR : 포트 재사용
                SO_KEEPALIVE : 연결 유지(Keep-Alive)
                SO_RCVBUF : 수신 버퍼 크기
                SO_SNDBUF : 송신 버퍼 크기
                SO_BROADCAST : 브로드캐스트 허용
                SO_LINGER : 연결 종료 시 대기 시간 설정
                SO_OOBINLINE : 긴급 데이터 인라인 처리
            optval (옵션 값)
                대부분 int형 값(예: 1은 활성화, 0은 비활성화)
                버퍼 크기 등은 원하는 크기(int)
                구조체가 필요한 옵션도 있음(예: SO_LINGER는 struct linger)
        */
        perror("setsockopt failed");
        close(server_fd);
        return 1;
    }
    
    struct sockaddr_in server_addr;
    /*
        sockaddr_in : IPv4 주소용
        sockaddr_in6 : IPv6 주소용
    */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // 모든 IP 허용
    server_addr.sin_port = htons(8080);        // 포트 8080
    
    // 4. 주소 바인딩
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        /*
            bind 함수는 소켓에 IP 주소와 포트 번호를 연결(바인딩)하는 역할을 합니다.

            함수 원형: int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

            sockfd : 바인딩할 소켓의 파일 디스크립터
            addr : 바인딩할 주소 정보(예: IP, 포트 등)를 담은 구조체의 포인터
            addrlen : 주소 구조체의 크기
            서버에서는 bind를 통해 "이 소켓은 이 IP와 이 포트에서 연결을 받을 거야"라고 운영체제에 알려줍니다.
            클라이언트는 보통 생략하지만, 서버는 반드시 필요합니다.
        */
        perror("Bind failed");
        close(server_fd);
        return 1;
    }
    
    if (listen(server_fd, 5) < 0) { //listen(socket, 동사에 대기할 수 있는 클라이언트 수)
        /*
            listen 함수는 서버 소켓을 "연결 대기" 상태로 전환시켜, 클라이언트의 접속 요청을 받을 수 있게 해줍니다.

            함수 원형:
                int listen(int sockfd, int backlog);

            sockfd : 대기 상태로 만들 소켓의 파일 디스크립터
            backlog : 동시에 대기할 수 있는 최대 연결 요청 수(큐의 크기)
            
            즉,bind로 IP/포트를 지정한 후, listen을 호출하면 서버가 클라이언트의 연결 요청을 받을 준비가 됩니다.
            이후 accept로 실제 연결을 처리합니다.
        */
        perror("Listen failed");
        close(server_fd);
        return 1;
    }
    
    cout << "채팅 서버 시작: 포트 8080에서 대기 중..." << endl;
    
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(client_addr.sin_port);
        
        cout << "클라이언트 연결: " << client_ip << ":" << client_port << endl;
        
        string welcome = "채팅 서버에 오신 것을 환영합니다!\n메시지를 입력하세요 (quit로 종료):\n";
        send(client_fd, welcome.c_str(), welcome.length(), 0);
        
        char buffer[1024];
        while (true) {
            memset(buffer, 0, sizeof(buffer));
            
            int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            
            if (bytes_received <= 0) {
                cout << "클라이언트 " << client_ip << ":" << client_port << " 연결 종료" << endl;
                break;
            }
            
            string message(buffer, bytes_received);
  
            if (!message.empty() && message.back() == '\n') {
                message.pop_back();
            }
            
            cout << "받은 메시지 [" << client_ip << "]: " << message << endl;
            
            if (message == "quit") {
                string goodbye = "안녕히 가세요!\n";
                send(client_fd, goodbye.c_str(), goodbye.length(), 0);
                break;
            }
            
            string response = "서버: " + message + " (받았습니다)\n";
            send(client_fd, response.c_str(), response.length(), 0);
        }
        
        close(client_fd);
    }
    
    close(server_fd);
    return 0;
}
