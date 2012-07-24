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
package br.larc.usp.simulations;

import br.larc.usp.controller.ClientController;
import br.larc.usp.nodes.Peer;
import br.larc.usp.nodes.PeerInterface;

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.ethernet.EtherHost;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;


//
// Basic ethernet topology with one router and everybody connected
//
network BasicTopology extends BasicNetwork
{
    parameters:
        @signal[SwarmManager_EnterSwarm];
        @statistic[EnterSwarm](
            source=SwarmManager_EnterSwarm;
            title="Enter Swarm Time";
            record=vector;
        );
        @signal[SwarmManager_LeaveSwarm];
        @statistic[LeaveSwarm](
            source=SwarmManager_LeaveSwarm;
            title="Leave Swarm Time";
            record=vector;
        );
        @signal[ContentManager_100_percentDownloadMarkTime];
        @statistic[DownloadComplete](
            source=ContentManager_100_percentDownloadMarkTime;
            title="Download Complete";
            unit=s;
            record=vector,histogram
        );

    submodules:
//        tracker: Tracker;
        peer[numPeers]: <> like PeerInterface;
        router: Router;
        flatNetworkConfigurator: FlatNetworkConfigurator;
        clientController: ClientController {
            @display("p=296,200");
        }
        tracker: StandardHost {
            numTcpApps = 1;
            tcpAppType = "TrackerApp";
            @display("p=392,293;i=device/server2");
        }
    connections:
        for i=0..numPeers-1 {
            peer[i].ethg <--> ethernetline <--> router.ethg++;
        }
        tracker.ethg++ <--> ethernetline <--> router.ethg++;
}
