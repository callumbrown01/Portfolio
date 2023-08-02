from struct import unpack
from constants import Header, DEFAULT_PORT

# parses a rake protocol header
def parse_header(header):
    unpacked = unpack("iii", header)
    return Header(unpacked[0], unpacked[1], unpacked[2])


# parses a rakefile, including ports and hosts, and producing arrays for actions and variables
# TODO: fix spaghetti
def parse_rakefile(filename):
    file = open(filename, "r")

    actions = []
    variables = []

    for line in file:
        hashtag = line.find("#")
        if hashtag != -1:
            line = line[:hashtag]
        if line in ["", "\n"] or line.startswith("#"):  # empty line, do nothing
            continue
        # TODO: support for multiple lines of required files
        elif line.startswith(("        ", "\t\t")):  # two tabs = required files
            files = line.split()
            actions[-1][-1][-1].append(files[1:])  # don't include files[0] (requires)
        elif line.startswith(("    ", "\t")):  # one tab = action
            action = line.strip(" \n\t")
            if action.startswith("remote"):
                action = [action[action.find("-") + 1 :], True]  # remote = True
            else:
                action = [action, False]
            actions[-1][-1].append(action)
        else:
            if "=" in line:  # variable (PORT, HOST)
                var = [terms.strip() for terms in line.split("=")]
                variables.append(var)
            elif ":" in line:  # actionset
                # append actionset name and empty array for actions
                actions.append([line.strip("\n:"), []])
            else:
                print("something is wrong", line)

    file.close()

    return (actions, variables)


# currently only supports one host at a time
# TODO: error handling for this - if post/host not found, if port is not a number, if ip addr in wrong format
# TODO: splitting up hosts when multiple
def parse_connection_variables(variables):
    HOSTS_LIST = []
    PORT = DEFAULT_PORT
    for var in variables:
        if var[0] == "PORT":
            PORT = var[1]
        if var[0] == "HOSTS":
            HOSTS_LIST = var[1].split()
        # if hosts are specified, add them to array
    if HOSTS_LIST:
        HOSTS = []
        for host in HOSTS_LIST:
            # if port is specified, use it
            if ":" in host:
                [hostname, port] = host.split(":")
                HOSTS.append((hostname, int(port)))
            # if port isn't specified, use rakefile's default port
            else:
                HOSTS.append((host, int(PORT)))
    # if a host isn't specified, use localhost
    else:
        HOSTS = []

    return (HOSTS, PORT)
