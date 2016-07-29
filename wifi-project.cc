/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 Northeastern University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 *
 * 
 */
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"

//This is a simple example of an IEEE 802.11n Wi-Fi network.
//
//Network topology:
//
//  Wifi 192.168.1.0
//



using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("wifi-project");

#define NUM_STATIONS	10



int main (int argc, char *argv[])
{
  double simulationTime = 10; //seconds

  CommandLine cmd;
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.Parse (argc,argv);

  std::cout << "Throughput" << '\n';
  for (int i = 0; i <= 15; i++)
    {
      uint32_t payloadSize; //1500 byte IP packet

      payloadSize = 1472; //bytes

      NodeContainer wifiStaNode1, wifiStaNode2, wifiStaNode3, wifiStaNode4;
      NodeContainer wifiApNode;
      NodeContainer routerNode;

      wifiStaNode1.Create(NUM_STATIONS);
      wifiStaNode2.Create(NUM_STATIONS);
      wifiStaNode3.Create(NUM_STATIONS);
      wifiStaNode4.Create(NUM_STATIONS);
      wifiApNode.Create(4);
      routerNode.Create(4);

      YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
      YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
      phy.SetChannel (channel.Create ());

      WifiHelper wifi = WifiHelper::Default ();
      wifi.SetRemoteStationManager("ns3::AarfWifiManager");
      wifi.SetStandard (WIFI_PHY_STANDARD_80211g);
      NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();


      Ssid ssid = Ssid ("ns3-80211g");
      mac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssid),
                   "ActiveProbing", BooleanValue (false));

      NetDeviceContainer staDevice1, staDevice2, staDevice3, staDevice4;
      staDevice1 = wifi.Install(phy, mac, wifiStaNode1);
      staDevice2 = wifi.Install(phy, mac, wifiStaNode2);
      staDevice3 = wifi.Install(phy, mac, wifiStaNode3);
      staDevice4 = wifi.Install(phy, mac, wifiStaNode4);

      mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));

      NetDeviceContainer apDevice;
      apDevice = wifi.Install (phy, mac, wifiApNode);

      // mobility.
      MobilityHelper mobility_sta1, mobility_sta2, mobility_sta3, mobility_sta4;
      MobilityHelper mobility, mobility_r;
      Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
      Ptr<ListPositionAllocator> positionAlloc_r = CreateObject<ListPositionAllocator>();
      // Install mobility for Sta
      mobility_sta1.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                    "X", StringValue ("ns3::UniformRandomVariable[Min=-200.0|Max=0.0]"),
                    "Y", StringValue ("ns3::UniformRandomVariable[Min=200.0|Max=0.0]") );
      mobility_sta1.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
      				"Bounds", RectangleValue (Rectangle (-200, 200, -200, 200)));
      mobility_sta1.Install (wifiStaNode1);
      mobility_sta2.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                    "X", StringValue ("ns3::UniformRandomVariable[Min=-200.0|Max=0.0]"),
                    "Y", StringValue ("ns3::UniformRandomVariable[Min=-200.0|Max=0.0]") );
      mobility_sta2.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
      				"Bounds", RectangleValue (Rectangle (-200, 200, -200, 200)));
      mobility_sta2.Install (wifiStaNode2);
      mobility_sta3.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                    "X", StringValue ("ns3::UniformRandomVariable[Min=200.0|Max=0.0]"),
                    "Y", StringValue ("ns3::UniformRandomVariable[Min=-200.0|Max=0.0]") );
      mobility_sta3.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
      				"Bounds", RectangleValue (Rectangle (-200, 200, -200, 200)));
      mobility_sta3.Install (wifiStaNode3);
      mobility_sta4.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                    "X", StringValue ("ns3::UniformRandomVariable[Min=200.0|Max=0.0]"),
                    "Y", StringValue ("ns3::UniformRandomVariable[Min=200.0|Max=0.0]") );
      mobility_sta4.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
      				"Bounds", RectangleValue (Rectangle (-200, 200, -200, 200)));
      mobility_sta4.Install (wifiStaNode4);
      // Install mobility for AP
      positionAlloc->Add(Vector(-100, 100, 0));
      positionAlloc->Add(Vector(-100, -100, 0));
      positionAlloc->Add(Vector(100, -100, 0));
      positionAlloc->Add(Vector(100, 100, 0));
      mobility.SetPositionAllocator(positionAlloc);
      mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
      mobility.Install (wifiApNode);
      // Install mobility for router
      positionAlloc_r->Add(Vector(-100, 0, 0));
      positionAlloc_r->Add(Vector(0, -100, 0));
      positionAlloc_r->Add(Vector(100, 0, 0));
      positionAlloc_r->Add(Vector(0, 100, 0));
      mobility_r.SetPositionAllocator(positionAlloc_r);
      mobility_r.SetMobilityModel("ns3::ConstantPositionMobilityModel");
      mobility.Install (routerNode);


      // Router Configuration
      PointToPointHelper p2p;
      p2p.SetDeviceAttribute ("DataRate", StringValue ("20Mbps"));
      p2p.SetChannelAttribute ("Delay", StringValue ("1ms"));

      p2p.Install (NodeContainer (routerNode.Get(0), wifiApNode.Get(0)));
      p2p.Install (NodeContainer (routerNode.Get(0), wifiApNode.Get(1)));
      p2p.Install (NodeContainer (routerNode.Get(1), wifiApNode.Get(1)));
      p2p.Install (NodeContainer (routerNode.Get(1), wifiApNode.Get(2)));
      p2p.Install (NodeContainer (routerNode.Get(2), wifiApNode.Get(2)));
      p2p.Install (NodeContainer (routerNode.Get(2), wifiApNode.Get(3)));
      p2p.Install (NodeContainer (routerNode.Get(3), wifiApNode.Get(3)));
      p2p.Install (NodeContainer (routerNode.Get(3), wifiApNode.Get(0)));




      /* Internet stack*/
      InternetStackHelper stack;
      stack.Install(wifiApNode);
      stack.Install(wifiStaNode1);
      stack.Install(wifiStaNode2);
      stack.Install(wifiStaNode3);
      stack.Install(wifiStaNode4);
      stack.Install(routerNode);

      Ipv4AddressHelper address;

      address.SetBase ("192.168.1.0", "255.255.255.0");
      Ipv4InterfaceContainer staNodeInterface1, staNodeInterface2, staNodeInterface3, staNodeInterface4;
      Ipv4InterfaceContainer apNodeInterface;

      staNodeInterface1 = address.Assign(staDevice1);
      staNodeInterface2 = address.Assign(staDevice2);
      staNodeInterface3 = address.Assign(staDevice3);
      staNodeInterface4 = address.Assign(staDevice4);
      apNodeInterface = address.Assign(apDevice);

      /* Setting applications */
      UdpServerHelper Server1(6001), Server2(6002), Server3(6003), Server4(6004);
      UdpClientHelper Client[4];
      ApplicationContainer serverApp[4], clientApp[4];

      for (i = 0; i < 4; i ++) {
          Client[i] = UdpClientHelper(apNodeInterface.GetAddress(i), (6001 + i));
          Client[i].SetAttribute("MaxPackets", UintegerValue(4294967295u));
          Client[i].SetAttribute("Interval", TimeValue (Time("0.002")));
          Client[i].SetAttribute("PacketSize", UintegerValue(1024));
      }
      clientApp[0] = Client[0].Install(wifiStaNode1);
      clientApp[1] = Client[1].Install(wifiStaNode2);
      clientApp[2] = Client[2].Install(wifiStaNode3);
      clientApp[3] = Client[3].Install(wifiStaNode4);


      serverApp[0] = Server1.Install(wifiApNode.Get(0));
      serverApp[1] = Server2.Install(wifiApNode.Get(1));
      serverApp[2] = Server3.Install(wifiApNode.Get(2));
      serverApp[3] = Server4.Install(wifiApNode.Get(3));
      for (i = 0; i < 4; i ++) {
          serverApp[i].Start(Seconds(1.0));
          serverApp[i].Stop(Seconds(simulationTime + 1));

          clientApp[i].Start(Seconds(1.0));
          clientApp[i].Stop(Seconds(simulationTime + 1));
      }


      Ipv4GlobalRoutingHelper::PopulateRoutingTables ();




      Simulator::Stop (Seconds (simulationTime+1));
      Simulator::Run ();
      Simulator::Destroy ();

      double throughput = 0;
      //UDP
      uint32_t totalPacketsThrough = DynamicCast<UdpServer> (serverApp[0].Get(0))->GetReceived();
      throughput = totalPacketsThrough * payloadSize * 8 / (simulationTime * 1000000.0); //Mbit/s

      std::cout << throughput << " Mbit/s" << std::endl;
    }
  return 0;
}

