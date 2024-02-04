import socket

def transmit_order(host, port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
        client_socket.connect((host, port))
        print(f"서버에 연결됨: {host}:{port}")
        # 서버에 메세지 보내기
        client_socket.sendall(b'1')  # 문자 '1'을 바이트로 전송
        print(f"transmit order")

if __name__ == '__main__':    
    server_port = 2112
    server_ip = '10.10.59.56'
