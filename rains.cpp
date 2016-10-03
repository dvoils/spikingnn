#include "rains.h"

/*****************************************************************************
Rains Network

*****************************************************************************/
RainsNet::RainsNet(int myid)
{
	cout << "creating RAINS network " << myid << endl;
	n = new Network(NEURONS);
	id = myid;



}

RainsNet::~RainsNet()
{

}

int RainsNet::start()
{

	int qsize;

	n->start();

	int k = 0;
	int sender;

	qsize = spqueue.size();


	if (qsize < 5)
	{

		while (!(n->backup.empty()))
		{
			k++;
			s = n->backup.front();
			n->backup.pop();
			//sender = s.sender % 100;
			sender = s.sender;
			cout << "RainsNet " << id << " s " << sender << " t " << s.t*Taum << "adj s" << sender << endl;

			// using a queue to stream spikes - dvoils

			

			myspinfo.source = sender; // transmitting neuron
			myspinfo.time = s.t*Taum;	// time stamp

			spqueue.push(myspinfo);

//			int_spq.push(s.sender);
//			fl_spq.push(s.t*Taum);

		//	sprintf(buf,"%d",(s.sender));
		//	fputs(buf,file);
		//	sprintf(buf,"%f",s.t*Taum);
		//	fputs(" ",file);
		//	fputs(buf,file);
		//	fputs("\n",file);
		}

	}

	

	myspinfo = spqueue.front();
	spqueue.pop();

	sp_source = myspinfo.source; 
	sp_time = myspinfo.time; 

//	sp_source = int_spq.front();
//	int_spq.pop();

//	sp_time = fl_spq.front();
//	fl_spq.pop();


//	cout << "RainsNet pop " << id << " s " << sp_source << " t " << sp_time << endl;

	return 0;
}



int RainsNet::GetSpike()
{
	myspinfo = spqueue.front();
	spqueue.pop();

	sp_source = myspinfo.source; 
	sp_time = myspinfo.time; 

//	myqueue.pop();

	return 0;
}


