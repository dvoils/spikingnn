#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include "network.h"

#include <queue>

/*****************************************************************************

RAINS network

*****************************************************************************/
class RainsNet
{




	struct spinfo {
	  int source;
	  float time;
	} ;

	spinfo myspinfo;

	queue<spinfo> spqueue;

//	queue<int> int_spq;
//	queue<float> fl_spq;

	Spike s;



	int id;

	int h2,m2,s2,h3,m3,s3;
	time_t timer1;
	Network *n;

	public:

	// call GetSpike to fill these
	int sp_source;
	float sp_time;


	RainsNet(int);
	

	~RainsNet();

	int start();
	int GetSpike();
	

};
