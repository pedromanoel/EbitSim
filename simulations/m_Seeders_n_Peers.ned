//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package br.larc.usp.iptv.simulations;

import ned.IdealChannel;
import inet.nodes.inet.Router;
import br.larc.usp.iptv.controller.SimulationController;
import br.larc.usp.iptv.nodes.RouterThruput;
//import br.larc.usp.iptv.nodes.GlobalStatistics;
import inet.nodes.ethernet.EtherSwitch;
import br.larc.usp.iptv.nodes.Tracker;
import br.larc.usp.iptv.nodes.Peer;
import br.larc.usp.iptv.nodes.PeerMultiple;
import inet.networklayer.autorouting.FlatNetworkConfigurator;

import ned.DatarateChannel;

channel ethernetline extends DatarateChannel
{
    parameters:
        delay = 0.1us;
}

//
// TODO auto-generated module
//
network Seeders_xM_Peers_xN extends BasicNetwork
{
    parameters:
        @statistic[SwarmManager_EnteredSwarm](
            title="Enter Swarm Instant";
            record=vector
        );
        @statistic[ContentManager_100_percentDownloadMarkTime](
            title="100% Download Time Ordinate";
            unit=s;
            record=vector, histogram
        );
        // statistics from the ContentManager
    //        @statistic[ContentManager_PieceDownloadTime](
    //            title="Global Piece Download Time";
    //            unit=s;
    //            record=histogram
    //        );
    submodules:
        tracker: Tracker {
            @display("p=306,29");
        }
        peer[numPeers]: Peer {
            @display("p=,,ri,100,100");
        }
        routerThruput: RouterThruput {
            @display("p=150,150");
        }
        flatNetworkConfigurator: FlatNetworkConfigurator {
            @display("p=306,90");
        }
        //        globalStatistics: GlobalStatistics {
        //            @display("p=79,54");
        //        }
        simulationController: SimulationController {
            @display("p=79,98");
        }
    connections:
        for i=0..numPeers-1 {
            //            peer[i].ethg++ <--> ethernetline <--> router.ethg++;
            peer[i].ethg <--> ethernetline <--> routerThruput.ethg++;
        }
        tracker.ethg <--> ethernetline <--> routerThruput.ethg++;
}

network MultipleBitTorrentApps extends BasicNetwork
{
    parameters:
        @statistic[SwarmManager_EnteredSwarm](
            title="Enter Swarm Instant";
            record=vector
        );
        @statistic[ContentManager_100_percentDownloadMarkTime](
            title="100% Download Time Ordinate";
            unit=s;
            record=vector, histogram
        );
        // statistics from the ContentManager
    //        @statistic[ContentManager_PieceDownloadTime](
    //            title="Global Piece Download Time";
    //            unit=s;
    //            record=histogram
    //        );
    submodules:
        tracker: Tracker {
            @display("p=306,29");
        }
        peer[numPeers]: PeerMultiple {
            @display("p=,,ri,100,100");
        }
        routerThruput: RouterThruput {
            @display("p=150,150");
        }
        flatNetworkConfigurator: FlatNetworkConfigurator {
            @display("p=306,90");
        }
        //        globalStatistics: GlobalStatistics {
        //            @display("p=79,54");
        //        }
        simulationController: SimulationController {
            @display("p=79,98");
        }
    connections:
        for i=0..numPeers-1 {
            //            peer[i].ethg++ <--> ethernetline <--> router.ethg++;
            peer[i].ethg <--> ethernetline <--> routerThruput.ethg++;
        }
        tracker.ethg <--> ethernetline <--> routerThruput.ethg++;
}

network BigScope
{
    parameters:
        int numPeerS1;
        int numPeerS2;

    submodules:
        tracker: Tracker {
            @display("p=54,29");
        }
        cache: Peer {
            @display("p=54,100");
        }
        proxy: Peer {
            @display("p=333,29");
        }
        r1: Router {
            @display("p=195,29");
        }
        r2: Router {
            @display("p=195,100");
        }
        peerS1[numPeerS1]: Peer {
            @display("p=176,264,ri,50,50");
        }
        peerS2[numPeerS2]: Peer {
            @display("p=333,285,ri,50,50");
        }
        s1: Router {
            @display("p=54,197");
        }
        s2: Router {
            @display("p=333,194");
        }
        switch: EtherSwitch {
            @display("p=176,196");
        }
        flatNetworkConfigurator: FlatNetworkConfigurator {
            @display("p=457,29");
        }
        //        globalStatistics: GlobalStatistics {
        //            @display("p=79,54");
        //        }
        simulationController: SimulationController {
            @display("p=457,100");
        }
    connections:
        tracker.ethg <--> IdealChannel <--> r1.ethg++;
        proxy.ethg <--> IdealChannel <--> r1.ethg++;

        r1.ethg++ <--> IdealChannel <--> r2.ethg++;

        cache.ethg <--> IdealChannel <--> r2.ethg++;

        r2.ethg++ <--> IdealChannel <--> s1.ethg++;
        r2.ethg++ <--> IdealChannel <--> s2.ethg++;

        s1.ethg++ <--> IdealChannel <--> switch.ethg++;

        for i=0..numPeerS2-1 {
            peerS2[i].ethg <--> IdealChannel <--> s2.ethg++;
        }

        for i=0..numPeerS1-1 {
            peerS1[i].ethg <--> IdealChannel <--> switch.ethg++;
        }
}
