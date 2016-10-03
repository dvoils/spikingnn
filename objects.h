#include "rains.h"

#include <stdlib.h>
#include <math.h>
#include <bitset>
#include <queue>
#include <iostream>
#include <sstream>

//#include <pthread.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
//#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

//#include "cv.h"
//#include "highgui.h"

#include<fstream>

// GUI communication is broken as of 
// Ubuntu 11.10. Don't have time or energy to debug it
//
#define USEGUI 0 // Use python gui
#define RAND_SPIKE 1 // Use random or rain spikes
#define THRESH 0.9

#define PORT 8888

#define NUM_RECS 64

#define DEBUG 1
#define MAX_DATA_WIDTH 64
#define MAX_MESH_SIZE 100 // number of digital fabric nodes
#define MAX_EDGES  MAX_MESH_SIZE*4
#define MAX_PIN_COUNT 1024 // number of pins in APP i/o buffer. This must synch with dfcon
#define MAX_PACKET_SIZE 24
#define MAX_LADDR_SIZE 16 // local address size in bits
#define MAX_CONNECT 1000 // max neural connections
#define MAX_BROADCAST 1000 // max broadcast distinations
#define MAX_CLOCK_COUNT 1000 // used for the timestamp
#define PIN_BITS 4 // Bits allocated to pin address. Used to make packet
#define MOD_BITS 7 // Bits allocated to module address. Used to make packet
#define TS_BITS 10 // Bits allocated to the time stamp. Used for logging
#define MAX_FIFO 50 // Global fifo depth
#define MAX_NIU_FIFO 100  //NIU fifo depth
using namespace std;



/*****************************************************************************

Simulation Supervisor

*****************************************************************************/
class Supervisor
{

	int String2Int(string);

	public:

	Supervisor();
	~Supervisor();
	
	int Print();
	int ReadFile(char *);
	int GetRecord(int,int);
};

/*****************************************************************************

GUI Buffer

*****************************************************************************/
class GUIBuffer
{


	queue<string> msg_q;

	pthread_mutex_t mutex1;


	public:
	int start; // user pressed start
	int stop; // user pressed stop
	int go;	//

	GUIBuffer();
	~GUIBuffer();

	string ReadQ();
	int WriteQ(int, int, int, int, int, int, int, int, int);
	int QEmpty();
	int QSize();
	int Done();

};


/*****************************************************************************

Thread Support

*****************************************************************************/
class Thread {
      private:
            pthread_t _id;
            // Prevent copying or assignment
            Thread(const Thread& arg);
            Thread& operator=(const Thread& rhs);
      protected:
            void *arg;
            static void *exec(void *thr);
      public:
            Thread();
            virtual ~Thread();
            void start(void *arg);
			void cancel();
            void join();
            virtual void run() = 0;
};

class Lock {
      protected:
            pthread_mutex_t mutex;
            // Prevent copying or assignment
            Lock(const Lock& arg);
            Lock& operator=(const Lock& rhs);
      public:
            Lock();
            virtual ~Lock();
            void lock();
            void unlock();
};


class MyThread : public Thread 
{

	int	serversock, clientsock;
	char buffer[256];
	GUIBuffer *gui_buffer;

      public:
	MyThread(GUIBuffer *);
	~MyThread();
            /*
              * This method will be executed by the Thread::exec method,
              * which is executed in the thread of execution
              */
            void run();
};



/*****************************************************************************
Random Number Generator
*****************************************************************************/
class RandGen
{

	public:
	RandGen();
	~RandGen();

	int SeedRandom(long);
	double Uniform();
	double Normal();
	

};

/*****************************************************************************
Main Channel

Main channel for the design. Contains signals common to all nodes.

*****************************************************************************/
class MainChannel
{

	public:
	int clk;
	int reset;

};

/*****************************************************************************
Top Channel

One of these channels is associated with each APP DFN pair. 

*****************************************************************************/
class TopChannel
{

	public:
	int clk;
	int reset;

	// APP
	int app_out_v;
	unsigned int app_out_data[MAX_PIN_COUNT];

};

/*****************************************************************************
Edge Channel

Used to connect two neighboring nodes

*****************************************************************************/
class EdgeChannel
{

	public:

	// channel 1
	int ch1_v;
	int ch1_a;
	unsigned int ch1_d;

	// channel 2
	int ch2_v;
	int ch2_a;
	unsigned int ch2_d;

};

/*****************************************************************************
DFN Channel

*****************************************************************************/
class DFNChannel
{
	public:

	// APP -> AIU interface
	int app_out_v;
	unsigned int app_out_data[MAX_PIN_COUNT];

	// AIU -> SPU interface
	int laddr_out_v;
	unsigned int laddr_out_d; // outgoing packet

	int laddr_in_v;
	unsigned int laddr_in_d; // incoming packet

	// SPU <-> Router interface
	int p_out_v;
	int p_out_a;
	unsigned int p_out_d; // outgoing packet from SPU

