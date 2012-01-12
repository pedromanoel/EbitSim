#!/usr/bin/python
'''A problem is occurring with the simulation.
Some peers are not downloading the whole content with the connections they
established after the first tracker response. This may be because this
particular subset of peers don't have all the pieces. Even though, at least
one peer should have all pieces, or else those leecher wouldn't have any
piece at all. The simulation still ends because those peers make a second
request to the tracker, receiving a fresh list of peers, probably with some
seeders in the mix.
This script parses the log files in search of received HaveMsg and
RequestBundles. The idea is to identify which peers didn't donwload the
whole file before the second request to the tracker and try to see if all
those peers are interconnected. If that's the case, one of those peers
should be connected to a seeder. With this, it is possible to identify the
bottleneck and, consequently, find the problem.
'''

def plot_graph(log_file):
    '''Parse the log file, create graph with the peers connected before
    the second tracker response, with those that finished the download
    after marked red.
    '''
    log_data = []
    
    with open(log_file, "r") as _file:
        log_data = _file.readlines()

    if log_data:
        #POSSIBLE INPUTS
        #'#123456 (T=1.234567890123)(BitTorrentClient) - Peer 100:
        # (Thread) connId 946, peerId 86, infoHash 201 -
        # Message "HaveMsg (56)" arrived'
        #'#123456 (T=1.234567890123)(BitTorrentClient) - Peer 100:
        # (ThreadDownload) connId 946, peerId 86, infoHash 201 -
        # Get RequestBundle((55,0)(55,1)(55,2)(55,3)(55,4)(55,5)(55,6)(55,7))'
        #'#123456 (T=1.234567890123)(ContentManager) - Peer 101:
        # infoHash 201 - Became a seeder 2'

        time_str = 'T=([0-9]+\.[0-9]+)' # time the event occured
        local_peer_str = 'Peer ([0-9]+)' # the local peer's id
        remote_peer_str = 'peerId ([0-9]+)' # the remote peer's id
        info_hash_str = 'infoHash ([0-9]+)' # the swarm's info_hash
        event_str = '(.*)'  # the event
 
        regexp_str = '\(' + time_str
        regexp_str += '\)\(.*' + local_peer_str
        regexp_str += ': (?:.*' + remote_peer_str + ', )?'
        regexp_str += info_hash_str
        regexp_str += ' - ' + event_str

        swarms = {}
        problematic_peers = set()

        for line in log_data:
            import re
            # see if the current line matches a peer log
            search_res = re.search(regexp_str, line)

            if search_res:
                instant = search_res.group(1)
                local_peer = search_res.group(2)
                remote_peer = search_res.group(3)
                info_hash = search_res.group(4)
                
                if remote_peer:
                    if info_hash not in swarms:
                        swarms[info_hash] = set()

                    # HaveMsg or GetRequestBundle before the second
                    # tracker response
                    if (float(instant) < 100.0 and
                        (remote_peer, local_peer) not in swarms[info_hash]):
                        # don't add if there is already a connection from
                        # the remote peer to the local_peer (remove
                        # duplicate connections)
                        swarms[info_hash].add((local_peer, remote_peer))
                    
                else:
                    # Became a seeder after the second tracker response
                    if float(instant) > 100.0:
                        problematic_peers.add(local_peer)

        # write graphviz dot file
        with open("connections.dot", "w") as out_f:
            out_f.write("graph Connections {\n node [overlap=false]\n")

            for info_hash in swarms.keys():
                # start a new subgraph for the current info_hash
                out_f.write("subgraph " + info_hash + " {\n")
                # give a name for this cluster
                out_f.write("label=\"Swarm " + info_hash + "\";\n")

                peer_labels = set()

                # write the connections
                for connection in swarms[info_hash]:
                    peer_labels.add(connection[0])
                    peer_labels.add(connection[1])

                    # prefix to uniquely identify a peer from different swarms
                    local_peer = "\"" + info_hash + "::" + connection[0] + "\""
                    remote_peer = "\"" + info_hash + "::" + connection[1] + "\""

                    out_f.write(local_peer + " -- " + remote_peer + ";\n")
                
                # write the labels
                for peer in peer_labels:
                    label_str = "\"" + info_hash + "::"
                    label_str += peer + "\"[label=\"" + peer + "\" "

                    if peer in problematic_peers:
                        label_str += "style=filled fillcolor=red"
                    label_str += "];\n"
                        
                    out_f.write(label_str)

                out_f.write("}\n") # close subgraph
                
            out_f.write("}\n") # close graph

        import subprocess
        result = subprocess.call(['/usr/bin/neato', '-T', 'png', '-o',
                                 'connections.png', 'connections.dot',
                                 '-Goverlap=false'])

        if result == 0:
            print "PNG file successfuly created"
        else:
            print "There was an error generating the graph"

def show_usage():
    ''' Print a help on the usage of this script. 
    '''
    print "Usage:", sys.argv[0], "<err_log_file>"

if __name__ == "__main__":
    import sys

    if len(sys.argv) != 2:
        show_usage()
        for i, par in enumerate(sys.argv):
            print i, par
    else:
        plot_graph(sys.argv[1])

