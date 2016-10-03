#include "objects.h"

/*****************************************************************************
Digital Fabric Simulator
Portland State University
written by Danny Voils

*****************************************************************************/


int in_file_matrix[MAX_CONNECT][4];


/*****************************************************************************
Simulation Supervisor

*****************************************************************************/
Supervisor::Supervisor()
{
}

Supervisor::~Supervisor()
{

}

int Supervisor::String2Int(string mystring)
{
	int temp;

	istringstream buffer(mystring);

	buffer >> temp;

	return temp;
}

int Supervisor::GetRecord(int line, int rec)
{
	if ((rec > 3) || (line > MAX_CONNECT))
	{
		cout << "Error: record index too big" << endl;
		return -1;
	}
	else
	{
//		return in_matrix[line][rec];
	}

	return 0;
}

int Supervisor::Print()
{
	int i, j;

	for (i=0;i<MAX_CONNECT;i++)
	{
		for (j=0;j<4;j++)
		{

			cout << in_file_matrix[i][j] << " ";

		}
		cout << endl;
	}

	return 0;
}


int Supervisor::ReadFile(char mypath[])
{
 	int i;

	int found;
	int line_count;
	int sp; // string pointer

	string line, record;

  	ifstream myfile (mypath);

	line_count = 0;

	if (myfile.is_open())
	{
		while (! myfile.eof() )
		{
			
			sp = 0;
			found = 0;

			getline (myfile,line);

			found=line.find(":",0);	// get first occurance of ":"

//			cout << "file in debug: found " << found  << endl;

			if (found != -1)
			{
				record = line.substr(0,found); // get record before first ":"

//				cout << record << " ";

				sp = found+2; // skip : and " "

				for (i=0;i<4;i++)
				{
					found=line.find(" ",sp);	// get next occurance of " "

//					cout << "file in debug: sp " << sp << " found " << found << endl;

					record = line.substr(sp,found-sp); // get record between " "
					
					in_file_matrix[line_count][i] = String2Int(record);

					sp = found+1;
				}
//				cout << endl;

				line_count++;
			}
			
		}
	}

	return 0;
}

/*****************************************************************************

GUI Buffer

*****************************************************************************/
GUIBuffer::GUIBuffer()
{
	pthread_mutex_init(&mutex1, NULL);
	
	start = 0;
	stop = 0;
	go = 0;
}

GUIBuffer::~GUIBuffer()
{
	pthread_mutex_destroy(&mutex1);
}

string GUIBuffer::ReadQ()
{
	string result;

	pthread_mutex_lock(&mutex1);
	result = msg_q.front();
	msg_q.pop();
	pthread_mutex_unlock(&mutex1);


	return result;
}

int GUIBuffer::WriteQ(int clock, int id, int niu, int fifo, int tstamp, int src_mod, int dst_mod, int dst_pin, int event)
{
	string message = "";

	string myclock, myid, myniu, myfifo, mytstamp, mysrc_mod, mydst_mod, mydst_pin, myevent;
	stringstream out1, out2, out3, out4, out5, out6, out7, out8, out9;


	// convert ints to strings
	out1 << clock;
	myclock = out1.str();

	out2 << id;
	myid = out2.str();

	out3 << niu;
	myniu = out3.str();

	out4 << fifo;
	myfifo = out4.str();

	out5 << tstamp;
	mytstamp = out5.str();

	out6 << src_mod;
	mysrc_mod = out6.str();

	out7 << dst_mod;
	mydst_mod = out7.str();

	out8 << dst_pin;
	mydst_pin = out8.str();

	out9 << event;
	myevent = out9.str();

	message = myclock + " " + myid + " " + myniu + " " + myfifo + " " + mytstamp + " " + mysrc_mod + " " + mydst_mod + " " + mydst_pin + " " + myevent + " ";

//	cout << "debug writeq: message: " << message << endl;

	pthread_mutex_lock(&mutex1);
	msg_q.push(message);
	pthread_mutex_unlock(&mutex1);
	return 0;
}

int GUIBuffer::QEmpty()
{
	if (msg_q.empty())
		return 1;
	else
		return 0;
}

int GUIBuffer::QSize()
{

	return msg_q.size();
}

int GUIBuffer::Done()
{
	string message = "done";

	pthread_mutex_lock(&mutex1);
	msg_q.push(message);
	pthread_mutex_unlock(&mutex1);
	return 0;
}

/*****************************************************************************

Thread Support

*****************************************************************************/

Thread::Thread() {
  //    cout << "Thread::Thread()" << endl;
}
Thread::~Thread() {
 //     cout << "Thread::~Thread()" << endl;
}
void Thread::start(void *arg) {
      int ret;
      this->arg = arg;
      /*
        * Since pthread_create is a C library function, the 3rd argument is
        * a global function that will be executed by the thread. In C++, we
        * emulate the global function using the static member function that
        * is called exec. The 4th argument is the actual argument passed to
        * the function exec. Here we use this pointer, which is an instance
        * of the Thread class.
        */
      if ((ret = pthread_create(&_id, NULL, &Thread::exec, this)) != 0) {
             cout  << endl;
             throw "Error";
      }

	/* make this thread cancellable using pthread_cancel() */
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
}

void Thread::cancel() 
{
	pthread_cancel(_id);
}

void Thread::join() {
      // Allow the thread to wait for the termination status
      pthread_join(_id, NULL);
}

// Function that is to be executed by the thread
void *Thread::exec(void *thr) {
      reinterpret_cast<Thread *> (thr)->run();
	return 0;
}

/*****************************************************************************

Mutex Lock

*****************************************************************************/

Lock::Lock() {
      pthread_mutex_init(&mutex, NULL);
}
Lock::~Lock() {
      pthread_mutex_destroy(&mutex);
}
void Lock::lock() {
      pthread_mutex_lock(&mutex);
}
void Lock::unlock() {
      pthread_mutex_unlock(&mutex);
}


