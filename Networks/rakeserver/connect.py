import logging
import socket
from constants import DEFAULT_PORT, DEFAULT_BUFFERSIZE
from protocol import Command, Header, write_socket, read_socket
from struct import unpack
from commands import echo_reply, quote, execute, receive_file

def start_server(PORT=DEFAULT_PORT):
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(("", PORT))
    server.listen()
    logging.info(f"listening on port {PORT}, sd={server.fileno()}")
    return server


def parse_header(header):
    command, length, intreply = unpack("iii", bytes(header))
    return Header(command, length, intreply)


def receive_data(client):
    while True:
        client.settimeout(0.1)
        raw_header = read_socket(client, DEFAULT_BUFFERSIZE)
        if not raw_header: break
        header = parse_header(raw_header)
        logging.info(f"received {header.command.name}, length {header.length}, intreply {header.intreply}")
        encoded_message = read_socket(client, header.length)
        if header.command != Command.SEND_FILE:
            message = encoded_message.decode()
            logging.info(f"\t <-- '{message}'")

        match header.command:
            case Command.ECHO:
                logging.info("echo request received")
                echo_reply(message, client)
            case Command.QUOTE_REQUEST:
                logging.info("quote request received")
                logging.info(message)
                quote((header, message), client)
            case Command.EXECUTE:
                logging.info("execute command received")
                execute((header, message), client)
            case Command.SEND_FILE:
                logging.info("send_file header received")
                receive_file((header, encoded_message), client)
