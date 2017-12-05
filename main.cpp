
#include "simlib.h"
#include "main.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <vector>
#include <cmath>

std::vector<Train*> trains;

Queue waitingRooms[amountOfStations-1];

Facility Stations[amountOfStations];

Histogram Table("Cestujici", 0, 600,20);
Histogram Trains("Vlaky", 0, 3600,20);


int TimeOfDay() {
	int time = ((int) Time % 86400);
	return time;
}


int getPartOfDay(int time) {
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
	Train(int time, unsigned int size) : Process() {
		initDepartureTime = time;
		vectorIndex = size;
		store = new Store((unsigned int) round(amountOfWagons*amountOfSpacesToSitInWagon));
		currentTime = initDepartureTime;
	}


	void Behavior() {

		currentStation = -1;
		for (int i = 0; i < amountOfStations; i++) {
			this->currentTime = TimeOfDay();
			Seize(Stations[i]);
			currentStation = i;
			Wait(this->store->Free() > waitingRooms[i].Length() ? ((waitingRooms[i].Length()+1)/amountOfEntersIntoWagon/amountOfWagons*timeToEnter) : ((this->store->Free()+1)/amountOfEntersIntoWagon/amountOfWagons*timeToEnter));
			Release(Stations[i]);
			currentStation = -1;
			if (i < amountOfStations-1)
				Wait(routes[i]);

			if (i == amountOfStations-1) {
				this->getStore()->Leave((this->getStore()->Used()));
			}
		}

		if (trains.size() == getVectorIndex()) {
			trains.clear();
		}
	}

	unsigned int getVectorIndex() const {
		return vectorIndex;
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
	unsigned int vectorIndex;
	int initDepartureTime;
	Store *store;
	int currentTime;
	int currentStation;
};


int getTrainInStation(int station) {
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
		if (Stations[station].Busy() && !trains.at((unsigned long) nearest)->isFull()) {
			if (waitingRooms[station].Length() > 0) {
				waitingRooms[station].GetFirst();
				Enter(*trains.at((unsigned long) nearest)->getStore());
			}
			inTrain = true;
		} else {
			Wait(1);
			goto enterTrain;
		}


		for (int i = station+1; i < amountOfStations; i++) {

			double vystup = Random();
		test:
			if (Stations[i].Busy() && isInTrain() && vystup <= 0.1 ) {
				Leave(*trains.at((unsigned long) getTrainInStation(i))->getStore());
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
		if (getPartOfDay(time) == 1) {
			Activate(Time+Exponential(PeakTimeInterval/((passengers[station]*PeakTimeParameter)/PeakTime)));
		} else if (getPartOfDay(time) == 2) {
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
				vlak = (new Train(departures[i], trains.size()));
				trains.push_back(vlak);
				vlak->Activate(Time);
				Print("Generating train.. -> ");
				Print("i: %d\n", i);
				if (i == (size -1)) {
					i = -1;
				}
			} else {
				Wait(1);
				goto prijezd;
			}
		}
	}

};


bool parseArguments(int argc, char *argv[]) {
	return true;
}


int main(int argc, char *argv[]) {

	parseArguments(argc, argv);
	double x = 1.0;

	Print("Model vlakove trasy Bucovice - Brno\n");
	RandomSeed(time(NULL));
	Init(0, (DAY*x));

	(new PassengerGenerator(0))->Activate();
	(new PassengerGenerator(1))->Activate();
	(new PassengerGenerator(2))->Activate();
	(new TrainGenerator())->Activate();
	Run();
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