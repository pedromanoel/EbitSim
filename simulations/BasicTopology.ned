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
import br.larc.usp.nodes.RouterThruput;
import br.larc.usp.nodes.PeerInterface;
import br.larc.usp.nodes.Tracker;

import inet.networklayer.autorouting.FlatNetworkConfigurator;

//
// Basic ethernet topology with one router and everybody connected
//
network BasicTopology extends BasicNetwork
{
    parameters:
        @statistic[SwarmManager_EnteredSwarm](title="Enter Swarm Instant"; record=vector);
        @statistic[ContentManager_100_percentDownloadMarkTime](title="100% Download Time Ordinate"; unit=s; record=vector,histogram);

    submodules:
        tracker: Tracker;
        peer[numPeers]: <> like PeerInterface;
        routerThruput: RouterThruput;
        flatNetworkConfigurator: FlatNetworkConfigurator;
        clientController: ClientController {
            @display("p=296,200");
        }
    connections:
        for i=0..numPeers-1 {
            peer[i].ethg <--> ethernetline <--> routerThruput.ethg++;
        }
        tracker.ethg <--> ethernetline <--> routerThruput.ethg++;
}
