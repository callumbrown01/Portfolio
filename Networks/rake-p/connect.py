import socket
from constants import DEFAULT_BUFFERSIZE
from parse import parse_header
import logging


def connect_to_server(address):
    logging.info(f"connecting to server {address}")
    client = socket.socket()
    client.settimeout(1)
    client.connect(address)
    return client


def close_connection(client):
    client.shutdown(socket.SHUT_RDWR)
    client.close()


# currently only supports one host at a time
# TODO: error handling for this - if post/host not found, if port is not a number, if ip addr in wrong format
# TODO: splitting up hosts when multiple
def parse_connection_variables(variables):
    for var in variables:
        if var[0] == "PORT":
            PORT = var[1]
        if var[0] == "HOSTS":
            HOSTS = var[1]

    logging.info("PORT\t = {PORT}")
    logging.info("HOSTS\t = {HOSTS}")
    return (HOSTS, PORT)
