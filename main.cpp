#include "simlib.h"
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <cmath>



const double RouteA = 3600.0; // doba cesty na trase Veseli - Bucovice
const double RouteB = 10.0*60.0; // doba cesty na trase Bucovice - Slavkov
const double RouteC = 25.0*60.0; // doba cesty na trase Slavkov - Brno
const double PeakTime = 180.5; // doba trvani spicky v minutach
const double NonPeakTime = 780.5; // doba trvani nespicky v minutach(cas mezi spickou a noci)

const double SpickaIntervalVlaku = 60; // interval mezi vlaky ve spicce
const double NespickaIntervalVlaku = 120; // interval mezi vlaky mimo spickuS
const int dobaNastupu = 2;
const int pocetVagonu = 3;
const double pocetMistVeVagonu = 33.3;
const int pocetVstupu = pocetVagonu*2;

const double pocetLidiVeSpicce = 0.54; // procent lidi ve spicce
const double VlakyVeSpicce = PeakTime/SpickaIntervalVlaku;
const double VlakyMimoSpicku = NonPeakTime/NespickaIntervalVlaku;

const int cestujici[3] = {550, 250, 200};

int arrivals[] = {5*3600, 6*3600, 7*3600, 8*3600, 10*3600, 12*3600, 14*3600, 16*3600, 18*3600, 20*3600, 22*3600};


const int amountOfStations = 4;
Queue cekarna[3];

Facility Stations[amountOfStations];

Histogram Table("Cestujici", 0, 600,20);
Histogram Trains("Vlaky", 0, 3600,20);
int TimeOfDay(){
	int time = ((int) Time % 86400);
	return time;
}



int castDne(int time) {
	if ((time >= 4*3600+1800) && (time <= 8*3600+1800)) {
		return 1;
	} else if ((time > (8*3600+1800)) && (time <= 22*3600)) {
		return 2;
	} else {
		return 0;
	}
}


class Vlak : public Process {
public:
	Vlak(int time) : Process() {
		initDepartureTime = time;
		store = new Store((unsigned int) (pocetVagonu*pocetMistVeVagonu));
		currentTime = initDepartureTime;
	}


	void Behavior() {

		this->currentTime = TimeOfDay();
		Seize(Stations[0]);
		Wait(20); // zastaveni vlaku
		Print("Waitin..\n");

		//Wait(((this->store->Free() > cekarna[0].Length()) ? (cekarna[0].Length()*pocetVagonu/pocetVstupu) : (this->store->Free())*pocetVagonu/pocetVstupu));
		Release(Stations[0]);
		//Print("---------------------------------------\n");
		//Print("Vlak v %d jede do Bucovic, vyjizdi v %f\n", cas/3600, Time/3600);
		Wait(RouteA);
		//Print("Vlak v %d | Trvani: %.0f (min) | Dojel v %.5f (hod) \n", cas/3600, RouteA/60.0, (Time)/3600);

		//Print("---------------------------------------\n");
		this->currentTime = TimeOfDay();
		Seize(Stations[1]);
		Wait(20); // zastaveni vlaku
		//Wait((this->store->Free() > cekarna[1].Length() ? cekarna[1].Length() : this->store->Free())*pocetVagonu/pocetVstupu);
		Release(Stations[1]);
		//Print("---------------------------------------\n");
		//Print("Vlak v %d jede do Slavkova\n", cas/3600);
		Wait(RouteB);
//		Print("Vlak v %d | Trvani: %.0f (min) | Dojel v %.2f (hod)\n", cas/3600, RouteB/60.0, (Time)/3600);
		//Print("---------------------------------------\n");
		this->currentTime = TimeOfDay();
		Seize(Stations[2]);
		Wait(20); // zastaveni vlaku
		//Wait((this->store->Free() > cekarna[2].Length() ? cekarna[2].Length() : this->store->Free())*pocetVagonu/pocetVstupu);
		Release(Stations[2]);
		//Print("---------------------------------------\n");
		//Print("Vlak v %d jede do Brna\n", cas/3600);
		Wait(RouteC);
		//Print("Vlak v %d | Trvani: %.0f (min)| Dojel v %.2f (hod)\n", cas/3600, RouteC/60.0, (Time)/3600);

		this->currentTime = TimeOfDay();
		Seize(Stations[3]);
		Wait(20); // zastaveni vlaku

		Release(Stations[3]);
	}

	int getInitDepartureTime() const {
		return initDepartureTime;
	}

	double getCurrentTime() const {
		return currentTime;
	}

