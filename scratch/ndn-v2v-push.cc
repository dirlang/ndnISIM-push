#include "ns3/core-module.h"

#include "ns3/mobility-module.h"
#include "ns3/network-module.h"


#include "ns3/ndnSIM-module.h"

#include "ns3/wifi-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ndnSIM/apps/ndn-producer.hpp"
#include "ns3/ndnSIM/apps/ndn-consumer-cbr.hpp"
#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include "ns3/ndnSIM/helper/ndn-app-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include <ns3/ndnSIM/helper/ndn-global-routing-helper.hpp>
#include "ns3/animation-interface.h"

#include "ns3/wave-net-device.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/wave-helper.h"

#include <algorithm>
#include <vector>

#include <sstream>



////////////////////////////////
// file: ndn-v2v.cc
// A V2V test with random-wait strategy
// Jiangtao Luo. 29 Mar 2020
// Run using: 
// ./waf --run ndn-v2v --command-template="%s random-wait"
// or:  ./waf --run ndn-v2v --command-template="%s multicast"   
      
////////////////////////////////
namespace ns3{

/**
 * This scenario simulates a scenario with 6 cars movind and communicating
 * in an ad-hoc way.
 *
 * 5 consumers request data from producer with frequency 1 interest per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-v2v-simple
 *
 * To modify the mobility model, see function installMobility.
 * To modify the wifi model, see function installWifi.
 * To modify the NDN settings, see function installNDN and for consumer and
 * producer settings, see functions installConsumer and installProducer
 * respectively.
 */

NS_LOG_COMPONENT_DEFINE ("ndn.v2v");


//static const uint32_t numNodes = 10;

// static const uint32_t numProducer = 1;
//static const uint32_t simulationEnd = 5;

typedef vector<uint32_t> vecProducerId;


class NdnV2VTest
{
public:
    NdnV2VTest(uint32_t numNodes, uint32_t numProducer);

// nPirs: number of background sessioins;
    // frequenct: frequency of Interests
    void Start(int nPairs, int frequency); 
    

// Set name prefix, like "/", "/V2V/Test"
    void
    setNamePrefix(string uri);

    // set strategy name
    // input last component,
    // "best-reout", "multicast", "random-wait"
    void
    setStrategy(string strStategy);

    // Install application delay trace on [0]
    void
    InstallTrace(string file);

    void
    setInterestOnCch(bool onCCH);  // set whether sending Interest on CCH

    // nPirs: number of background sessioins;
    // frequenct: frequency of Interests
    // A Push session as observed
    void
    StartPush(int nPairs, int frequency);

private:

    void InstallNDN(void);

    void InstallConsumer(void);

    void InstallProducer(void);


    void CreateWaveNodes(void);

    void StartWave(void);

    void InstallObserved(); // [0] consumer; [m_nNodes-1]: Producer

    void InstallBackground(int nPair, int frequency); // Observed traffic; nPair: number of consumer-producer pair

    void InstallObservedPush(); // [m_nNodes-1] ProducerPush

private:

    uint32_t m_nNodes;  // Number of nodes


    uint32_t m_nProducer; // number of Producers

    NodeContainer m_nodes; // the nodes
    NetDeviceContainer m_devices; // the devices

    int m_nodeSpeed; // m/s
    int m_nodePause; // s

    vecProducerId m_vecProducer;

    string m_namePrefix;  // Name prefix
    string  m_strategy; // Name strategy

    //AnimationInterface m_anim;  // NetAni Interface

