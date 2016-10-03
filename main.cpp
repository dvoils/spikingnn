//#include <iostream>
#include "objects.h"

using namespace std;

int main () 
{
	int i;
	int clcnt;
	int done;

//	int mystart, mystop;

	Mesh *mymesh;
	Supervisor *mysup;
	mysup = new Supervisor;

	char infile[] = "dfout.txt";
	mysup->ReadFile(infile);
//	mysup->Print();

 	srand ( time(NULL) );


	MyThread *mynet;
	GUIBuffer *mybuf;

	mybuf = new GUIBuffer;
//	mynet = new MyThread(mybuf);
	mymesh = new Mesh(mybuf);

	for (i=0;i<3000;i++)
	{
		cout << "clock " << clcnt << endl;
//		mybuf->start = 0;

		mymesh->Run();

//			mybuf->Done();

//			mybuf->go = 1;
		clcnt++;
	}


//	mynet->cancel();
//	mynet->join();

	delete mysup;
	delete mymesh;
//	delete mynet;
	delete mybuf;

  return 0;
}