	Store *getStore() const {
		return store;
	}
	bool isFull() {
		return this->store->Full();
	}
private:
	int initDepartureTime;
	Store *store;
	double currentTime;
};


std::vector<Vlak*> vlaky;


int nejblizsiVlak(int time) {
	int nearest = 0;
	for (unsigned int i = 0; i < vlaky.size(); i++) {
		std::cout << "I: " << i << std::endl;
		std::cout << "Time: " << Time << std::endl;
		std::cout << "Aktualni cas vlaku: " << vlaky.at(i)->getCurrentTime()/3600 << std::endl;
		if ((vlaky.at(i)->getCurrentTime() - time) > (vlaky.at(nearest)->getCurrentTime() - time)) {
			nearest = i;
		}
	}
	return nearest;
}


class Cestujici : public Process {
public:
	Cestujici(int Stanice) : Process() {
		stanice = Stanice;
	}


	bool isInTrain() {
		return inTrain;
	}



void Behavior() {
		double Prichod;
		Prichod = Time;
		inTrain = false;


		Into(cekarna[stanice]);
	enterTrain:
		unsigned long nearest = (unsigned long) nejblizsiVlak(TimeOfDay());
		if (Stations[stanice].Busy() && !vlaky.at(nearest)->isFull()) {
			cekarna[stanice].GetFirst();
			Enter(*vlaky.at(nearest)->getStore());
			inTrain = true;
		} else {
			WaitUntil(Stations[stanice].Busy());
			goto enterTrain;
		}
/*
		if (!Stanice[stanice].Busy() || !vlaky.at(nearest)->isFull()) {
			Into(cekarna[stanice]);
			WaitUntil(Stanice[stanice].Busy() && !vlaky.at(nearest)->isFull());
			cekarna[stanice].GetFirst();
			goto enterTrain;
		}
		if (Stanice[stanice].Busy() && !isInTrain() && !vlaky.at(nearest)->isFull()) {
			Enter(*vlaky.at(nearest)->getStore());
			inTrain = true;
			Table(Time-Prichod);
		} else {

		}
*/


		for (int i = stanice+1; i < amountOfStations; i++) {
			WaitUntil(inTrain && Stations[i].Busy());
			double vystup = Random();
			if (isInTrain() && vystup <= 0.1) {
				Leave(*vlaky.at(nearest)->getStore());
				inTrain = false;
			}
		}


		if (Stations[2].Busy() && inTrain) {
			Leave(*vlaky.at(nearest)->getStore());
		}

	}

	bool inTrain;
	int stanice;
};



class GeneratorCestujici : public Event {
public:
	GeneratorCestujici(int Stanice) : Event() {
		stanice = Stanice; // stanice 0 je Veseli, stanice 1 je Bucovice, 2 je Slavkov (negenerujeme nic)
	}

	void Behavior() {
		int time = TimeOfDay();
		if (castDne(time) == 1) {
			Activate(Time+Exponential(SpickaIntervalVlaku/((cestujici[stanice]*pocetLidiVeSpicce)/PeakTime)));
		} else if (castDne(time) == 2) {
			Activate(Time+Exponential(SpickaIntervalVlaku/((cestujici[stanice]*(1.00-pocetLidiVeSpicce))/NonPeakTime)));
		} else {
			Activate(Time+Normal(3600, 55));
		}

		(new Cestujici(stanice))->Activate();
	}

	int stanice;

};



class GeneratorVlak : public Process {
	void Behavior() {
		Vlak* vlak;
		int size = sizeof(arrivals)/sizeof(arrivals[0]);
		for (int i = 0; i < size; i++) {
		prijezd:
			int time = TimeOfDay();
			if (time == arrivals[i]) {
				vlak = (new Vlak(arrivals[i]));
				vlaky.push_back(vlak);
				vlak->Activate(Time);
			} else {
				Wait(1);
				goto prijezd;
			}
		}
	}

};

int main() {
	Print("Model vlakove trasy Bucovice - Brno\n");
	RandomSeed(time(NULL));
	Init(0, 86400);

	(new GeneratorCestujici(0))->Activate();
	(new GeneratorCestujici(1))->Activate();
	(new GeneratorCestujici(2))->Activate();
	(new GeneratorVlak())->Activate();
	Run();
	//Table.Output();
	//Trains.Output();
	Stations[0].Output();
	Stations[1].Output();
	Stations[2].Output();
	Stations[3].Output();
	cekarna[0].Output();
	cekarna[1].Output();
	cekarna[2].Output();
	for (unsigned int i = 0; i < vlaky.size(); i++) {
		vlaky.at(i)->getStore()->Output();
	}

	return 0;
};