    bool m_isInterestOnCch; // whether sending Interest on CCH of WAVE

     
};

NdnV2VTest::NdnV2VTest(uint32_t numNodes, uint32_t numProducer):
    m_nodeSpeed(20)
    , m_nodePause(0)
    , m_isInterestOnCch(false)
{

    NS_ASSERT(numProducer <= numNodes);
    
    m_nodes = NodeContainer();
    m_nodes.Create(numNodes);

    m_nNodes = numNodes;
    m_nProducer = numProducer;

    m_namePrefix ="/";
    setStrategy("multicast");

}

void
NdnV2VTest::setInterestOnCch(bool onCCH)
{
    m_isInterestOnCch = onCCH;
}
    

void
NdnV2VTest::setNamePrefix(string uri)
{
    NS_ASSERT(uri[0] == '/');
    m_namePrefix = uri;
}

void
NdnV2VTest::setStrategy(string strategy)
{
    m_strategy = ("/localhost/nfd/strategy/");

    if (strategy == "best-route" ||
        strategy == "multicast" ||
        strategy == "random-wait" )
    {
         m_strategy.append(strategy);
    }

}
    

// void printPosition(Ptr<const MobilityModel> mobility) //DEBUG purpose
// {
//   Simulator::Schedule(Seconds(1), &printPosition, mobility);
//   NS_LOG_INFO("Car "<<  mobility->GetObject<Node>()->GetId() << " is at: " <<mobility->GetPosition());
// }





void
NdnV2VTest::CreateWaveNodes(void)
{
    MobilityHelper mobility;
    // random number for positions and mobility
    Ptr<UniformRandomVariable> randomDistX = CreateObject<UniformRandomVariable>();
    randomDistX->SetAttribute("Min", DoubleValue(10));
    randomDistX->SetAttribute("Max", DoubleValue(60));

    Ptr<UniformRandomVariable> randomY = CreateObject<UniformRandomVariable>();
    randomY->SetAttribute("Min", DoubleValue(-5));
    randomY->SetAttribute("Max", DoubleValue(5));
   
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
 
    double xPos = 0.0;
    double yPos = 0.0;
    double y0 = 30.0;

    int j = 0;
    while (j < m_nNodes){
        xPos += randomDistX->GetValue();
        yPos = y0 + randomY->GetValue();
        
        positionAlloc->Add (Vector(xPos, yPos, 0));
        j++;
    }
    
    //mobility.SetPositionAllocator (positionAlloc);
    
    //mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

    // ObjectFactory pos;
    // pos.SetTypeId("ns3::RandomBoxPositionAllocator");
    // pos.Set("X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=800.0]"));
    // pos.Set("Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=8.0]"));
    // // we need antenna height uniform [1.0 .. 2.0] for loss model
    // pos.Set ("Z", StringValue ("ns3::UniformRandomVariable[Min=1.0|Max=2.0]"));

    // Ptr<PositionAllocator> taPositionAlloc = pos.Create()->GetObject<PositionAllocator>();

     // std::stringstream ssSpeed;
     // ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << m_nodeSpeed << "]";
     // std::stringstream ssPause;
     // ssPause << "ns3::ConstantRandomVariable[Constant=" << m_nodePause << "]";

     // mobility.SetMobilityModel("ns3::RandomWaypointMobilityModel",
     //                           "Speed", StringValue(ssSpeed.str()),
     //                           "Pause", StringValue(ssPause.str()),
     //                           // "PositionAllocator", PointerValue(taPositionAlloc));
     //                            "PositionAllocator", PointerValue(positionAlloc));
                               
     // mobility.SetPositionAllocator(taPositionAlloc);


    // ConstantVelocity model
    
    
     mobility.SetPositionAllocator (positionAlloc);
     mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
     mobility.Install(m_nodes);
     
      // set random velocity for each vehicle
     Ptr<UniformRandomVariable> randomXSpeed = CreateObject<UniformRandomVariable>();
     randomXSpeed->SetAttribute("Min", DoubleValue(15));  // unit: m/s
     randomXSpeed->SetAttribute("Max", DoubleValue(40));  // 140 km/h

     j = 0;
     double xSpeed = 0.0; // m/s
     while (j < m_nNodes) {
         // Get the MobilityModel
         Ptr<ConstantVelocityMobilityModel> pMob =
            m_nodes.Get(j)->GetObject<ConstantVelocityMobilityModel>();

             xSpeed = randomXSpeed->GetValue();

             pMob->SetVelocity(Vector(xSpeed, 0, 0));

             j++;
     }
        

    YansWifiChannelHelper waveChannel = YansWifiChannelHelper::Default();
    YansWavePhyHelper wavePhy = YansWavePhyHelper::Default();
    wavePhy.SetChannel(waveChannel.Create());

    QosWaveMacHelper waveMac = QosWaveMacHelper::Default();
    WaveHelper waveHelper = WaveHelper::Default();

    m_devices = waveHelper.Install(wavePhy, waveMac, m_nodes);

    
}

void
NdnV2VTest::StartWave(void)
{
    for (uint32_t i=0; i != m_devices.GetN(); i++)
    {
        Ptr<WaveNetDevice> device = DynamicCast<WaveNetDevice> (m_devices.Get(i));

        const TxProfile txProfile = TxProfile(SCH1);
        device->RegisterTxProfile(txProfile);

        SchInfo schInfo = SchInfo(SCH1, false, EXTENDED_ALTERNATING);

        device->StartSch(schInfo);
    }

}

    
// void installWifi(NodeContainer &c, NetDeviceContainer &devices)
// {
//   // Modulation and wifi channel bit rate
//   std::string phyMode("OfdmRate24Mbps");

//   // Fix non-unicast data rate to be the same as that of unicast
//   Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

//   WifiHelper wifi;
//   wifi.SetStandard(WIFI_PHY_STANDARD_80211a);

//   YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
//   wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

//   YansWifiChannelHelper wifiChannel;
//   wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
//   wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel",
//                                  "MaxRange", DoubleValue(19.0));
//   wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel",
//                                  "m0", DoubleValue(1.0),
//                                  "m1", DoubleValue(1.0),
//                                  "m2", DoubleValue(1.0));
//   wifiPhy.SetChannel(wifiChannel.Create());

//   // Add a non-QoS upper mac
//   NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default();
//   // Set it to adhoc mode
//   wifiMac.SetType("ns3::AdhocWifiMac");

//   // Disable rate control
//   wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
//                                "DataMode", StringValue(phyMode),
//                                "ControlMode", StringValue(phyMode));

//   devices = wifi.Install(wifiPhy, wifiMac, c);
// }

void
NdnV2VTest::InstallNDN(void)
{
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);

