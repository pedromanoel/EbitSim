#!/usr/bin/python
### This script breaks a full debug log by Peer. Each Peer will then
### have a debug log in a separate file, saved in the directory from
### where this script was called

def process_log(log_file):
    # group the log lines by their peer_id
    list_of_peers = {}
    # list of log lines that don't match with a peer
    unlisted = []

    with open(log_file, "r") as in_file:
        peer_regexp_str = "(\d+)" # event number
        peer_regexp_str += ".*Peer (\d+): " # peer identification
        peer_regexp_str += "(?:\(Thread\w+\) )?" # optional thread identification
        peer_regexp_str += "(?:connId (\d+),)?" # connection id

        # the previous event number, used to group logs with the same EV #
        prev_ev = None
        # in the case a log line don't have peerId, use the previous one
        prev_peer_id = None
        # in the case a log line don't have connId, use the previous one
        prev_conn_id = None

        for line in in_file:
            import re
            # see if the current line matches a peer log
            s = re.search(peer_regexp_str, line)
            if s:
                peer_id = int(s.group(2))

                # initialize the list for this peer_id
                # initialize the dictionary for this peer_id
                if peer_id not in list_of_peers:
                    list_of_peers[peer_id] = []

                # group lines with the same event number by removing the first char 
                ev = s.group(1)
                if ev != prev_ev:
                    prev_ev = ev

                # reset prev_conn_id, since the peer_id changed 
                if peer_id != prev_peer_id:
                    prev_peer_id = peer_id

                list_of_peers[peer_id].append(line)

            else:
                unlisted.append(line)

        out_filename = "peer_%s.log"

    # write to file the log messages ordered by Peer
    for peer_id, lines in list_of_peers.iteritems():
        with open(out_filename % peer_id, "w") as f:
            f.write("=== Peer %d ===\n" % peer_id)
            f.writelines(lines)

    # write to file the lines that didn't match the pattern
    with open("unlisted.log", "w") as f:
        for line in unlisted:
            f.write(line)

def show_usage():
    print "Usage:", sys.argv[0], "<err_log_file>"

if __name__ == "__main__":
    import sys

    if len(sys.argv) != 2:
        show_usage()
        for i, par in enumerate(sys.argv):
            print i, par
    else:
        process_log(sys.argv[1])
