from protocol import read_socket, write_socket, Header, Command
from subprocess import run
from os import listdir, path
from time import time
import logging
from random import randrange
from struct import unpack, pack

# echoes a received message after a random delay
def echo_reply(message, client):
    sleep(getpid() % 5 + 2)
    encoded_message = message.encode()
    header = Header(Command.ECHO_REPLY, len(encoded_message))

    logging.info(f"\t--> length {header.length}")

    write_socket(client, header.pack_header())
    write_socket(client, encoded_message)


def quote(command, client):
    q = str(randrange(10)).encode()
    (request_header, message) = command
    header = Header(Command.QUOTE_REPLY, len(q), request_header.intreply)
    logging.info(f"\t--> quote {q.decode()}")
    write_socket(client, header.pack_header() + q)


def receive_file(data, client):
    (request_header, file) = data
    packed_lengths = file[:8]
    lengths = unpack("ii", packed_lengths)
    filename = file[8 : lengths[0] + 8]
    file_contents = file[lengths[0] + 8 :]
    logging.info(f"received file {filename.decode()}")
    with open(filename, "wb") as f:
        f.write(file_contents)


def execute(message, client):
    # need to report: return status, stdout, stderr, return_file
    start_time = time()
    logging.info(f"execute message: {message}")
    (request_header, command) = message
    result = run(command, shell=True, capture_output=True)
    logging.info(f"execute request header {request_header.intreply}")

    files = listdir("./")

    logging.info(f"files: {files}")

    new_file = ""

    for file in files:
        logging.info(f"checking time for {file}")
        if path.getctime(file) > start_time:
            logging.info(f"new file found: {file}")
            new_file = file

    logging.info(f"most recent file: {new_file}")
    # send the return status to the client
    return_status = str(result.returncode)
    status_header = Header(
        Command.RETURN_STATUS, len(return_status), request_header.intreply
    )
    logging.info(f"status_header intreply: {status_header.intreply}")
    logging.info(f"\t--> return status {return_status}")
    write_socket(client, status_header.pack_header() + str(return_status).encode())

    stdout = result.stdout
    stdout_header = Header(Command.RETURN_STDOUT, len(stdout), request_header.intreply)
    write_socket(client, stdout_header.pack_header() + stdout)
    logging.info(f"\t--> stdout {stdout.decode()}")

    stderr = result.stderr
    stderr_header = Header(Command.RETURN_STDERR, len(stderr), request_header.intreply)
    write_socket(client, stderr_header.pack_header() + stderr)
    logging.info(f"\t--> stderr {stderr.decode()}")

    if new_file:
        logging.info("recent file found")
        filename = path.basename(new_file)

        with open(filename, "rb") as f:
            data = f.read()

        encoded_filename = filename.encode()

        file_header = pack("ii", len(encoded_filename), len(data))

        logging.info(f"file_header type {type(file_header)}")
        logging.info(f"encoded_filename type {type(encoded_filename)}")
        logging.info(f"data type {type(data)}")

        header = Header(
            Command.RETURN_FILE,
            len(file_header) + len(encoded_filename) + len(data),
            request_header.intreply,
        )
        client.sendall(header.pack_header() + file_header + encoded_filename + data)

    else:
        logging.info("no recent file found")
        header = Header(Command.RETURN_FILE, 0, request_header.intreply)
        write_socket(client, header.pack_header())