  // set Interest on CCH
  if (m_isInterestOnCch)
  {
    ndnHelper.setInterestOnCch();
  } 

  ndnHelper.Install(m_nodes);
  //ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/multicast");
  //ndn::StrategyChoiceHelper::InstallAll("/v2v", "/localhost/nfd/strategy/randomwait");

  ndn::StrategyChoiceHelper::InstallAll(m_namePrefix, m_strategy);

  ///todo add v2v face


}

void
NdnV2VTest::InstallConsumer(void)
{
  ndn::AppHelper cHelper("ns3::ndn::ConsumerCbr");
  cHelper.SetAttribute("Frequency", DoubleValue (1.0));
  cHelper.SetAttribute("MaxSeq", IntegerValue(1)); // For test: onlay send one interest
  cHelper.SetAttribute("Randomize", StringValue("uniform"));


  
  //string prefix = "/v2v/test/";
  //string prefix = m_namePrefix;
  
  
  // For start jitter
  // Ptr<UniformRandomVariable> rv = CreateObject<UniformRandomVariable>();
  // rv->SetAttribute("Min", DoubleValue(0.000));
  // rv->SetAttribute("Max", DoubleValue(0.500)); // 500 ms

  string strNodeId;
  // for (int k=0; k<m_nNodes; k++) {
  //     if(std::find(m_vecProducer.begin(), m_vecProducer.end(), k) == m_vecProducer.end())
  //     {
  //         strNodeId = std::to_string(k);
  //         string newPrefix = prefix;
  //         newPrefix.append(strNodeId);

  //         cHelper.SetPrefix(m_namePrefix);
          
  //         // No Producer installed
  //         ApplicationContainer consumerContainer = cHelper.Install(m_nodes.Get(k)); // each consumer app per node

  //         NS_LOG_INFO("Consumer installed on Node# " << k);

  //         Time startTime = Seconds(rv->GetValue());
  //         Ptr<Application> pConsumer = consumerContainer.Get(0);  // only one

  //         pConsumer->SetStartTime(startTime);  // start randomly between 0-1 second.

  //         NS_LOG_INFO("Schedule Consumer[" << k << "] start at " << startTime );

  //         // for test
  //         break;

  //     }

  //}
  // Node[0]: consumer observed application
  int k = 0;
  strNodeId = std::to_string(k);
  string newPrefix = m_namePrefix;
  newPrefix.append(strNodeId);

  cHelper.SetPrefix(newPrefix);
  ApplicationContainer app = cHelper.Install(m_nodes.Get(k));
  app.Start(Seconds(0.0));
  NS_LOG_INFO("Consumer installed on Node# " << k);


}

