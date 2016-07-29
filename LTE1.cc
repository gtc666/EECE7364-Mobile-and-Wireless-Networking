/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Manuel Requena <manuel.requena@cttc.es>
 *         Nicola Baldo <nbaldo@cttc.es>
 */


#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/point-to-point-helper.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store.h"
#include "ns3/radio-bearer-stats-calculator.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/random-walk-2d-mobility-model.h"

#include <iomanip>
#include <string>


using namespace ns3;

//NS_LOG_COMPONET_DEFINE ("LogDistancePropagationLossModel");

/**
 * This simulation script creates two eNodeBs and drops randomly several UEs in
 * a disc around them (same number on both). The number of UEs , the radius of
 * that disc and the distance between the eNodeBs can be configured.
 */
int main (int argc, char *argv[])
{
  double enbDist = 100.0;
  double radius = 50.0;
  uint32_t numUes = 1;
  
    double simTime = 150.0;
    double interPacketInterval = 100;
    double enbX = 100.0;
    double enbY = 300.0;
    uint32_t numenb = 4;
    uint32_t numueN = 10;
    uint32_t bandwidth = 100; //in number of Resource Blocks

  std::ostringstream tag;
  tag  << "_enbDist" << std::setw (3) << std::setfill ('0') << std::fixed << std::setprecision (0) << enbDist
       << "_radius"  << std::setw (3) << std::setfill ('0') << std::fixed << std::setprecision (0) << radius
       << "_numUes"  << std::setw (3) << std::setfill ('0')  << numUes;


    Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
    Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
    lteHelper->SetEpcHelper (epcHelper);
    
    Ptr<Node> pgw = epcHelper->GetPgwNode ();

    // set path loss model
    lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::LogDistancePropagationLossModel"));
    
    // set handover algorithm
    lteHelper->SetHandoverAlgorithmType ("ns3::A2A4RsrqHandoverAlgorithm");
    lteHelper->SetHandoverAlgorithmAttribute ("ServingCellThreshold", UintegerValue (30));
    lteHelper->SetHandoverAlgorithmAttribute ("NeighbourCellOffset",UintegerValue (1));
    
    // Set Bandwidth to 20MHz
    lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (bandwidth));
    lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (bandwidth));
    
    //lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (7600)); //2190.0 MHz
    //lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (25600)); //2010.0 MHz
    
    //set Tx Power of eNodes to 20 dBm
    Config::SetDefault("ns3::LteEnbPhy::TxPower",DoubleValue(20));

    // Create a single RemoteHost
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create (1);
    Ptr<Node> remoteHost = remoteHostContainer.Get (0);
    InternetStackHelper internet;
    internet.Install (remoteHostContainer);

    // Create the Internet
    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
    p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
    p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
    NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
    // interface 0 is localhost, 1 is the p2p device
    Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);
    
    
    // Routing of the Internet Host (towards the LTE network)
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
    // interface 0 is localhost, 1 is the p2p device
    remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
    
    
    
    
    // Create Nodes: eNodeB and UE
    NodeContainer enbNodes;
    NodeContainer ueNodes1, ueNodes2, ueNodes3, ueNodes4;
    enbNodes.Create (numenb);
    ueNodes1.Create (numueN);
    ueNodes2.Create (numueN);
    ueNodes3.Create (numueN);
    ueNodes4.Create (numueN);
    
    // Position of eNBs
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (Vector (enbX, enbX, 0.0));
    positionAlloc->Add (Vector (enbY, enbX, 0.0));
    positionAlloc->Add (Vector (enbX, enbY, 0.0));
    positionAlloc->Add (Vector (enbY, enbY, 0.0));
    MobilityHelper enbMobility;
    enbMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    enbMobility.SetPositionAllocator (positionAlloc);
    enbMobility.Install (enbNodes);



    //random walk
    //mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel","Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
    
     // Position of UEs attached to eNB 1
    MobilityHelper ue1mobility;
    ue1mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                                      "X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=200.0]"),
                                      "Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=200.0]") );
    ue1mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Bounds", RectangleValue (Rectangle (0, 400, 0, 400)));
    //ue1mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    ue1mobility.Install (ueNodes1);

    // Position of UEs attached to eNB 2
    MobilityHelper ue2mobility;
    ue2mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                                      "X", StringValue ("ns3::UniformRandomVariable[Min=200.0|Max=400.0]"),
                                      "Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=200.0]") );
    ue2mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Bounds", RectangleValue (Rectangle (0, 400, 0, 400)));
    //ue2mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    ue2mobility.Install (ueNodes2);

    // Position of UEs attached to eNB 3
    MobilityHelper ue3mobility;
    ue3mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                                      "X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=200.0]"),
                                      "Y", StringValue ("ns3::UniformRandomVariable[Min=200.0|Max=400.0]") );
    ue3mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Bounds", RectangleValue (Rectangle (0, 400, 0, 400)));
    //ue3mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    ue3mobility.Install (ueNodes3);

    // Position of UEs attached to eNB 4
    MobilityHelper ue4mobility;
    ue4mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                                      "X", StringValue ("ns3::UniformRandomVariable[Min=200.0|Max=400.0]"),
                                      "Y", StringValue ("ns3::UniformRandomVariable[Min=200.0|Max=400.0]") );
    ue4mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Bounds", RectangleValue (Rectangle (0, 400, 0, 400)));
    //ue4mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    ue4mobility.Install (ueNodes4);
 
    // Create Devices and install them in the Nodes (eNB and UE)
    NetDeviceContainer enbDevs;
    NetDeviceContainer ueDevs1;
    NetDeviceContainer ueDevs2;
    NetDeviceContainer ueDevs3;
    NetDeviceContainer ueDevs4;
    enbDevs = lteHelper->InstallEnbDevice (enbNodes);
    ueDevs1 = lteHelper->InstallUeDevice (ueNodes1);
    ueDevs2 = lteHelper->InstallUeDevice (ueNodes2);
    ueDevs3 = lteHelper->InstallUeDevice (ueNodes3);
    ueDevs4 = lteHelper->InstallUeDevice (ueNodes4);
    
    
    // Install the IP stack on the UEs
    internet.Install (ueNodes1);
    Ipv4InterfaceContainer ueIpIface1;
    ueIpIface1 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs1));
    
    internet.Install (ueNodes2);
    Ipv4InterfaceContainer ueIpIface2;
    ueIpIface2 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs2));
    
    internet.Install (ueNodes3);
    Ipv4InterfaceContainer ueIpIface3;
    ueIpIface3 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs3));
    
    internet.Install (ueNodes4);
    Ipv4InterfaceContainer ueIpIface4;
    ueIpIface4 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs4));
    
    // ********Assign IP address to UE1s
    for (uint32_t u = 0; u < ueNodes1.GetN (); ++u)
    {
        Ptr<Node> ueNode1 = ueNodes1.Get (u);
        // Set the default gateway for the UE
        Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode1->GetObject<Ipv4> ());
        ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }
    // ********Assign IP address to UE2s
    for (uint32_t u = 0; u < ueNodes2.GetN (); ++u)
    {
        Ptr<Node> ueNode2 = ueNodes2.Get (u);
        // Set the default gateway for the UE
        Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode2->GetObject<Ipv4> ());
        ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }
    // ********Assign IP address to UE3s
    for (uint32_t u = 0; u < ueNodes3.GetN (); ++u)
    {
        Ptr<Node> ueNode3 = ueNodes3.Get (u);
        // Set the default gateway for the UE
        Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode3->GetObject<Ipv4> ());
        ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }
    // ********Assign IP address to UE4s
    for (uint32_t u = 0; u < ueNodes4.GetN (); ++u)
    {
        Ptr<Node> ueNode4 = ueNodes4.Get (u);
        // Set the default gateway for the UE
        Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode4->GetObject<Ipv4> ());
        ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }
    
    // Attach UEs to a eNB
    lteHelper->Attach (ueDevs1, enbDevs.Get (0));
    lteHelper->Attach (ueDevs2, enbDevs.Get (1));
    lteHelper->Attach (ueDevs3, enbDevs.Get (2));
    lteHelper->Attach (ueDevs4, enbDevs.Get (3));
    
    
    // Install and start applications on UEs and remote host
  uint16_t dlPort = 1234;
  uint16_t ulPort = 2000;
  uint16_t otherPort = 3000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  for (uint32_t u = 0; u < ueNodes1.GetN (); ++u)
    {
      ++ulPort;
      ++otherPort;
      PacketSinkHelper dlPacketSinkHelper1 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
      PacketSinkHelper ulPacketSinkHelper1 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
      PacketSinkHelper packetSinkHelper1 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));
      serverApps.Add (dlPacketSinkHelper1.Install (ueNodes1.Get(u)));
      serverApps.Add (ulPacketSinkHelper1.Install (remoteHost));
      serverApps.Add (packetSinkHelper1.Install (ueNodes1.Get(u)));

      UdpClientHelper dlClient1 (ueIpIface1.GetAddress (u), dlPort);
      dlClient1.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
      dlClient1.SetAttribute ("MaxPackets", UintegerValue(1000000));

      UdpClientHelper ulClient1 (remoteHostAddr, ulPort);
      ulClient1.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
      ulClient1.SetAttribute ("MaxPackets", UintegerValue(1000000));

      UdpClientHelper client1 (ueIpIface1.GetAddress (u), otherPort);
      client1.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
      client1.SetAttribute ("MaxPackets", UintegerValue(1000000));

      clientApps.Add (dlClient1.Install (remoteHost));
      clientApps.Add (ulClient1.Install (ueNodes1.Get(u)));
      if (u+1 < ueNodes1.GetN ())
        {
          clientApps.Add (client1.Install (ueNodes1.Get(u+1)));
        }
      else
        {
          clientApps.Add (client1.Install (ueNodes1.Get(0)));
        }
    }
    

    for (uint32_t u = 0; u < ueNodes2.GetN (); ++u)
    {
        ++ulPort;
        ++otherPort;
        PacketSinkHelper dlPacketSinkHelper2 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
        PacketSinkHelper ulPacketSinkHelper2 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
        PacketSinkHelper packetSinkHelper2 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));
        serverApps.Add (dlPacketSinkHelper2.Install (ueNodes2.Get(u)));
        serverApps.Add (ulPacketSinkHelper2.Install (remoteHost));
        serverApps.Add (packetSinkHelper2.Install (ueNodes2.Get(u)));
        
        UdpClientHelper dlClient2 (ueIpIface2.GetAddress (u), dlPort);
        dlClient2.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
        dlClient2.SetAttribute ("MaxPackets", UintegerValue(1000000));
        
        UdpClientHelper ulClient2 (remoteHostAddr, ulPort);
        ulClient2.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
        ulClient2.SetAttribute ("MaxPackets", UintegerValue(1000000));
        
        UdpClientHelper client2 (ueIpIface2.GetAddress (u), otherPort);
        client2.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
        client2.SetAttribute ("MaxPackets", UintegerValue(1000000));
        
        clientApps.Add (dlClient2.Install (remoteHost));
        clientApps.Add (ulClient2.Install (ueNodes2.Get(u)));
        if (u+1 < ueNodes2.GetN ())
        {
            clientApps.Add (client2.Install (ueNodes2.Get(u+1)));
        }
        else
        {
            clientApps.Add (client2.Install (ueNodes2.Get(0)));
        }
    }
    
    for (uint32_t u = 0; u < ueNodes3.GetN (); ++u)
    {
        ++ulPort;
        ++otherPort;
        PacketSinkHelper dlPacketSinkHelper3 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
        PacketSinkHelper ulPacketSinkHelper3 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
        PacketSinkHelper packetSinkHelper3 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));
        serverApps.Add (dlPacketSinkHelper3.Install (ueNodes3.Get(u)));
        serverApps.Add (ulPacketSinkHelper3.Install (remoteHost));
        serverApps.Add (packetSinkHelper3.Install (ueNodes3.Get(u)));
        
        UdpClientHelper dlClient3 (ueIpIface3.GetAddress (u), dlPort);
        dlClient3.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
        dlClient3.SetAttribute ("MaxPackets", UintegerValue(1000000));
        
        UdpClientHelper ulClient3 (remoteHostAddr, ulPort);
        ulClient3.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
        ulClient3.SetAttribute ("MaxPackets", UintegerValue(1000000));
        
        UdpClientHelper client3 (ueIpIface3.GetAddress (u), otherPort);
        client3.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
        client3.SetAttribute ("MaxPackets", UintegerValue(1000000));
        
        clientApps.Add (dlClient3.Install (remoteHost));
        clientApps.Add (ulClient3.Install (ueNodes3.Get(u)));
        if (u+1 < ueNodes3.GetN ())
        {
            clientApps.Add (client3.Install (ueNodes3.Get(u+1)));
        }
        else
        {
            clientApps.Add (client3.Install (ueNodes3.Get(0)));
        }
    }
    
    for (uint32_t u = 0; u < ueNodes4.GetN (); ++u)
    {
        ++ulPort;
        ++otherPort;
        PacketSinkHelper dlPacketSinkHelper4 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
        PacketSinkHelper ulPacketSinkHelper4 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
        PacketSinkHelper packetSinkHelper4 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));
        serverApps.Add (dlPacketSinkHelper4.Install (ueNodes4.Get(u)));
        serverApps.Add (ulPacketSinkHelper4.Install (remoteHost));
        serverApps.Add (packetSinkHelper4.Install (ueNodes4.Get(u)));
        
        UdpClientHelper dlClient4 (ueIpIface4.GetAddress (u), dlPort);
        dlClient4.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
        dlClient4.SetAttribute ("MaxPackets", UintegerValue(1000000));
        
        UdpClientHelper ulClient4 (remoteHostAddr, ulPort);
        ulClient4.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
        ulClient4.SetAttribute ("MaxPackets", UintegerValue(1000000));
        
        UdpClientHelper client4 (ueIpIface4.GetAddress (u), otherPort);
        client4.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
        client4.SetAttribute ("MaxPackets", UintegerValue(1000000));
        
        clientApps.Add (dlClient4.Install (remoteHost));
        clientApps.Add (ulClient4.Install (ueNodes4.Get(u)));
        if (u+1 < ueNodes4.GetN ())
        {
            clientApps.Add (client4.Install (ueNodes4.Get(u+1)));
        }
        else
        {
            clientApps.Add (client4.Install (ueNodes4.Get(0)));
        }
    }
    
    
    serverApps.Start (Seconds (0.01));
    clientApps.Start (Seconds (0.01));
  







    Simulator::Stop (Seconds (simTime));
    
    //Insert RLC Performance Calculator
    std::string dlOutFname = "DlRlcStats";
    dlOutFname.append (tag.str ());
    std::string ulOutFname = "UlRlcStats";
    ulOutFname.append (tag.str ());
    
    lteHelper->EnableRlcTraces ();
    lteHelper->EnablePhyTraces ();


  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
