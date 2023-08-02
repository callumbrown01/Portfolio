from sys import argv
from connect import start_server, receive_data
from constants import DEFAULT_PORT
import logging

PORT = DEFAULT_PORT

# parses command line arguments
for arg in argv[1:]:
    if arg == "-v":
        logging.basicConfig(format="%(message)s", level=logging.INFO)
    elif arg.isnumeric():
        PORT = int(arg)
    else:
        print(f"usage: python3 rakeserver.py [-v] [PORT]")
        exit(0)

server = start_server(PORT)

while True:
    clientSocket, clientAddress = server.accept()

    logging.info(f"accepted new client on sd = {clientSocket.fileno()}")

    with clientSocket:
        try:
            data = receive_data(clientSocket)
        except (BrokenPipeError, ConnectionResetError):
            logging.info(f"connection to {clientSocket.fileno()} was terminated")
            continue
        logging.info(data)
        if not data or data == b"" or data == None:
            logging.info(f"closing connection to sd={clientSocket.fileno()}")
            continue