// Install a Producer at a random node
void
NdnV2VTest::InstallProducer(void)
{
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix(m_namePrefix);



  //  Ptr<UniformRandomVariable> randomNum = CreateObject<UniformRandomVariable> ();
  // int producerId = randomNum->GetValue(0, m_nNodes-1);

  // //Seclect randomly
  // for (int i=0; i<m_nProducer; i++)
  // {
  //     producerHelper.Install(m_nodes.Get(producerId));
  //     m_vecProducer.push_back(producerId);
  //     NS_LOG_INFO("Producer installed on node " << producerId);
      
  //     producerId = randomNum->GetValue(0, m_nNodes-1);
  // }
  producerHelper.Install(m_nodes.Get(m_nNodes-1)); // For test
  NS_LOG_INFO("Producer installed on node " << (m_nNodes-1));
  //NS_LOG_INFO("No Producer installed");

}

void
NdnV2VTest::InstallObserved(void)
{
  ndn::AppHelper cHelper("ns3::ndn::ConsumerCbr");
  cHelper.SetAttribute("Frequency", DoubleValue (10.0));
  cHelper.SetAttribute("MaxSeq", IntegerValue(20)); // For test: onlay send one interest
  cHelper.SetAttribute("Randomize", StringValue("uniform"));

  // The observed traffic
  // between [0] <--> [m_nNodes -1]
  int k = 0;
  string strNodeId = std::to_string(k);
  string newPrefix = m_namePrefix;
  newPrefix.append(strNodeId);

  cHelper.SetPrefix(newPrefix);
  ApplicationContainer app = cHelper.Install(m_nodes.Get(k));  // V[0]: consumer
  app.Start(Seconds(1.0));
  
  NS_LOG_INFO("Observed consumer installed on Node# " << k);

  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix(newPrefix);

  producerHelper.Install(m_nodes.Get(m_nNodes-1)); // V[N-1]: producer
  NS_LOG_INFO("Producer installed on node " << (m_nNodes-1));
}

void
NdnV2VTest::InstallBackground(int nPair, int frequency)
{
    assert(nPair >0);
    assert(nPair < m_nNodes);
    
    ndn::AppHelper cHelper("ns3::ndn::ConsumerCbr");
    cHelper.SetAttribute("Frequency", DoubleValue (frequency));
    //cHelper.SetAttribute("MaxSeq", IntegerValue()); // For test: onlay send one interest
    //cHelper.SetAttribute("Randomize", StringValue("uniform"));
    cHelper.SetAttribute("Randomize", StringValue("exponential"));

    ndn::AppHelper producerHelper("ns3::ndn::Producer");

    std::set<int> nodeSet; // set of candiate nodes
    // candiate set
    for (int i=1; i<m_nNodes-1; i++) {
        nodeSet.insert(i);
    }

    int pair = 0;
    std::set<int>::iterator it1, it2;
    while ((nodeSet.size() >= 2) && (pair<nPair)) {
        it1 = nodeSet.begin(); // the first

        int consumerId = *it1;
    
        //it2 = it1 + ;
        it2 = it1;
        int j = rand() % nodeSet.size();
        srand(time(NULL)); /*根据当前时间设置“随机数种子”*/
        while (j==0) j = rand() % nodeSet.size(); // avoid consumerId == producerId
        advance(it2, j);

        int producerId = *it2;

        // Construct prefix
        string strNodeId = std::to_string(consumerId);
        string newPrefix = m_namePrefix;
        newPrefix.append(strNodeId);

        // Install consumer
        cHelper.SetPrefix(newPrefix);
        cHelper.Install(m_nodes.Get(consumerId));
        NS_LOG_INFO("Background Consumer installed on Node# " << consumerId);

        producerHelper.SetPrefix(newPrefix);

        producerHelper.Install(m_nodes.Get(producerId));
        NS_LOG_DEBUG("Background producer installed on Node# " << producerId);

        pair ++;

        nodeSet.erase(it1);
        nodeSet.erase(it2); // delete installed nodes
    }

}

void
NdnV2VTest::Start(int nPairs, int frequency)
{
    CreateWaveNodes();

    InstallNDN();

    InstallObserved();

    InstallBackground(nPairs, frequency);

    StartWave();
    
}

