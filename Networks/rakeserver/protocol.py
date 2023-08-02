from enum import IntEnum
from struct import pack


class Command(IntEnum):
    ECHO = 0
    ECHO_REPLY = 1

    QUOTE_REQUEST = 2
    QUOTE_REPLY = 3

    SEND_FILE = 4

    EXECUTE = 5
    RETURN_STATUS = 6
    RETURN_STDOUT = 7
    RETURN_STDERR = 8
    RETURN_FILE = 9


class Header:
    def __init__(self, command_type, length, intreply=-1):
        if type(command_type) is int:
            self.command = Command(command_type)
        else:
            self.command = command_type
        self.length = length
        self.intreply = intreply

    def pack_header(self):
        return pack("iii", int(self.command), self.length, int(self.intreply))


def read_socket(client, nbytes):
    try:
        return client.recv(nbytes)
    except TimeoutError:
        return


def write_socket(client, data):
    client.send(data)