/*****************************************************************************

Network Server Thread

*****************************************************************************/
MyThread::MyThread(GUIBuffer *gbuff)
{
	gui_buffer = gbuff;

	struct sockaddr_in server;


	/* open socket */
	if ((serversock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		//	quit("socket() failed", 1);
		cout << "Error can't open socket." << endl;
		return;
	}

	// Sockets are typically kept in a kind of limbo for a minute or two 
    // after you've finished listening on them to prevent communications intended for the 
    // previous process coming to yours. It's called the 'TIME_WAIT' state.

    // If you want to override that behaviour use setsockopt to set the SO_REUSEADDR flag 
    // against the socket before listening on it.

	int one = 1;
	setsockopt(serversock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

	/* setup server's IP and port */
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = INADDR_ANY;


	/* bind the socket */
	if (bind(serversock, (struct sockaddr *) &server, sizeof(server)) < 0) 
	{
		cout << "Error can't bind socket." << endl;
		return;
	}

	/* wait for connection */
	if (listen(serversock, 10) == -1) {
		cout << "Error: listen failed" << endl;
		return;
	}

	cout << "Digital Fabric Simulator v1.0" << endl;
	cout << "Waiting for client" << endl;

	/* accept a client */
	// This call loops until a client is accepted
	if ((clientsock = accept(serversock, NULL, NULL)) == -1) {
		cout << "Error: can't accept client" << endl;
		return;
	}
	else
	{
		cout << "Client Accepted " << endl;
	}
}


MyThread::~MyThread()
{

	cout << "Closing socket " << endl;
	close(clientsock);
	close(serversock);

}
/*
	This is a worker thread which runs outside the main program loop. If this thread
	is blocked, the main loop will continue.
*/
void MyThread::run()
{
	int i;

	cout << "running a thread" << endl;

	while(1) 
	{

//		cout << "debug waiting for start" << endl;
		if ((read(clientsock,buffer,255) == -1))
		{
			cout << "Error: can't read client data" << endl;
			return;
		}
		else
		{
//			cout << "k: " << k << " message is: " << buffer << endl;
			if (strncmp(buffer,"test1",5) == 0)
			{
				cout << "debug got start" << endl;
				// informs the main program start received 
				gui_buffer->start = 1;

				// Wait for the main program to run the simulation and 
                // the output Q has data to transmit
				while (gui_buffer->QEmpty() == 1)
				{

				}

				while (gui_buffer->QEmpty() == 0)
				{
//					cout << "reading from q: " << gui_buffer->QSize() << endl;
					string temp = gui_buffer->ReadQ();

					char *cstr;
					cstr = new char [256];

					strcpy (cstr, temp.c_str());

					for (i=0;i<256;i++)
					{
						buffer[i] = cstr[i];
					}

					if ((write(clientsock,buffer,256) == -1))
					{
						cout << "Error: can't write client data" << endl;
					}


//					cout << "debug waiting for next" << endl;
					if ((read(clientsock,buffer,255) == -1))
					{
						cout << "Error: can't read client data" << endl;
						return;
					}
				}
			} // if

			else if (strncmp(buffer,"stop",4) == 0)
			{
				gui_buffer->stop = 1;
			}

		} // else


	} // while

//	usleep(1);
//	cout << "debug waiting here 4" << endl;
}



/*****************************************************************************
Random Number Generator

to do: Using crappy rand() for now, need to improve this.

*****************************************************************************/
RandGen::RandGen()
{

}

RandGen::~RandGen()
{

}

int RandGen::SeedRandom(long seed)
{
 //	srand ( time(NULL) );
	return 0;
}

double RandGen::Uniform()
{

	return rand()/(double)RAND_MAX;
}

double RandGen::Normal()
{
	return 0;
} 

/*****************************************************************************
Test Bench

*****************************************************************************/
TestBench::TestBench(MainChannel *m_channel)
{

	clk = &m_channel->clk;
	reset = &m_channel->reset;
	*clk = 0;

}

TestBench::~TestBench()
{


}

int TestBench::Run()
{
	if (*clk==0) 
	{
//		cout << "Start new clock period" << endl;
		*clk = 1;
	}
	else *clk = 0;

//	cout << "debug " << *clk << endl;

	return 0;
}

/*****************************************************************************
Ambric Register

*****************************************************************************/
AmbricReg::AmbricReg()
{

}

AmbricReg::~AmbricReg()
{


}

int AmbricReg::Run()
{

	return 0;
}

/*****************************************************************************
APP Fifo

*****************************************************************************/
APPFifo::APPFifo()
{

}

APPFifo::~APPFifo()
{

}

int APPFifo::read()
{
	data_out = myqueue.front();
	myqueue.pop();

	return 0;
}

int APPFifo::write()
{
	myqueue.push (data_in);

	return 0;
}

int APPFifo::run()
{

	return 0;
}


 
/*****************************************************************************
DFN Fifo

*****************************************************************************/
Fifo::Fifo()
{

}

Fifo::~Fifo()
{

}

int Fifo::read()
{
	data_out = myqueue.front();
	myqueue.pop();

	return 0;
}

int Fifo::write()
{
	myqueue.push (data_in);

	return 0;
}

int Fifo::run()
{

	return 0;
}

/*****************************************************************************
Packet Router

*****************************************************************************/
Router::Router(MainChannel *m_channel, DFNChannel *dfn_channel)
{
	clk = &m_channel->clk;
	reset = &m_channel->reset;

	p_out_v = &dfn_channel->p_out_v;
	p_out_a = &dfn_channel->p_out_a;
	p_out_d = &dfn_channel->p_out_d; // outgoing packet from SPU

	p_in_v = &dfn_channel->p_in_v;
	p_in_a = &dfn_channel->p_in_a;
	p_in_d = &dfn_channel->p_in_d; // incominging packet to SPU

	dfn_chan = dfn_channel;

	meshsize = 9;
	rr = 0;

	// create mask for module selection
	int i;
	int n;

	n = 1;
	mod_mask = 0;
	for (i=0;i<MOD_BITS;i++)
	{
		mod_mask = mod_mask | n;
		n = n << 1;
	}

	n = 1;

	ts_mask = 0;
	for (i=0;i<TS_BITS;i++)
	{
		ts_mask = mod_mask | n;
		n = n << 1;
	}

//	mod_mask = 255;

//	cout << "mod_mask " << mod_mask << endl;
}

Router::~Router()
{
}

int Router::CheckBounds(int mod, string dir)
{

	return 0;
}

int Router::CalcRoute()
{

	return 0;
}

int Router::HighPhase()
{

	int p_v = *p_out_v;
	int p_d = *p_out_d;

	int src_mod;
	int dst_mod;
	int tstamp;
	int dist;
	int row, col;
	int drow, dcol;
	int srow, scol;

	unsigned int data;
	int in_valid = 0;

	north = 0;
	south = 0;
	east = 0;
	west = 0;


	dfn_chan->n_out_v = 0;
	dfn_chan->s_out_v = 0;
	dfn_chan->e_out_v = 0;
	dfn_chan->w_out_v = 0;

	// destination module
	dst_mod = (p_d >> PIN_BITS) & mod_mask;

	// this node's row and column
	row = floor(id/10);
	col = id%10;
	
	// round robbin selection
	rr++;
	if (rr == 5) rr = 0;	
	//		cout << " debug a *********************************** " << rr << endl;

	switch(rr)
	{
		case 0:
//			cout << " debug1 *********************************** " << endl;
        	*n_a = 1;
			*s_a = 0;
			*e_a = 0;
			*w_a = 0;
            break;

        case 2:
//			cout << " debug2 *********************************** " << endl;
        	*n_a = 0;
			*s_a = 1;
			*e_a = 0;
			*w_a = 0;
            break;

        case 3:
//			cout << " debug3 *********************************** " << endl;
        	*n_a = 0;
			*s_a = 0;
			*e_a = 1;
			*w_a = 0;
            break;

        case 4:
//			cout << " debug4 *********************************** " << endl;
        	*n_a = 0;
			*s_a = 0;
			*e_a = 0;
			*w_a = 1;
            break;

	}
	
	if (*n_v == 1)
	{
		data = *n_d;
		dst_mod = (data >> PIN_BITS)  &  mod_mask;
		in_valid = 1;
		cout << "Node " << id << ": Router: north valid, destination module: " << dst_mod << " data " << data << endl;
	}

	if (*s_v == 1)
	{
		data = *s_d;
		dst_mod = (data >> PIN_BITS)  &  mod_mask;
		in_valid = 1;
		cout << "Node " << id << ": Router: south valid, destination module: " << dst_mod << " data " << data << endl;
	}

	if (*e_v == 1)
	{
		data = *e_d;
		dst_mod = (data >> PIN_BITS)  &  mod_mask;
		in_valid = 1;
		cout << "Node " << id << ": Router: east valid, destination module: " << dst_mod << " data " << data << endl;
	}

	if (*w_v == 1)
	{
		data = *w_d;
		dst_mod = (data >> PIN_BITS)  &  mod_mask;
		in_valid = 1;
		cout << "Node " << id << ": Router: west valid, destination module: " << dst_mod << " data " << data << endl;
	}


	if ((p_v == 1) && (in_valid ==0))
	{
		cout << "Node " << id << ": Router: APP packet valid, destination module: " << dst_mod << " data " << p_d << endl;
		data = p_d;
		in_valid = 1;
	}

	// destination row and colum
	drow = floor(dst_mod/10);
	dcol = dst_mod%10;



//	cout << "debug router 1" << endl;
	// routing algorithm
	if ((in_valid == 1)&&(id == dst_mod))
	{

		dst_mod = (data >> (PIN_BITS))  &  mod_mask;
		src_mod = (data >> (MOD_BITS + PIN_BITS))  &  mod_mask;
		tstamp =  (data >> (MOD_BITS + MOD_BITS + PIN_BITS))  &  ts_mask;

		srow = floor(src_mod/10);
		scol = src_mod%10;
		dist = abs(abs(srow-drow) + abs(scol-dcol));


		cout << "Node " << id << ": Router: retire packet" << " source module: " << src_mod << " time stamp: " << tstamp << " distance: " << dist  << endl;
		dfn_chan->p_in_v = 1;
		dfn_chan->p_in_d = data;

	}
	
	
	else if (in_valid == 1)
	{
		if (row == drow)
		{
			if (dcol > col)
			{
				if (abs(dcol - col) == meshsize)
				{
					cout << "Node " << id << ": Router: sending packet west, wrap, row == drow" << endl;
					dfn_chan->w_out_v = 1;
					dfn_chan->w_out_d = data;
				}
				else
				{
					cout << "Node " << id << ": Router: sending packet east" << endl;
					dfn_chan->e_out_v = 1;
					dfn_chan->e_out_d = data;
				}
			}
			else
			{
				if (abs(dcol - col) == meshsize)
				{
					cout << "Node " << id << ": Router: sending packet east, wrap, row == drow" << endl;
					dfn_chan->e_out_v = 1;
					dfn_chan->e_out_d = data;
				}
				else
				{
					cout << "Node " << id << ": Router: sending packet west" << endl;
					dfn_chan->w_out_v = 1;
					dfn_chan->w_out_d = data;
				}
			}
			return 0;
		}

//cout << "debug router 2" << endl;
		if (col == dcol)
		{
			if (drow < row)
			{
				if (abs(drow - row) == meshsize)
				{
					cout << "Node " << id << ": Router: sending packet south loop" << endl;
					dfn_chan->s_out_v = 1;
					dfn_chan->s_out_d = data;
				}
				else
				{
					cout << "Node " << id << ": Router: sending packet north" << endl;
					dfn_chan->n_out_v = 1;
					dfn_chan->n_out_d = data;
				}
			}
			else
			{
				if (abs(drow - row) == meshsize)
				{
					cout << "Node " << id << ": Router: sending packet north loop" << endl;
					dfn_chan->n_out_v = 1;
					dfn_chan->n_out_d = data;
				}
				else
				{				
					cout << "Node " << id << ": Router: sending packet south" << endl;
					dfn_chan->s_out_v = 1;
					dfn_chan->s_out_d = data;
				}
			}
			return 0;
		}	
//cout << "debug router 1" << endl;

		if (row < drow)
		{
			// lower right
			if (col < dcol)
			{

				if (abs(drow - row) == meshsize)
				{
					cout << "Node " << id << ": Router: sending packet north loop" << endl;
					dfn_chan->n_out_v = 1;
					dfn_chan->n_out_d = data;
				}
				else
				{
					cout << "Node " << id << ": Router: sending packet south" << endl;
					south = 1;
					dfn_chan->s_out_v = 1;
					dfn_chan->s_out_d = data;
				}
			}

			// lower left
			else 
			{
				if (abs(dcol - col) == meshsize)
				{
					cout << "Node " << id << ": Router: sending packet east, wrap" << endl;
					dfn_chan->e_out_v = 1;
					dfn_chan->e_out_d = data;
				}
				else
				{
					cout << "Node " << id << ": Router: sending packet west" << endl;
					west = 1;
					dfn_chan->w_out_v = 1;
					dfn_chan->w_out_d = data;
				}
			}
			return 0;
		}

		else
		{
			// upper right
			if (col < dcol)
			{
				if (abs(dcol - col) == meshsize)
				{
					cout << "Node " << id << ": Router: sending packet west wrap" << endl;
					dfn_chan->w_out_v = 1;
					dfn_chan->w_out_d = data;
				}
				else
				{
					cout << "Node " << id << ": Router: sending packet east" << endl;
					east = 1;
					dfn_chan->e_out_v = 1;
					dfn_chan->e_out_d = data;
				}

			}

			// upper left
			else 
			{

				if (abs(drow - row) == meshsize)
				{
					cout << "Node " << id << ": Router: sending packet south loop" << endl;
					dfn_chan->s_out_v = 1;
					dfn_chan->s_out_d = data;
				}
				else
				{
					cout << "Node " << id << ": Router: sending packet north" << endl;
					north = 1;
					dfn_chan->n_out_v = 1;
					dfn_chan->n_out_d = data;
				}

			}
		}
	}

	return 0;
}

int Router::LowPhase()
{

	return 0;
}

int Router::Run()
{

	if (*clk == 1)
	{
		HighPhase();
	}
	else
	{
		LowPhase();
	}

	return 0;
}

/*****************************************************************************
Spike Packet Generator

*****************************************************************************/
SPU::SPU(MainChannel *m_channel, DFNChannel *dfn_channel)
{

//	RandGen *rand;

//	rand = new RandGen;

//	rand->SeedRandom(0);

	// fifo depth counters
	rx_count = 0;
	tx_count = 0;
	pq_count = 0;

	clk = &m_channel->clk;
	reset = &m_channel->reset;

	// AIU -> SPU interface
	laddr_out_v = &dfn_channel->laddr_out_v;
	laddr_out_d = &dfn_channel->laddr_out_d; // outgoing packet

	laddr_in_v = &dfn_channel->laddr_in_v;
	laddr_in_d = &dfn_channel->laddr_in_d; // outgoing packet

	// SPU <-> Router interface
	p_out_v = &dfn_channel->p_out_v;
	p_out_a = &dfn_channel->p_out_a;
	p_out_d = &dfn_channel->p_out_d; // outgoing packet

	p_in_v = &dfn_channel->p_in_v;
	p_in_a = &dfn_channel->p_in_a;
	p_in_d = &dfn_channel->p_in_d; // incoming packet

}

SPU::~SPU()
{

}

int SPU::LoadAddrList()
{
	int i,j;
//	int temp;
	int src_pin, dst_mod, dst_pin;
	int pcount;

//	cout << "node id: " << id << endl;
	// Search through entier connection list. Find connecitons corresponding to me. 
	// These get loaded into the node's 'local memory'.
	for (i=0;i<MAX_CONNECT;i++)
	{
		if (id == in_file_matrix[i][0])
		{
	
			for (j=0;j<4;j++)
			{
//				cout << in_file_matrix[i][j] << " ";

			}
//			cout << endl;
			src_pin = in_file_matrix[i][1];
			dst_mod = in_file_matrix[i][2];
			dst_pin = in_file_matrix[i][3];

			pcount = addrl[src_pin].size;

			addrl[src_pin].list[pcount].mod = dst_mod;
			addrl[src_pin].list[pcount].pin = dst_pin;

			int temp = addrl[src_pin].list[pcount].pin;

			cout << "debug: SPU load, id " << id << " source pin " << src_pin << " dest mod " << dst_mod << " dest pin " << dst_pin << " pcount " << pcount << " source pin read " << temp << endl;

			pcount++;
			addrl[src_pin].size = pcount;

		}
	}


	return 0;
}

int SPU::MakePacket(int myaddr)
{
	int i;
	unsigned int result;

	int pcount;
	unsigned int dst_mod, dst_pin;

	unsigned int pin_mask;
	unsigned int mod_mask;
	unsigned int ts_mask;

//	int dst_pin;
//	int dst_mod;
	int src_mod;
//	int tstamp;

	// create masks for logging
//	int i;
	int n;

	n = 1;
	pin_mask = 0;
	for (i=0;i<PIN_BITS;i++)
	{
		pin_mask = pin_mask | n;
		n = n << 1;
	}

	n = 1;
	mod_mask = 0;
	for (i=0;i<MOD_BITS;i++)
	{
		mod_mask = mod_mask | n;
		n = n << 1;
	}

	n = 1;
	ts_mask = 0;
	for (i=0;i<TS_BITS;i++)
	{
		ts_mask = mod_mask | n;
		n = n << 1;
	}




	pcount = addrl[myaddr].size;

	if (pcount < 1)
	{
		cout << "Node " << id << " Error: local address not connected. " << myaddr << endl;
		return 0;

	}
	else
	{
//	cout << "Node " << id << ": SPU local address " << myaddr << endl;
	}

	cout << "Node " << id << ": SPU count " << pcount << endl;

//	pcount--;

	for (i=0;i<pcount;i++)
	{
		dst_pin = addrl[myaddr].list[i].pin;
		dst_mod = addrl[myaddr].list[i].mod;

		result = 0;
		// make packet
		result = (id << (MOD_BITS + PIN_BITS)) | 
			(dst_mod << PIN_BITS) | 
			dst_pin;

	//	cout << "packet: " << " dst_pin " << dst_pin << " dst_mod " << dst_mod << " result " << result << endl;
	//	cout << "Node " << id << ": SPU addr packetq push " << myaddr << " mod " << dst_mod << " pin " << dst_pin << " result " << result << endl;

		dst_pin = result & pin_mask;
		dst_mod = (result >> (PIN_BITS))  &  mod_mask;
		src_mod = (result >> (MOD_BITS + PIN_BITS))  &  mod_mask;

		cout << "Node " << id << ": spu_packet Q write " <<
				"dst pin " << dst_pin << 
				", dst mod " << dst_mod <<
				", src mod " << src_mod << endl;

		pqueue.push(result);

		if (pqueue.size() >= MAX_FIFO)
		{
			pqueue.front();
			pqueue.pop();
			cout << "Node " << id << ": SPU packet Q drop " << pqueue.size() << endl;
		}

	}
	return 0;
}

int SPU::HighPhase()
{
//	int i, j;
//	int dst_pin, dst_mod, pcount;

//	cout << "Node " << id << ": SPU " << endl;
	unsigned int loc_addr;
	int loc_addr_v;
	int txqsize, pqsize;
	unsigned int packet;

	unsigned int pin_mask;
	unsigned int mod_mask;
	unsigned int ts_mask;

	int dst_pin;
	int dst_mod;
	int src_mod;
	int tstamp;
	unsigned int data;

	// create masks for logging
	int i;
	int n;

	n = 1;
	pin_mask = 0;
	for (i=0;i<PIN_BITS;i++)
	{
		pin_mask = pin_mask | n;
		n = n << 1;
	}

	n = 1;
	mod_mask = 0;
	for (i=0;i<MOD_BITS;i++)
	{
		mod_mask = mod_mask | n;
		n = n << 1;
	}

	n = 1;
	ts_mask = 0;
	for (i=0;i<TS_BITS;i++)
	{
		ts_mask = mod_mask | n;
		n = n << 1;
	}



	clcount++;
	if (clcount > MAX_CLOCK_COUNT)
	{
		clcount = 0;
	}

	loc_addr_v = *laddr_out_v;
	loc_addr = *laddr_out_d;

	txqsize = txqueue.size();
	pqsize = pqueue.size();
	

/*
	for (i=0;i<MAX_PIN_COUNT;i++)
	{
		pcount = addrl[i].size;

		if (pcount > 0)
		{
			dst_pin = addrl[i].list[pcount-1].pin;
			dst_mod = addrl[i].list[pcount-1].mod;

//			cout << "Node " << id << ": SPU addr mod " << dst_mod << " pin " << dst_pin << endl;
		}

	}
*/


	if (loc_addr_v == 1)
	{
//		cout << "Node " << id << ": SPU laddr valid " << loc_addr <<endl;
		packet = MakePacket(loc_addr);
//		cout << "Node " << id << ": SPU txqueue write " << packet << endl;
	}

	// Incoming spike gets sent to AIU
	if (*p_in_v == 1)
	{
			*laddr_in_v = 1;
			packet = *p_in_d;
			dst_pin = packet & pin_mask;
			*laddr_in_d = dst_pin;


		data = packet;

		dst_pin = data & pin_mask;
		dst_mod = (data >> (PIN_BITS))  &  mod_mask;
		src_mod = (data >> (MOD_BITS + PIN_BITS))  &  mod_mask;
		tstamp =  (data >> (MOD_BITS + MOD_BITS + PIN_BITS))  &  ts_mask;

		cout << 
				"clk " << clcount << 
				", Node " << id << 
				", SPU in " << id << 
				", ts " << tstamp <<
				", src mod " << src_mod <<
				", dst mod " << dst_mod <<
				", dst pin " << dst_pin <<
				endl;



	}

	// grab next packet from packet Q and put into transmit Q. Add time stamp
	if (pqsize > 3)
	{
		packet = 0;

		packet = pqueue.front();
		pqueue.pop();

		packet = packet | (clcount << (MOD_BITS + MOD_BITS + PIN_BITS));


		txqueue.push(packet);

		if (txqueue.size() >= MAX_FIFO)
		{
			txqueue.front();
			txqueue.pop();
			cout << "Node " << id << ": SPU TX Q drop" << endl;
		}

		dst_pin = packet & pin_mask;
		dst_mod = (packet >> (PIN_BITS))  &  mod_mask;
		src_mod = (packet >> (MOD_BITS + PIN_BITS))  &  mod_mask;
		tstamp =  (packet >> (MOD_BITS + MOD_BITS + PIN_BITS))  &  ts_mask;

		cout << "Node " << id << ": SPU txqueue_write " <<
				"clcount " << clcount << 
				", dst pin " << dst_pin << 
				", dst mod " << dst_mod <<
				", src mod " << src_mod <<
				", tstamp " << tstamp << endl;
	}

	if (txqsize > 0)
	{
		*p_out_v = 1;
		packet = 0;
		packet = txqueue.front();
		*p_out_d = packet;
		txqueue.pop();
//		cout << "Node " << id << ": SPU txqueue read " << packet << endl;

		dst_pin = packet & pin_mask;
		dst_mod = (packet >> (PIN_BITS))  &  mod_mask;
		src_mod = (packet >> (MOD_BITS + PIN_BITS))  &  mod_mask;
		tstamp =  (packet >> (MOD_BITS + MOD_BITS + PIN_BITS))  &  ts_mask;

		cout << "Node " << id << ": SPU txqueue read " <<
				"dst pin " << dst_pin << 
				", dst mod " << dst_mod <<
				", src mod " << src_mod <<
				", tstamp " << tstamp << endl;


	}
	else
	{
		*p_out_v = 0;

	}

	return 0;
}

int SPU::LowPhase()
{


	return 0;
}

int SPU::Run()
{

	if (*clk == 1)
	{
		HighPhase();
	}
	else
	{
		LowPhase();
	}

	return 0;
}

/*****************************************************************************
APP Interface Unit

*****************************************************************************/
AIU::AIU(TopChannel *top_channel, MainChannel *m_channel, DFNChannel *dfn_channel)
{
	int i;

	CSD *csd_list[MAX_PIN_COUNT];

	rx_count = 0;

	// connect inputs
	clk = &m_channel->clk;
	reset = &m_channel->reset;

	in_valid = &top_channel->app_out_v;

	for (i=0;i<MAX_PIN_COUNT;i++)
	{
		csd_list[i] = new CSD();
		csd_list[i]->clk = clk;
		in_data[i] = &top_channel->app_out_data[i];

	}

	// connect outputs
	laddr_out_v = &dfn_channel->laddr_out_v;
	laddr_out_d = &dfn_channel->laddr_out_d;

	// Generate APP pin local addresses.
	for (i=0;i<MAX_PIN_COUNT;i++)
	{
		addr_list[i] = i;
	}

	

}

AIU::~AIU()
{

}

int AIU::addr2pins(int addr)
{


	return 0;
}


int AIU::HighPhase()
{
	int i;
	int temp;
	int pin_address;

	temp = *in_valid;

//	cout << "AIU run " << temp << endl;

	// note: add reset

	// latch receive data valid
	app_valid = *in_valid;

	if (*in_valid == 1)
	{

		// latch in data from APP
		for (i=0;i<MAX_PIN_COUNT;i++)
		{
	//		cout << "debug AIU: " << i << endl;
			app_in_data[i] = *in_data[i];
		}

		// Write all active pin addresses to receive address
		// FIFO in one clock. This probably would not happen 
		// in real hardware. However, the APP is unlikely to 
		// generate more than one output spike per system clock. 
		for (i=0;i<MAX_PIN_COUNT;i++)
		{
			if (app_in_data[i] == 1)
			{

				cout << "Node " << id << ": AIU rxqueue write " << addr_list[i] << endl;
				rxqueue.push(addr_list[i]);

				if (rxqueue.size() >= MAX_FIFO)
				{
					rxqueue.front();
					rxqueue.pop();
					cout << "Node " << id << ": AIU RX Q drop" << endl;
				}
//				RXFifo->data_in = addr_list[i];
//				RXFifo->write();
			}
		}

	}
	else
	{
			*laddr_out_v = 0;
			// Read from fifo when it's not being written to.
			if (rxqueue.size() > 0)
			{				
				*laddr_out_v = 1;
				pin_address = rxqueue.front();
				*laddr_out_d = pin_address;
				rxqueue.pop();

				cout << "Node " << id << ": AIU rxqueue read " << pin_address << endl;

			}

	}

	
	return 0;
}

int AIU::LowPhase()
{

	return 0;
}

int AIU::Run()
{
	int temp;


	temp = *clk;
//	cout << "AIU->clk " << temp << endl;

	if (*clk == 1)
	{
		HighPhase();
	}
	else
	{
		LowPhase();
	}

	return 0;
}

/*****************************************************************************
Channel Spike Detector

*****************************************************************************/
CSD::CSD()
{

}


CSD::~CSD()
{

}

int CSD::HighPhase()
{
	return 0;
}

int CSD::LowPhase()
{
	return 0;
}


int CSD::Run()
{
	int temp;


	temp = *clk;
//	cout << "AIU->clk " << temp << endl;

	if (*clk == 1)
	{
		HighPhase();
	}
	else
	{
		LowPhase();
	}

	return 0;
}

/*****************************************************************************
Node Interface Unit

*****************************************************************************/
NIU::NIU(MainChannel *m_channel, DFNChannel *dfn_channel, GUIBuffer *g_buffer)
{
	// connect inputs
	clk = &m_channel->clk;
	reset = &m_channel->reset;

	gbuff = g_buffer;

	clk_count = 0;

	// create masks for logging
	int i;
	int n;

	n = 1;
	pin_mask = 0;
	for (i=0;i<PIN_BITS;i++)
	{
		pin_mask = pin_mask | n;
		n = n << 1;
	}

	n = 1;
	mod_mask = 0;
	for (i=0;i<MOD_BITS;i++)
	{
		mod_mask = mod_mask | n;
		n = n << 1;
	}

	n = 1;
	ts_mask = 0;
	for (i=0;i<TS_BITS;i++)
	{
		ts_mask = mod_mask | n;
		n = n << 1;
	}

}

NIU::~NIU()
{

}

int NIU::HighPhase()
{
	int size;
	int in_size;
	int out_size;
	int gui_id;
	unsigned int data;

	int src_mod;
	int dst_mod;
	int dst_pin;
	int tstamp;

	clk_count++;
	
//	cout << "Node " << node_id << ", NIU " << id << ": debug gpf" << data << endl;

	*edge_out_v = 0;
	*in_v = 0;

	// output valid from router
	if (*out_v == 1)
	{
		data = *out_d;

		dst_pin = data & pin_mask;
		dst_mod = (data >> (PIN_BITS))  &  mod_mask;
		src_mod = (data >> (MOD_BITS + PIN_BITS))  &  mod_mask;
		tstamp =  (data >> (MOD_BITS + MOD_BITS + PIN_BITS))  &  ts_mask;

		cout << "clk " << clk_count << 
				", module " << node_id << 
				", NIU " << id << 
				", Out FIFO" <<
				", ts " << tstamp <<
				", src mod " << src_mod <<
				", dst mod " << dst_mod <<
				", dst pin " << dst_pin <<
				", push " << endl;

		// send packet data to gui
		if (USEGUI == 1)
		{
			gbuff->WriteQ(clk_count, node_id, id, 2, tstamp, src_mod, dst_mod, dst_pin, 1); // out push
		}

//		cout << "Node " << node_id << ", NIU " << id << ": out fifo write, src mod: " <<  src_mod << endl;

		out_queue.push(data);

		if (out_queue.size() >= MAX_NIU_FIFO)
		{
			out_queue.front();
			out_queue.pop();
			cout << "Node " << id << ": NIU drop out Q" << endl;
		}

	}

	size = out_queue.size();
	out_size = size;
	
	// drive output to next node
	if (size > 0)
	{
		data = out_queue.front();

		dst_pin = data & pin_mask;
		dst_mod = (data >> (PIN_BITS))  &  mod_mask;
		src_mod = (data >> (MOD_BITS + PIN_BITS))  &  mod_mask;
		tstamp =  (data >> (MOD_BITS + MOD_BITS + PIN_BITS))  &  ts_mask;

		cout << "clk " << clk_count << 
				", module " << node_id << 
				", NIU " << id << 
				", Out FIFO" <<
				", ts " << tstamp <<
				", src mod " << src_mod <<
				", dst mod " << dst_mod <<
				", dst pin " << dst_pin <<
				", pop " << endl;

		if (USEGUI == 1)
		{
			gbuff->WriteQ(clk_count, node_id, id, 2, tstamp, src_mod, dst_mod, dst_pin, 2);
		}

		out_queue.pop();

		*edge_out_v = 1;
		*edge_out_d = data;
		
	}

//	cout << "Node " << node_id << ", NIU " << id << " gpf debug " << endl;

	// input valid from neighboring node
	if (*edge_in_v == 1)
	{
		data = *edge_in_d;

		dst_pin = data & pin_mask;
		dst_mod = (data >> (PIN_BITS))  &  mod_mask;
		src_mod = (data >> (MOD_BITS + PIN_BITS))  &  mod_mask;
		tstamp =  (data >> (MOD_BITS + MOD_BITS + PIN_BITS))  &  ts_mask;

		cout << "clk " << clk_count << 
				", module " << node_id << 
				", NIU " << id << 
				", In FIFO" <<
				", ts " << tstamp <<
				", src mod " << src_mod <<
				", dst mod " << dst_mod <<
				", dst pin " << dst_pin <<
				", count " << in_queue.size() <<
				", push " << endl;

		if (USEGUI == 1)
		{
			gbuff->WriteQ(clk_count, node_id, id, 1, tstamp, src_mod, dst_mod, dst_pin, 1);
		}

		in_queue.push(data);

		if (in_queue.size() >= MAX_NIU_FIFO)
		{
			in_queue.front();
			in_queue.pop();
			cout << "Node " << node_id << ": NIU drop in Q" << endl;
		}

//		cout << "Node " << node_id << ", NIU " << id << ": in fifo, data: " << data << endl;
	}

	// drive input to router
	size = in_queue.size();
	in_size = size;
	if ((size > 0) && (*in_a == 1))
	{
		data = in_queue.front();

		dst_pin = data & pin_mask;
		dst_mod = (data >> (PIN_BITS))  &  mod_mask;
		src_mod = (data >> (MOD_BITS + PIN_BITS))  &  mod_mask;
		tstamp =  (data >> (MOD_BITS + MOD_BITS + PIN_BITS))  &  ts_mask;

		cout << "clk " << clk_count << 
				", module " << node_id << 
				", NIU " << id << 
				", In FIFO" <<
				", ts " << tstamp <<
				", src mod " << src_mod <<
				", dst mod " << dst_mod <<
				", dst pin " << dst_pin <<
				", pop " << endl;

		if (USEGUI == 1)
		{
			gbuff->WriteQ(clk_count, node_id, id, 1, tstamp, src_mod, dst_mod, dst_pin, 2);
		}

		in_queue.pop();
//		cout << "Node " << node_id << ", NIU " << id << ": in fifo read, data: " << data << endl;
		*in_v = 1;
		*in_d = data;
	}

	gui_id = id * 2;

	if (in_size > 9) in_size = 9;
	if (in_size > 9) out_size = 9;
//	gbuff->WriteQ(node_id, gui_id, in_size);
//	gbuff->WriteQ(node_id, gui_id+1, out_size);

	return 0;
}

int NIU::LowPhase()
{


	return 0;
}

int NIU::Run()
{
	if (*clk == 1)
	{
		HighPhase();
	}
	else
	{
		LowPhase();
	}

	return 0;
}

/*****************************************************************************
Async Pulse Processor

*****************************************************************************/
APP::APP(int myid, TopChannel *top_channel, MainChannel *m_channel, GUIBuffer *g_buffer)
{
	int i;

	
	sim_time = 0.0;

	srand ( time(NULL) );

	// fill queue with initial data
	if (RAND_SPIKE==0)
	{	
		myrain = new RainsNet(myid);
		myrain->start();
		sp_source = myrain->sp_source;
		sp_time = myrain->sp_time;
	}


	id = myid;

	one_shot = 0;

	// input
	clk = &m_channel->clk;
	reset = &m_channel->reset;

	// output
	valid = &top_channel->app_out_v;
	for (i=0;i<MAX_PIN_COUNT;i++)
	{
		data[i] = &top_channel->app_out_data[i];
	}

	gbuff = g_buffer;


}

APP::~APP()
{
	delete myrain;
}

int APP::EvalInput()
{


	// this must be made global later
	sim_time += .001; // 1MHz clock

	if (*reset == 1)
	{
		rnd->SeedRandom(0);
	}


	return 0;
}

int APP::DriveOutput()
{
	int i;
	int fire;
	double temp;
	int fpin;
	double threshold = THRESH;
	threshold = 0.9;
	fire = 0;

	for(i=0;i<MAX_PIN_COUNT;i++)
	{
		*data[i] = 0;
	}

	if (RAND_SPIKE == 1)
	{
		temp = (double)MAX_PIN_COUNT*(double)rand()/(double)RAND_MAX;
		fpin = floor(temp);
		
		//cout << "debug random " << fpin << endl;

		temp = rand()/(double)RAND_MAX;

		//cout << "debug random " << temp << endl;
		if (Connected[fpin] == 1)
		{
			if (temp < threshold)
			{
				cout << "APP " << id << " fire " << " pin " << fpin <<  endl;

				*data[fpin] = 1;
				*valid = 1;
			}
			else
			{
				*data[fpin] = 0;
				*valid = 0;
			}
		}

	}
	else
	{
		if (sim_time >= sp_time)
		{
			if (Connected[sp_source] == 1)
			{

			// time to spike
			cout << "APP " << id << " spike source " << sp_source << " time " << sp_time << " sim time " << sim_time << endl;

				*data[sp_source] = 1;
				fire = 1; 
			}
			
			// Get next spike ready
			myrain->start();
			sp_source = myrain->sp_source;
			sp_time = myrain->sp_time;

			// GUI interface
			if (fire == 1)
			{
	//			gbuff->WriteQ(id, 0, 5);
			}
			else
			{
	//			gbuff->WriteQ(id, 0, 0);
			}
		
			*valid = fire;

		}
	}

	return 0;
}

int APP::Run()
{
//	cout << "APP " << id << "runapp" << endl;	
	if (*clk == 1)
	{
//		cout << "clock is high" << endl;
		EvalInput();
	}
	else
	{
//		cout << "clock is low" << endl;
		DriveOutput();
	}
	return 0;
}

int APP::LoadAddrList()
{
	int i, j;
	int src_pin;

	//cout << "APP " << id << "loadaddrlist" << endl;	

			for (j=0;j<4;j++)
			{
	//			cout << in_file_matrix[i][j] << " ";

			}

	for (i=0;i<MAX_CONNECT;i++)
	{
		if (id == in_file_matrix[i][0])
		{
	/*
			for (j=0;j<4;j++)
			{
//				cout << in_file_matrix[i][j] << " ";

			}
//			cout << endl;
			src_pin = in_file_matrix[i][1];
			dst_mod = in_file_matrix[i][2];
			dst_pin = in_file_matrix[i][3];
*/

			src_pin = in_file_matrix[i][1];
			Connected[src_pin] = 1;
		}

	}

	return 0;
}

/*****************************************************************************

Digital Fabric Node

*****************************************************************************/
DFN::DFN (int myid, TopChannel * top_channel, MainChannel *m_channel, GUIBuffer *gbuff) 
{
	id = myid;

	dfn_channel = new DFNChannel;

	APPIU = new AIU(top_channel, m_channel, dfn_channel);
	APPIU->id = id;

	SPU1 = new SPU(m_channel, dfn_channel);
	SPU1->id = id;
	SPU1->LoadAddrList();

	R1 = new Router(m_channel, dfn_channel);
	R1->id = id;
	
	// Router sends packets to one of four destinations,
	// north, south, east and west.

	// NIU interface
	NorthNIU = new NIU(m_channel, dfn_channel, gbuff);
	NorthNIU->node_id = id;
	NorthNIU->id = 1;
	NorthNIU->out_v = &dfn_channel->n_out_v;
	NorthNIU->out_a = &dfn_channel->n_out_a;
	NorthNIU->out_d = &dfn_channel->n_out_d;
	NorthNIU->in_v = &dfn_channel->n_in_v;
	NorthNIU->in_a = &dfn_channel->n_in_a;
	NorthNIU->in_d = &dfn_channel->n_in_d;

	SouthNIU = new NIU(m_channel, dfn_channel, gbuff);
	SouthNIU->node_id = id;
	SouthNIU->id = 2;
	SouthNIU->out_v = &dfn_channel->s_out_v;
	SouthNIU->out_a = &dfn_channel->s_out_a;
	SouthNIU->out_d = &dfn_channel->s_out_d;
	SouthNIU->in_v = &dfn_channel->s_in_v;
	SouthNIU->in_a = &dfn_channel->s_in_a;
	SouthNIU->in_d = &dfn_channel->s_in_d;

	EastNIU = new NIU(m_channel, dfn_channel, gbuff);
	EastNIU->node_id = id;
	EastNIU->id = 3;
	EastNIU->out_v = &dfn_channel->e_out_v;
	EastNIU->out_a = &dfn_channel->e_out_a;
	EastNIU->out_d = &dfn_channel->e_out_d;
	EastNIU->in_v = &dfn_channel->e_in_v;
	EastNIU->in_a = &dfn_channel->e_in_a;
	EastNIU->in_d = &dfn_channel->e_in_d;


	WestNIU = new NIU(m_channel, dfn_channel, gbuff);
	WestNIU->node_id = id;
	WestNIU->id = 4;
	WestNIU->out_v = &dfn_channel->w_out_v;
	WestNIU->out_a = &dfn_channel->w_out_a;
	WestNIU->out_d = &dfn_channel->w_out_d;
	WestNIU->in_v = &dfn_channel->w_in_v;
	WestNIU->in_a = &dfn_channel->w_in_a;
	WestNIU->in_d = &dfn_channel->w_in_d;

	// Router 
	R1->n_v = &dfn_channel->n_in_v;
	R1->n_a = &dfn_channel->n_in_a;
	R1->n_d = &dfn_channel->n_in_d;

	R1->s_v = &dfn_channel->s_in_v;
	R1->s_a = &dfn_channel->s_in_a;
	R1->s_d = &dfn_channel->s_in_d;

	R1->e_v = &dfn_channel->e_in_v;
	R1->e_a = &dfn_channel->e_in_a;
	R1->e_d = &dfn_channel->e_in_d;

	R1->w_v = &dfn_channel->w_in_v;
	R1->w_a = &dfn_channel->w_in_a;
	R1->w_d = &dfn_channel->w_in_d;

}



DFN::~DFN () {

	delete APPIU;
	delete NorthNIU;
	delete SouthNIU;
	delete EastNIU;
	delete WestNIU;
 
}

int DFN::init()
{

	return 0;
}

int DFN::EvalInputs()
{

	return 0;
}

int DFN::Run()
{	
	APPIU->Run();
	SPU1->Run();
	R1->Run();
	NorthNIU->Run();
	SouthNIU->Run();
	EastNIU->Run();
	WestNIU->Run();

	return 0;
}

/*****************************************************************************
Digital Fabric Mesh

*****************************************************************************/
Mesh::Mesh(GUIBuffer *gbuff)
{
	int i;
	int size;
	int num_edge;

	term = 0;
	
	MainChan = new MainChannel;

	for (i=0;i<MAX_MESH_SIZE;i++)
	{
		ChanList[i] = new TopChannel;

		ProcList[i] = new APP(i, ChanList[i], MainChan, gbuff);
		ProcList[i]->LoadAddrList();


		NodeList[i] = new DFN(i, ChanList[i], MainChan, gbuff);
	}

	// Test Bench drives clock and reset
	Bench = new TestBench(MainChan);

	size = sqrt(MAX_MESH_SIZE);
	num_edge = 2 * (size -1) * (size);
	num_edge = num_edge + size; // north - south wrap
	num_edge = num_edge + size; // east - west wrap

	for (i=0;i<num_edge;i++)
	{
		EdgeList[i] = new EdgeChannel;
	}
	
//	cout << "debug: mesh connect" << endl;

// 12 13 14 15 
// 8  9  10 11
// 4  5  6  7
// 0  1  2  3  


	int j;
	int temp;
	int k = 0;
	for (i=0;i<size-1;i++)
	{
		for (j=0;j<size-1;j++)
		{
			temp = j+i*size;
			cout << "connecting " << temp << " to " << temp+1 << " using " << k << endl;
			ConnectH(temp,temp+1,k);
			k++;
			cout << "connecting " << temp << " to " << temp+size << " using " << k << endl;
			ConnectV(temp,temp+size,k);
			k++;
		}
	}

	// reverse
	for (i=0;i<size;i++)
	{
		temp = i*size;
		cout << "connecting " << temp+size-1 << " to " << temp << " using " << k << endl;
		ConnectH(temp+size-1,temp,k);
		k++;
	}

	for (i=0;i<size-1;i++)
	{
		temp = i*size;
		cout << "connecting " << temp+size-1 << " to " << temp+size+size-1 << " using " << k << endl;
		ConnectV(temp+size-1,temp+size+size-1,k);
		k++;
	}

	// reverse
	for (i=0;i<size;i++)
	{
		temp = i;
		cout << "connecting " << temp+(size-1)*size << " to " << temp << " using " << k << endl;
		ConnectV(temp+(size-1)*size,temp,k);
		k++;
	}

	for (i=0;i<size-1;i++)
	{
		temp = i;
		cout << "connecting " << temp+(size-1)*size << " to " << temp+(size-1)*size+1 << " using " << k << endl;
		ConnectH(temp+(size-1)*size,temp+(size-1)*size+1,k);
		k++;
	}


}



Mesh::~Mesh()
{
	int i;

	for (i=0;i<MAX_MESH_SIZE;i++)
	{
		delete ProcList[i];
		delete NodeList[i];
	}

}

int Mesh::ConnectH(int node_a, int node_b, int channel)
{
	 NodeList[node_a]->EastNIU->edge_out_v = &EdgeList[channel]->ch1_v;
	 NodeList[node_a]->EastNIU->edge_out_a = &EdgeList[channel]->ch1_a;
	 NodeList[node_a]->EastNIU->edge_out_d = &EdgeList[channel]->ch1_d;

	 NodeList[node_b]->WestNIU->edge_in_v = &EdgeList[channel]->ch1_v;
	 NodeList[node_b]->WestNIU->edge_in_a = &EdgeList[channel]->ch1_a;
	 NodeList[node_b]->WestNIU->edge_in_d = &EdgeList[channel]->ch1_d;

	 NodeList[node_a]->EastNIU->edge_in_v = &EdgeList[channel]->ch2_v;
	 NodeList[node_a]->EastNIU->edge_in_a = &EdgeList[channel]->ch2_a;
	 NodeList[node_a]->EastNIU->edge_in_d = &EdgeList[channel]->ch2_d;

	 NodeList[node_b]->WestNIU->edge_out_v = &EdgeList[channel]->ch2_v;
	 NodeList[node_b]->WestNIU->edge_out_a = &EdgeList[channel]->ch2_a;
	 NodeList[node_b]->WestNIU->edge_out_d = &EdgeList[channel]->ch2_d;

	return 0;
}

int Mesh::ConnectV(int node_a, int node_b, int channel)
{
	 NodeList[node_a]->SouthNIU->edge_out_v = &EdgeList[channel]->ch1_v;
	 NodeList[node_a]->SouthNIU->edge_out_a = &EdgeList[channel]->ch1_a;
	 NodeList[node_a]->SouthNIU->edge_out_d = &EdgeList[channel]->ch1_d;

	 NodeList[node_b]->NorthNIU->edge_in_v = &EdgeList[channel]->ch1_v;
	 NodeList[node_b]->NorthNIU->edge_in_a = &EdgeList[channel]->ch1_a;
	 NodeList[node_b]->NorthNIU->edge_in_d = &EdgeList[channel]->ch1_d;

	 NodeList[node_a]->SouthNIU->edge_in_v = &EdgeList[channel]->ch2_v;
	 NodeList[node_a]->SouthNIU->edge_in_a = &EdgeList[channel]->ch2_a;
	 NodeList[node_a]->SouthNIU->edge_in_d = &EdgeList[channel]->ch2_d;

	 NodeList[node_b]->NorthNIU->edge_out_v = &EdgeList[channel]->ch2_v;
	 NodeList[node_b]->NorthNIU->edge_out_a = &EdgeList[channel]->ch2_a;
	 NodeList[node_b]->NorthNIU->edge_out_d = &EdgeList[channel]->ch2_d;

	return 0;
}



int Mesh::Print()
{

	int i;
	unsigned int mydata;
	int data1;

	cout << "app_out_data register" << endl;
	for (i=0;i<MAX_PIN_COUNT;i++)
	{
	mydata = NodeList[5]->APPIU->app_in_data[i];
//	mydata = ChanList[5]->app_out_data[i];

	cout << mydata << " ";
	}
	cout << endl;

	data1 = *NodeList[5]->APPIU->clk;

	cout << "AIU clock: " << data1 << endl;



	return 0;
}

int Mesh::Run()
{
	int i;

	cout << "high phase " << endl;
	Bench->Run();
		
	for (i=0;i<MAX_MESH_SIZE;i++)
	{
		ProcList[i]->Run();
		NodeList[i]->Run();
	}

//	Print();

	// low phase

	cout << "low phase " << endl;
	Bench->Run();
	
	for (i=0;i<MAX_MESH_SIZE;i++)
	{
		ProcList[i]->Run();
		NodeList[i]->Run();
	}

//	Print();




	return 0;
}