void
NdnV2VTest::StartPush(int nPairs, int frequency)
{
    CreateWaveNodes();

    InstallNDN();

    InstallObservedPush();  // Push session as observed

    InstallBackground(nPairs, frequency);

    StartWave();   
}    

void
NdnV2VTest::InstallTrace(string file)
{
    ndn::AppDelayTracer::Install(m_nodes.Get(0), file); // observed only
}

void
NdnV2VTest::InstallObservedPush()
{
  // Push Producer
  std::string prefix = "/accident/G75";
  ndn::AppHelper pushProducerHelper("ns3::ndn::ProducerPush");
  //  pushProducerHelper.SetPrefix("/Accident");
  pushProducerHelper.SetAttribute("DataName", StringValue(prefix));
  pushProducerHelper.SetAttribute("EmergencyInd", StringValue("Emergency"));

  pushProducerHelper.SetAttribute("Frequency", DoubleValue(10.0));
  pushProducerHelper.SetAttribute("MaxSeq", IntegerValue(20)); // For test: onlay send one interest
  pushProducerHelper.SetAttribute("Randomize", StringValue("uniform"));
  
  ApplicationContainer pApp =  pushProducerHelper.Install(m_nodes.Get(m_nNodes-1)); //V_(N-1)
  
  pApp.Start(Seconds(0.0));  // Easy for computation

   // Install Consumers
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  consumerHelper.SetAttribute("MaxSeq", IntegerValue(0));  // request nothing
  consumerHelper.Install(m_nodes.Get(0));  // V_0
  
}
        


// ./waf --run ndn-v2v [strategy] [freq] [isPush]
// strategy: random-wait, multicast
// freq: background Interest frequency
// isPush: 0-false (native NDN): V_0 ---> V_(N-1); 1-true: V_(N-1) --> V_0 (no Interest sent by V_0)
// ./runAndDrawPush

int main (int argc, char *argv[])
{
  NS_LOG_UNCOND ("V2VTest Simulator");

  int simulationEnd = 5; // End time
  //int nNodes = 10; // total numer of nodes
  int nNodes = 10; // total numer of nodes
  

  string strategy = "multicast";
  int frequency = 10;

  bool onCCH = false; // whether sending Interest on CCH

  bool isPush = false; // default native NDN

  string appDelayFile = "app-delay.txt"; // default

  if ( (argc > 1) && (argc != 5) ) {
      cout << "Command argument not given enough!!!" <<endl;
      exit(0);
  }

  if (argc == 5) {
      strategy = argv[1];
 
      string strFreq = argv[2];
 
      frequency = atoi(strFreq.c_str());

      if (0 == strcmp(argv[3], "push")){
          isPush = true;
      }

      appDelayFile = argv[4]; 
  }
  
  cout << "Strategy = " << strategy << endl;
  cout << "Background Interest frequency = " << frequency << " interests/sec." << endl;

  if (onCCH) {
     cout << "Send Interest on WAVE CCH ... ..." << endl;
  }
  else {
     cout << "Send Interest on WAVE SCH (default)... ..." << endl;
  }

 

   // Set number of  nodes and producer 
  NdnV2VTest exp(nNodes, 1);

  exp.setNamePrefix("/V2V/test/");
 
  exp.setStrategy(strategy);

  // Set Interest on CCH?
  exp.setInterestOnCch(onCCH);

  if (isPush) {
     cout << "Observed session in Push model ... ..." << endl;

     // number of background sessions; background Interest frequency 
     exp.StartPush((nNodes-2)/2, frequency); 
  }
  else {
     cout << "Observed session in Pull model (native NDN) ... ..." << endl;
     // number of background sessions; background Interest frequency
     exp.Start((nNodes-2)/2, frequency); 
  }


  //
  
  Simulator::Stop(Seconds(simulationEnd));

  //std::string animFile = "v2v-test.xml";
  // std::string animFile = strategy + ".xml";
  //AnimationInterface anim(animFile);

  //string appDelayFile = "app-delay-" + strategy + ".txt";

  exp.InstallTrace(appDelayFile);


  //string rateTraceFile = "rate-trace-" + strategy + ".txt";
  //ndn::L3RateTracer::InstallAll(rateTraceFile, Seconds(0.01));
  
  Simulator::Run ();

  Simulator::Destroy();

  return 0;
}
} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
