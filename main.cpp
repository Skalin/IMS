
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

class Train;
std::vector<Train*> trains;

const int amountOfStations = 4;
Queue waitingRooms[amountOfStations-1];

Facility Stations[amountOfStations];

Histogram Table("Cestujici", 0, 600,20);
Histogram Trains("Vlaky", 0, 3600,20);
int TimeOfDay(){
	int time = ((int) Time % 86400);
	return time;
}



int partOfDay(int time) {
	if ((time >= 4*3600) && (time <= 8*3600+1800)) {
		return 1;
	} else if ((time > (8*3600+1800)) && (time < 21*3600)) {
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
		for (int i = 0; i < amountOfStations; i++) {
			this->currentTime = TimeOfDay();
			Seize(Stations[i]);
			currentStation = i;
			Wait(30);
			Release(Stations[i]);
			currentStation = -1;
			if (i < amountOfStations-1)
				Wait(routes[i]);

			if (i == amountOfStations-1) {
				this->getStore()->Leave((this->getStore()->Used()));

			}
		}
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


int getTrainInStation(int station) {
	//Print("Looking for station: %d\n", station);
	//Print("Amount of Trains: %d\n", trains.size());

	for (unsigned int i = 0; i < trains.size(); i++) {
		if ((trains.at(i)->getCurrentStation() == station)) {
			return i;
		}
	}
	return -1;
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
		inTrain = false;


		Into(waitingRooms[station]);
enterTrain:
		int nearest = getTrainInStation(station);
		if (Stations[station].Busy() && !trains.at(nearest)->isFull()) {
			Print("Vychazim z cekarny\n");
			std::cout << "Pocet cekajicich: " << waitingRooms[station].Length() << " v cekarne: " << station << std::endl;
			if (waitingRooms[station].Length() > 0) {
				waitingRooms[station].GetFirst();
				Enter(*trains.at(nearest)->getStore());
			}
			Print("Vstupuji do vlaku\n");
			inTrain = true;
		} else {
			Wait(1);
			goto enterTrain;
		}


		for (int i = station+1; i < amountOfStations; i++) {

			double vystup = Random();
		test:
			if (Stations[i].Busy() && isInTrain() && vystup <= 0.1 ) {
				Leave(*trains.at(getTrainInStation(i))->getStore());
				this->inTrain = false;
			} else {
				Wait(1);
				goto test;
			}
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
	Init(0, 2*86400);

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
		Print("Vlak: %d\n", i);
		trains.at(i)->getStore()->Output();
	}

	return 0;
};