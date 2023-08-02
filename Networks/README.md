# 3002Project
Networks project, developing an internetworked compiler and linker of C programs across different devices connected via internet protocols.

1. developing a client in python (rake-p.py) that reads through a 'rakefile' and stores the information in arrays.

    - 'PORT NUMBER = XXXX' connection information leads with no tabs.
    - 'actionsetx:' an 'actionset' gives a structure/order for the client to move through on execution.
    - '    action x x x' a single tab followed by a statement identifies an action to be carried out by the client itself.
    - '        file requirements.x' two tabs followed by file name(s) details the dependent files required for the client to execute the previous action.

2. developing a client in c (rake-c) 

3. developing a rakeserver 
    
    - A list of all the types of packets to be exchanged between the client and server
        (echo, echoreply, quote_request, quote_reply, send_file, execute, return_status, return_stdout, return_stderr, return_file).

REPORT
the protocol you have designed and developed for all communication between your client and server programs,

a 'walk-through' of the execution sequence employed to compile and link an multi-file program, and
 
the conditions under which remote compilation and linking appears to perform better (faster) than just using your local machine.
