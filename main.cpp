#include "simlib.h"
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <cmath>



const double routes[3] = {60.0*60.0, 10.0*60.0, 25.0*60.0}; // doba cesty na jednotlivych usecich - Veseli - Bucovice, Bucovice - Slavkov, Slavkov - Brno
const double PeakTime = 180.5; // doba trvani spicky v minutach
const double NonPeakTime = 780.5; // doba trvani nespicky v minutach(cas mezi spickou a noci)

const double PeakTimeInterval = 60; // interval mezi vlaky ve spicce
const double NonPeakTimeInterval = 120; // interval mezi vlaky mimo spickuS
const int timeToEnter = 2;
const int amountOfWagons = 3;
const double pocetMistVeVagonu = 33.3;
const int amountOfEntersIntoWagon = amountOfWagons*2;

const double PeakTimeParameter = 0.54; // procent lidi ve spicce
const double TrainsInPeakTime = PeakTime/PeakTimeInterval;
const double TrainsInNonPeakTime = NonPeakTime/NonPeakTimeInterval;

const int passengers[3] = {550, 250, 200};

int departures[] = {4*3600+3000, 6*3600+60, 7*3600+60, 9*3600+60, 11*3600+60, 13*3600+60, 15*3600+60, 17*3600+60, 19*3600+60, 21*3600+60};


const int amountOfStations = 4;
Queue waitingRooms[3];

Facility Stations[amountOfStations];

Histogram Table("Cestujici", 0, 600,20);
Histogram Trains("Vlaky", 0, 3600,20);
int TimeOfDay(){
	int time = ((int) Time % 86400);
	return time;
}



int partOfDay(int time) {
	if ((time >= 4*3600+1800) && (time <= 8*3600+1800)) {
		return 1;
	} else if ((time > (8*3600+1800)) && (time <= 22*3600)) {
		return 2;
	} else {
		return 0;
	}
}


class Train : public Process {
public:
	Train(int time) : Process() {
		initDepartureTime = time;
		store = new Store((unsigned int) round(amountOfWagons*pocetMistVeVagonu));
		currentTime = initDepartureTime;
	}


	void Behavior() {

		currentStation = -1;
		this->currentTime = TimeOfDay();
		Seize(Stations[0]);
		currentStation = 0;
		//Wait(20); // zastaveni vlaku
		Wait(2);

		//Wait(((this->store->Free() > waitingRooms[0].Length()) ? (waitingRooms[0].Length()*amountOfWagons/pocetVstupu) : (this->store->Free())*amountOfWagons/pocetVstupu));
		Release(Stations[0]);
		currentStation = -1;
		Print("---------------------------------------\n");
		Print("Vlak v %d jede do Bucovic, vyjizdi v %f\n", getCurrentTime()/3600, Time/3600);
		Wait(routes[0]);
		Print("Vlak v %d | Trvani: %.0f (min) | Dojel v %.5f (hod) \n", getCurrentTime()/3600, routes[0]/60.0, (Time)/3600);

		//Print("---------------------------------------\n");
		this->currentTime = TimeOfDay();
		std::cout<< "Aktualni cas: " << this->getCurrentTime()/3600 << std::endl;
		Seize(Stations[1]);
		currentStation = 1;
		Wait(2);
		//Wait(20); // zastaveni vlaku
		//Wait((this->store->Free() > waitingRooms[1].Length() ? waitingRooms[1].Length() : this->store->Free())*pocetVagonu/pocetVstupu);
		Release(Stations[1]);
		currentStation = -1;
		Print("---------------------------------------\n");
		Print("Vlak v %d jede do Slavkova\n", getCurrentTime()/3600);
		Wait(routes[1]);
		Print("Vlak v %d | Trvani: %.0f (min) | Dojel v %.2f (hod)\n", getCurrentTime()/3600, routes[1]/60.0, (Time)/3600);
		Print("---------------------------------------\n");
		this->currentTime = TimeOfDay();
		std::cout<< "Aktualni cas: " << this->getCurrentTime()/3600 << std::endl;
		Seize(Stations[2]);
		currentStation = 2;
		//Wait(20); // zastaveni vlaku
		//Wait((this->store->Free() > waitingRooms[2].Length() ? waitingRooms[2].Length() : this->store->Free())*pocetVagonu/pocetVstupu);
		Release(Stations[2]);
		currentStation = -1;
		Print("---------------------------------------\n");
		Print("Vlak v %d jede do Brna\n", getCurrentTime()/3600);
		Wait(routes[2]);
		Print("Vlak v %d | Trvani: %.0f (min)| Dojel v %.2f (hod)\n", getCurrentTime()/3600, routes[2]/60.0, (Time)/3600);

		this->currentTime = TimeOfDay();
		std::cout<< "Aktualni cas: " << this->getCurrentTime()/3600 << std::endl;
		Seize(Stations[3]);
		currentStation = 3;
		//Wait(20); // zastaveni vlaku

		Release(Stations[3]);
	}

