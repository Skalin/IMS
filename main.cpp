
#include <cstring>
#include "simlib.h"
#include "main.h"

std::vector<int> departures (departureTimes, departureTimes + sizeof(departureTimes) / sizeof(departureTimes[0]));
std::vector<Train*> trains;

std::string namesOfStations[amountOfStations] = {"Veseli", "Bucovice", "Slavkov", "Brno"};

Queue waitingRooms[amountOfStations-1];

int passengersLeftInTrain = 0;

Facility Stations[amountOfStations];

Histogram Table("Cestujici", 0, HOUR/6, 20);
Histogram Trains("Vlaky", 0, HOUR, 20);

void printHelp() {}


void throwException(const char *message) {
	std::cerr << message << std::endl;
	exit(EXIT_FAILURE);
}


int TimeOfDay() {
	int time = ((int) Time % 86400);
	return time;
}



std::string getNameOfStation(int station) {
	return namesOfStations[station];
}


int getPartOfDay(int time) {
	if ((time >= 4*HOUR) && (time <= 8*HOUR+30*MIN)) {
		return 1;
	} else if ((time > (8*HOUR+30*MIN)) && (time < 21*HOUR)) {
		return 2;
	} else {
		return 0;
	}
}


void insertNewTrain(int time) {

}


class Train : public Process {
public:
	Train(int time, unsigned int size) : Process() {
		initDepartureTime = time;
		vectorIndex = size;
		store = new Store((amountOfWagons)*amountOfSpacesToSitInWagon);
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
			this->filledIn[i] = this->getUsed();
			this->currentTime = TimeOfDay();
			currentStation = -1;
			if (i < amountOfStations-1) {
				double usage = 100*(double)this->getUsed()/(double)this->getCapacity();
				Print("| Train starting at %02d:%02d | left the station:\t%s at %02d:%02d \t| used: %d\t| capacity: %d\t| usage: %.2f % \t\t\t\t|\n", getInitDepartureTime()/HOUR, (getInitDepartureTime()%HOUR)/MIN, getNameOfStation(i).c_str(), getCurrentTime()/HOUR, (getCurrentTime()%HOUR)/MIN, this->getUsed(), this->getCapacity(), usage);
				Wait(routes[i]);
			}

			if (i == amountOfStations-1) {
				passengersLeaveTrain();
				double usage = 100*(double)this->getUsed()/(double)this->getCapacity();
				Print("| Train starting at %02d:%02d | ended in station:\t%s at %02d:%02d \t\t| used: %d\t| capacity: %d\t| usage: %.2f %\t\t\t\t\t|\n", getInitDepartureTime()/HOUR, (getInitDepartureTime()%HOUR)/MIN, getNameOfStation(i).c_str(), getCurrentTime()/HOUR, (getCurrentTime()%HOUR)/MIN, this->getUsed(), this->getCapacity(), usage);
				double trainFullness = this->getTrainFullness();
				if(trainFullness >= 70.0) {
					Print("| Train starting at %02d:%02d will be fulfilled on majority of route and therefore it is good to run it on this track with a coefficient of: %.1f \t\t|\n", getInitDepartureTime()/HOUR, (getInitDepartureTime()%HOUR)/MIN, trainFullness);
				} else {
					Print("| Train starting at %02d:%02d will not be fulfilled on majority of route and therefore it is not good to run it in this time with a coefficient of: %.1f \t|\n", getInitDepartureTime()/HOUR, (getInitDepartureTime()%HOUR)/MIN, trainFullness);
				}
			}
		}

		if (trains.size() == getVectorIndex()) {
			trains.clear();
		}
	}

	int getInitDepartureTime() const {
		return initDepartureTime;
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



	void passengersLeaveTrain() {
		while(!this->store->Empty()) {
			try {
				this->store->Leave(1);
			} catch (std::exception const &e) {
				passengersLeftInTrain += 1;
			}

		}}

	double getTrainFullness() const {
		double sumOfFullness = 0;
		double sumOfCapacity = 0;
		for (unsigned int i = 0; i < amountOfStations-1; i++) {
			sumOfCapacity += this->getCapacity()*(int)routes[i];
			sumOfFullness += filledIn[i]*(int)routes[i];
		}

		return (100*sumOfFullness/sumOfCapacity);
	}

	unsigned long getUsed() const {
		return this->getStore()->Used();
	}

	unsigned long getCapacity() const {
		return this->getStore()->Capacity();
	}

	bool isFull() {
		return this->store->Full();
	}
private:
	unsigned long filledIn[amountOfStations];
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
			Activate(Time+Normal(HOUR, 55));
		}

		(new Passenger(station))->Activate();
	}

	int station;

};

class TrainGenerator : public Process {
	void Behavior() {
		Train* vlak;
		unsigned long size = departures.size();
		for (int i = 0; (unsigned) i < size; ++i) {
		prijezd:
			int time = TimeOfDay();
			if (time == departures.at(i)) {
				vlak = (new Train(departures.at(i), trains.size()));
				trains.push_back(vlak);
				vlak->Activate(Time);
				if ((unsigned)i == (size-1)) {
					i = -1;
				}
			} else {
				Wait(MIN);
				goto prijezd;
			}
		}
	}

};


void parseArguments(int argc, char *argv[], int *newTrainTime, int *arg, bool *newTrain) {

	for (int i = 1; i < argc; i++){
		if (strcmp(argv[i],"-h") == 0 || strcmp(argv[i], "--help") == 0) {
			printHelp();
		}

		if (strcmp(argv[i], "-t") == 0) {
			*arg = i+1;
			char *str = (argv[*arg]);
			if (strlen(str) > 4 || strlen(str) < 4) {
				throwException("Wrong format of time, please enter time in format: HHMM");
			} else {
				int hours = 0;
				int minutes = 0;
				for (unsigned int i = 0; i < strlen(str); i++) {
					if (i == 0) {
						hours += ((int) (str[i]-48))*10;
					} else if (i == 1) {
						hours += ((int) (str[i]-48));
					} else if (i == 2) {
						minutes += ((int) (str[i]-48))*10;
					} else {
						minutes += ((int) (str[i]-48));
					}
					*newTrainTime = HOUR*hours+MIN*minutes;
				}
				*newTrain = true;
			}
		}
	}
}


int main(int argc, char *argv[]) {

	/* Default value of amount of days the simulation will run */
	double amountOfDays = 1.0;

	int trainTime = 0;
	bool newTrainFlag = false;
	int arg = 1;

	parseArguments(argc, argv, &trainTime, &arg, &newTrainFlag);

	Print("Model vlakove trasy Bucovice - Brno\n");
	if (newTrainFlag) {
		insertNewTrain(trainTime);
		Print("Simulace se pokusi vlozit novy vlak v case: %s\n", argv[arg]);
	}

	RandomSeed(time(NULL));
	Init(0, (DAY*amountOfDays));

	// Passenger and train generators
	(new PassengerGenerator(0))->Activate();
	(new PassengerGenerator(1))->Activate();
	(new PassengerGenerator(2))->Activate();
	(new TrainGenerator())->Activate();
	Run();
	Stations[0].Output();
	Stations[1].Output();
	Stations[2].Output();
	Stations[3].Output();
	for (unsigned int i = 0; i < sizeof(waitingRooms)/sizeof(waitingRooms[0]); i++) {
		waitingRooms[i].Output();
	}/*
	for (unsigned int i = 0; i < trains.size(); i++) {
		trains.at(i)->getStore()->Output();
	}*/

	return 0;
};