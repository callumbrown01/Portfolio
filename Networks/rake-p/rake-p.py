from parse import parse_rakefile, parse_connection_variables, parse_header
from connect import connect_to_server
from os import path
from struct import unpack

# from execute import echo
from sys import argv
from constants import DEFAULT_BUFFERSIZE, Command, Header
import selectors
from struct import pack
import sys
import logging

sel = selectors.DefaultSelector()

in_progress = []
complete = []

for arg in argv[1:]:
    if arg == "-v":
        logging.basicConfig(format="%(message)s", level=logging.INFO)
    else:
        filename = arg

files_dir = path.dirname(path.realpath(filename))

(actionsets, variables) = parse_rakefile(filename)
servers, default_port = parse_connection_variables(variables)
# TODO: update this to use PORT variable
local_server = ("localhost", int(default_port))

logging.info(f"actionsets: {actionsets}")


def handle_event(key, mask):
    server = key.fileobj
    address = key.data[0]
    data = key.data[1]
    received = []
    if mask & selectors.EVENT_READ:
        while True:
            try:
                encoded_header = server.recv(DEFAULT_BUFFERSIZE)
            except TimeoutError:
                logging.info(f"timeout while receiving from {address}")
                break
            if encoded_header:
                header = parse_header(encoded_header)
                logging.info(f"\t<--length {header.length}")
                if header.length != 0:
                    received_data = server.recv(header.length)
                    if not header.command == Command.RETURN_FILE:
                        received.append((header, received_data.decode()))
                    else:
                        received.append((header, received_data))
                else:
                    received.append((header, ""))
            else:
                break
    if mask & selectors.EVENT_WRITE:
        if data and key.fd:
            try:
                server.connect(address)
            except:
                logging.info("already connected")
            logging.info(f"sent data to {key.fd}")
            server.sendall(data)
        logging.info(f"setting {server} to read only")
        sel.modify(server, selectors.EVENT_READ, data=(address, b""))

    return received


def send_quote_requests(remote_actions, servers):
    quote_requests = []

    for i, action in enumerate(remote_actions):
        logging.info(f"preparing quote for {action} - intreply {i}")
        header = Header(Command.QUOTE_REQUEST, 0, i)
        quote_requests.append(header)

    for address in servers:
        server = connect_to_server(address)
        logging.info(f"registering {server} as write only")
        sel.register(
            server, selectors.EVENT_WRITE | selectors.EVENT_READ, data=(address, b"")
        )
        for quote_request in quote_requests:
            logging.info(
                f"sending quote request for {quote_request.intreply} to {address}"
            )
            server.sendall(quote_request.pack_header())


def send_files(action, i):
    (command, address) = action
    (cmd, remote, files) = command

    server = connect_to_server(address)
    sel.register(
        server, selectors.EVENT_WRITE | selectors.EVENT_READ, data=(address, b"")
    )

    for file in files:
        with open(files_dir + "/" + file, "rb") as f:
            data = f.read()

        encoded_filename = file.encode()

        file_header = pack("ii", len(encoded_filename), len(data))

        header = Header(
            Command.SEND_FILE, len(file_header) + len(encoded_filename) + len(data), i
        )
        server.sendall(header.pack_header() + file_header + encoded_filename + data)


def execute_command(action, i=len(in_progress)):
    (command, address) = action
    logging.info(f"command to execute: {command}")
    encoded_command = command[0].encode()
    server = connect_to_server(address)
    in_progress.append([server])
    header = Header(Command.EXECUTE, len(encoded_command), i)
    sel.register(
        server,
        selectors.EVENT_WRITE | selectors.EVENT_READ,
        data=(address, header.pack_header() + encoded_command),
    )


