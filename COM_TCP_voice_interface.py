import socket


def start_voice_interface(host, port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        server_socket.bind((host, port))
        server_socket.listen()
        print(f"voice interface waiting from {host}:{port}.")

        connection, address = server_socket.accept()
        with connection:
            print(f"connected to {address}")
            while True:
                data = connection.recv(4)
                if not data:
                    break
                print(f"receive order from rapi")
                return 

if __name__ == "__main__":
    start_server('10.10.59.56', 2112)