	int getInitDepartureTime() const {
		return initDepartureTime;
	}

	int getCurrentStation() const {
		return currentStation;
	}

	int getCurrentTime() const {
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
	int currentTime;
	int currentStation;
};


std::vector<Train*> trains;


int nejblizsiVlak(int time, int station) {
	int nearest = 0;
	for (unsigned int i = 0; i < trains.size(); i++) {
		/*std::cout << "I: " << i << std::endl;
		std::cout << "Time: " << Time << std::endl;
		std::cout << "Aktualni cas vlaku: " << trains.at(i)->getCurrentTime()/3600 << std::endl;*/
		if ((trains.at(i)->getCurrentTime() - time) > (trains.at(nearest)->getCurrentTime() - time)) {
			nearest = i;
		}
	}
	return nearest;
}


class Passenger : public Process {
public:
	Passenger(int Station) : Process() {
		station = Station;
	}


	bool isInTrain() {
		return inTrain;
	}



	void Behavior() {
		double Came;
		Came = Time;
		inTrain = false;


		Into(waitingRooms[station]);
	enterTrain:
		unsigned long nearest = (unsigned long) nejblizsiVlak(TimeOfDay(), station);
		if (Stations[station].Busy() && !trains.at(nearest)->isFull()) {
			waitingRooms[station].GetFirst();
			Enter(*trains.at(nearest)->getStore());
			inTrain = true;
		} else {
			Wait(2);
			goto enterTrain;
		}


		for (int i = station+1; i < amountOfStations; i++) {

			double vystup = Random();
			test:
			if (Stations[i].Busy() && isInTrain() && vystup <= 0.1) {
				Leave(*trains.at(nearest)->getStore());
				inTrain = false;
			} else {
				Wait(2);
				goto test;
			}
		}


		if (Stations[2].Busy() && isInTrain()) {
			Leave(*trains.at(nearest)->getStore());
		}

	}

	bool inTrain;
	int station;
};



class PassengerGenerator : public Event {
public:
	PassengerGenerator(int Station) : Event() {
		station = Station; // stanice 0 je Veseli, stanice 1 je Bucovice, 2 je Slavkov (negenerujeme nic)
	}

	void Behavior() {
		int time = TimeOfDay();
		if (partOfDay(time) == 1) {
			Activate(Time+Exponential(PeakTimeInterval/((passengers[station]*PeakTimeParameter)/PeakTime)));
		} else if (partOfDay(time) == 2) {
			Activate(Time+Exponential(NonPeakTimeInterval/((passengers[station]*(1.00-PeakTimeParameter))/NonPeakTime)));
		} else {
			Activate(Time+Normal(3600, 55));
		}

		(new Passenger(station))->Activate();
	}

	int station;

};



class TrainGenerator : public Process {
	void Behavior() {
		Train* vlak;
		int size = sizeof(departures)/sizeof(departures[0]);
		for (int i = 0; i < size; i++) {
		prijezd:
			int time = TimeOfDay();
			if (time == departures[i]) {
				vlak = (new Train(departures[i]));
				trains.push_back(vlak);
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

	(new PassengerGenerator(0))->Activate();
	(new PassengerGenerator(1))->Activate();
	(new PassengerGenerator(2))->Activate();
	(new TrainGenerator())->Activate();
	Run();
	//Table.Output();
	//Trains.Output();
	Stations[0].Output();
	Stations[1].Output();
	Stations[2].Output();
	Stations[3].Output();
	waitingRooms[0].Output();
	waitingRooms[1].Output();
	waitingRooms[2].Output();
	for (unsigned int i = 0; i < trains.size(); i++) {
		trains.at(i)->getStore()->Output();
	}

	return 0;
};