def execute_actionset(actionset):
    success = True
    error = ""

    logging.info(f"AS to execute: {actionset}")
    # creates an empty array for each remote action to store quotes
    remote_actions = [action for action in actionset if action[1]]
    # if remote flag is false, ready to execute on local server

    ready = [(action, local_server) for action in actionset if not action[1]]

    logging.info(f"remote actions: {remote_actions}")
    logging.info(f"local actions: {ready}")
    quotes = [[] for action in actionset if action[1] == True]

    logging.info(f"remote_actions: {remote_actions}")

    if remote_actions:
        send_quote_requests(remote_actions, servers)

    while True:
        events = sel.select(timeout=0.1)
        received = None

        if ready:
            for i, command in enumerate(ready):
                if len(command) == 2:
                    execute_command(command)
                    ready.remove(command)
                else:
                    send_files(command, len(in_progress))
                    execute_command(command)
                    ready.remove(command)

        for key, mask in events:
            received = handle_event(key, mask)
            while received:
                logging.info(f"in progress: {in_progress}")
                header, data = received.pop()
                logging.info(
                    f"received some {header.command.name} data for {header.intreply}"
                )

                if header.command == Command.QUOTE_REPLY:
                    server = key.data[0]
                    logging.info(f"quote server {server}")
                    quotes[header.intreply].append((server, data))
                    logging.info(
                        f"quote received for intreply {header.intreply}, {len(quotes[header.intreply])} received total of {len(servers)} quotes"
                    )
                    # if the number of quotes matches the number of servers, all quotes are received
                    if len(quotes[header.intreply]) == len(servers):
                        best_quote = min(
                            [quote[1] for quote in quotes[header.intreply]]
                        )
                        best_server = [
                            quote[0]
                            for quote in quotes[header.intreply]
                            if quote[1] == best_quote
                        ][0]
                        logging.info(
                            f"best quote for {remote_actions[header.intreply]}: {best_quote}"
                        )
                        ready.append((remote_actions[header.intreply], best_server))

                        finished_quoting = [
                            quote for quote in quotes if len(quote) == len(servers)
                        ]

                        if len(finished_quoting) == len(quotes):
                            logging.info("all quotes received")
                            sel.unregister(key.fileobj)

                if (
                    header.command == Command.RETURN_STATUS
                    or header.command == Command.RETURN_STDERR
                    or header.command == Command.RETURN_STDOUT
                    or header.command == Command.RETURN_FILE
                ):
                    server = key.fileobj
                    logging.info(f"in progress: {in_progress}")
                    logging.info(f"length: {len(in_progress[header.intreply])}")

                    if header.command == Command.RETURN_FILE:
                        if header.length > 0:
                            packed_lengths = data[:8]
                            lengths = unpack("ii", packed_lengths)
                            encoded_filename = data[8 : lengths[0] + 8]
                            file_contents = data[lengths[0] + 8 :]
                            filename = encoded_filename.decode()
                            logging.info(f"received file {filename}")

                            with open(files_dir + "/" + filename, "wb") as f:
                                f.write(file_contents)
                            in_progress[header.intreply].append((header, filename))
                        else:
                            in_progress[header.intreply].append((header, ""))
                    else:
                        in_progress[header.intreply].append((header, data))
                    # length will be 4 when all data is received

                    if len(in_progress[header.intreply]) > 4:
                        result = in_progress[header.intreply]
                        logging.info(f"processing results: {result}")

                        while len(in_progress[header.intreply]) > 1:
                            response = result.pop(1)
                            match response[0].command:
                                case Command.RETURN_STATUS:
                                    exit_status = response[1]
                                    if exit_status != "0":
                                        success = False
                                case Command.RETURN_STDOUT:
                                    stdout = response[1]
                                    if stdout != "": print(f"stdout: {stdout}")
                                case Command.RETURN_STDERR:
                                    stderr = response[1]
                                    if stderr != "": print(f"stderr: {stderr}")
                                case Command.RETURN_FILE:
                                    if response[1] != "":
                                        print(f"saved to {response[1]}")
                        complete.append(result)
                        if len(complete) == len(actionset):
                            if not success:
                                logging.info(f"an action in this set failed")
                                return 1
                            logging.info("execution completed successfully")
                            return 0
                        else:
                            logging.info(
                                f"{len(complete)} of {len(actionset)} actions completed"
                            )

                        if not server in [action[0] for action in in_progress]:
                            sel.unregister(key.fileobj)
                    else:
                        logging.info(
                            f"waiting for more data for {header.intreply} ({len(in_progress[header.intreply])}): {in_progress[header.intreply]}"
                        )


# actionsets are executed sequentially
for actionset in actionsets:
    in_progress = []
    complete = []
    if execute_actionset(actionset[1]) == 1:
        print(f"error in {actionset[0]}, exiting...")
        break