	int p_in_v;
	int p_in_a;
	unsigned int p_in_d; // incoming packet to SPU

	// Router <-> North NIU interface
	int n_out_v;
	int n_out_a;
	unsigned int n_out_d;

	int n_in_v;
	int n_in_a;
	unsigned int n_in_d;

	// Router <-> South NIU interface
	int s_out_v;
	int s_out_a;
	unsigned int s_out_d;

	int s_in_v;
	int s_in_a;
	unsigned int s_in_d;

	// Router <-> East NIU interface
	int e_out_v;
	int e_out_a;
	unsigned int e_out_d;

	int e_in_v;
	int e_in_a;
	unsigned int e_in_d;

	// Router <-> West NIU interface
	int w_out_v;
	int w_out_a;
	unsigned int w_out_d;

	int w_in_v;
	int w_in_a;
	unsigned int w_in_d;

};


/*****************************************************************************
Test Bench

*****************************************************************************/
class TestBench
{

	public:
	TestBench(MainChannel *);
	~TestBench();

	int *clk;
	int *reset;

	int Run();

};


/*****************************************************************************
APP Fifo

*****************************************************************************/
class APPFifo
{
	queue<int> myqueue;


	unsigned int data;


	int width;		// data width
	int size;		// fifo size

	unsigned int *din;
	unsigned int *dout;

	public:

	int read();
	int write();




  	int data_in;	// Data input
  	int data_out;	// Data Output



	APPFifo();
	~APPFifo();

	int run();	// run one clock cycle
};

/*****************************************************************************
Fifo

*****************************************************************************/
class Fifo
{
//	bitset<MAX_DATA_WIDTH> ram[MAX_FIFO_SIZE];
	queue<unsigned int> myqueue;

	unsigned int data;


	int width;		// data width
	int size;		// fifo size

	int read();
	int write();

	public:

	int clk;		// Clock input
   	int rst;		// Active high reset


  	int cs;			// chip select
  	int rd_en;		// Read enable
 	int wr_en;		// Write Enable

  	unsigned int data_in;	// Data input
  	unsigned int data_out;	// Data Output

  	int empty;		// FIFO empty
  	int full;		// FIFO full


	Fifo();
	~Fifo();

	int run();	// run one clock cycle

};

/*****************************************************************************
Ambric Register

*****************************************************************************/
class AmbricReg
{
	int clk;
	unsigned int data_reg[MAX_DATA_WIDTH];

	int EvalInput();
	int DriveOutput();

	public:

	int clock;

	AmbricReg();
	~AmbricReg();

	// input
	int in_valid;
	int tx_accept;

	// output
	int tx_valid;
	int rx_accept;

	unsigned int in_data[MAX_DATA_WIDTH];
	unsigned int tx_data[MAX_DATA_WIDTH];

	int Run();


};

/*****************************************************************************
Packet Router

*****************************************************************************/
class Router
{
	int north;
	int south;
	int east;
	int west;

	int CheckBounds(int, string);
	int CalcRoute();
	int HighPhase();
	int LowPhase();

	int meshsize; 

	DFNChannel *dfn_chan;

	public:
	Router(MainChannel *, DFNChannel *);
	~Router();

	int id;
	int rr;

	int mod_mask;
	int ts_mask;

	int *clk;
	int *reset;

	// SPU <-> Router interface
	int *p_out_v;
	int *p_out_a;
	unsigned int *p_out_d; // outgoing packet from SPU

	int *p_in_v;
	int *p_in_a;
	unsigned int *p_in_d; // incoming packet to SPU

	// North NIU <-> Router interface
	int *n_v;
	int *n_a;
	unsigned int *n_d; // outgoing packet

	// South NIU <-> Router interface
	int *s_v;
	int *s_a;
	unsigned int *s_d; // outgoing packet

	// East NIU <-> Router interface
	int *e_v;
	int *e_a;
	unsigned int *e_d; // outgoing packet

	// West NIU <-> Router interface
	int *w_v;
	int *w_a;
	unsigned int *w_d; // outgoing packet

	int Run();
};

/*****************************************************************************
Spike Packet Generator

*****************************************************************************/
class SPU
{
	queue<unsigned int> rxqueue;
	queue<unsigned int> txqueue;
	queue<unsigned int> pqueue; // used to stream multiple destinations

	// fifo depth counters
	int rx_count;
	int tx_count;
	int pq_count;

	// clock counter used to make timestamp. 
	unsigned int clcount;

	int MakePacket(int);
	int HighPhase();
	int LowPhase();

	struct addr
	{
		int mod;
		int pin;
	};

	struct addr_list
	{
		int size;
		addr list[MAX_BROADCAST];
	};

	addr_list addrl[MAX_PIN_COUNT];

	public:
	SPU(MainChannel *, DFNChannel *);
	~SPU();

	int id;

	int *clk;
	int *reset;

