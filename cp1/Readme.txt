Deamon listen on port 1666 (TCP)

Supported commands:

E1 - enable counting of SIG_USR1
D1 - disable counting of SIG_USR1
E2 - enable counting of SIG_USR2
D2 - disable counting of SIG_USR2
STAT - show statistics of SIG_USR1 and SIG_USR2 received
EXIT - disconnect client from daemon

Signal action:
SIG_USR1, SIG_USR2 - counted by daemon or ignored
SIG_TERM - close network listen and exit
SIG_HUP - reread config (restore default params: SIG_USR1 and SIG_USR2 counted, all counters is zero)