	// AIU <-> SPU interface
	int *laddr_out_v;
	unsigned int *laddr_out_d; // outgoing packet

	int *laddr_in_v;
	unsigned int *laddr_in_d; // incoming packet

	// SPU <-> Router interface
	int *p_out_v;
	int *p_out_a;
	unsigned int *p_out_d; // outgoing packet

	int *p_in_v;
	int *p_in_a;
	unsigned int *p_in_d; // incoming packet

	int LoadAddrList();
	int Run();
};

/*****************************************************************************
APP Interface Unit

*****************************************************************************/
class AIU
{
	APPFifo *RXFifo;
	APPFifo *TXFifo;

	// fifo drop counters
	int rx_count;
	int tx_count;

	int HighPhase();
	int LowPhase();

	int addr2pins(int); // used to drive address to SPU

	queue<int> rxqueue;
	queue<int> txqueue;

	public:
	AIU(TopChannel *, MainChannel *, DFNChannel *);
	~AIU();

	// Meta signals
	int id; // unit id
	int addr_list[MAX_PIN_COUNT];

	// Input
	int *clk;
	int *reset;

	int *in_valid;
	unsigned int *in_data[MAX_PIN_COUNT];

	// Registers
	int app_valid;
	unsigned int app_in_data[MAX_PIN_COUNT];

	// AIU -> SPU interface
	int *laddr_out_v;
	unsigned int *laddr_out_d; // outgoing packet

	int *laddr_in_v;
	unsigned int *laddr_in_d; // incoming packet

	int Run();


};


/*****************************************************************************
Channel Sample Detector

*****************************************************************************/
class CSD
{

	int HighPhase();
	int LowPhase();


	public:
	CSD();
	~CSD();

	int *clk;
	int *reset;

	int Run();
};


/*****************************************************************************
Node Interface Unit

*****************************************************************************/
class NIU
{

	queue<unsigned int> out_queue;
	queue<unsigned int> in_queue;
	
	// following used for logging
	unsigned int clk_count; // clock count
	unsigned int mod_mask;  // module mask
	unsigned int pin_mask;  // pin mask
	unsigned int ts_mask; // time stamp mask

	int HighPhase();
	int LowPhase();

	GUIBuffer *gbuff;

	public:
	NIU(MainChannel *, DFNChannel *, GUIBuffer *);
	~NIU();

	int node_id;
	int id;

	int term;

	int *clk;
	int *reset;

	// Router <-> NIU interface
	int *out_v;
	int *out_a;
	unsigned int *out_d;

	int *in_v;
	int *in_a;
	unsigned int *in_d;

	// NIU <-> Edge channel interface
	int *edge_out_v;
	int *edge_out_a;
	unsigned int *edge_out_d;

	int *edge_in_v;
	int *edge_in_a;
	unsigned int *edge_in_d;


	int Run();
};

/*****************************************************************************
Analog Pulse Processor

*****************************************************************************/
class APP
{
	int id;
	int one_shot;

	int EvalInput();
	int DriveOutput();
	int Connected[MAX_PIN_COUNT];

	float sim_time;


	queue<unsigned int> spike_train;

	RandGen *rnd;

	GUIBuffer *gbuff;
	RainsNet *myrain;

	int sp_source;
	float sp_time; 


	public:
	APP(int, TopChannel *, MainChannel *, GUIBuffer *);
	~APP();
	
	// input
	int *clk;
	int *reset;
	unsigned int *input[MAX_PIN_COUNT];

	// output
	int *valid;
	unsigned int *data[MAX_PIN_COUNT];

	int Run();
	int LoadAddrList();
};


/*****************************************************************************
Digital Fabric Node

*****************************************************************************/
class DFN 
{
	int id; // unit id;

	int init();
	int EvalInputs();
	int DriveOutputs();

  public:
    DFN (int, TopChannel *, MainChannel *, GUIBuffer *);
    ~DFN ();

	// modules
	DFNChannel *dfn_channel;

	AIU *APPIU;
	SPU *SPU1;
	Router *R1;
	NIU *NorthNIU;
	NIU *SouthNIU;
	NIU *EastNIU;
	NIU *WestNIU;

	int *clk;
	int *reset;

	int Run();
};


/*****************************************************************************
Digital Fabric Mesh

*****************************************************************************/
class Mesh
{
	int *clk;

	// channels
	MainChannel *MainChan;
	TopChannel *ChanList[MAX_MESH_SIZE];

	// modules
	TestBench *Bench;
	APP *ProcList[MAX_MESH_SIZE];
	DFN *NodeList[MAX_MESH_SIZE];
	EdgeChannel *EdgeList[MAX_EDGES];

	int term; // used to terminate edge connections.

	int ReadInputs();
	int DriveOutputs();
	int Print();
	int ConnectH(int, int, int);
	int ConnectV(int, int, int);

	public:
	Mesh(GUIBuffer *);
	~Mesh();

	int Run();
};



