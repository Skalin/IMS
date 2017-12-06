
#include <cstring>
#include "simlib.h"
#include "main.h"

std::vector<int> departures (departureTimes, departureTimes + sizeof(departureTimes) / sizeof(departureTimes[0]));
std::vector<Train*> trains;

std::string namesOfStations[amountOfStations] = {"Veseli", "Bucovice", "Slavkov", "Brno"};

Queue waitingRooms[amountOfStations-1];

int passengersLeftInTrain = 0;

Facility Stations[amountOfStations];

/*
 * Function prints help on stdout and ends program with EXIT_SUCCESS return code
 *
 */
void printHelp() {
	std::cout << "IMS 2017/2018 - Doprava zbozi nebo osob" << std::endl;
	std::cout << "Individualni zadani: Vyuziti a efektivita vlakoveho spoje: Veseli - Bucovice - Slavkov - Brno\n"  << std::endl;
	std::cout << "Autori: \tDominik Skala (xskala11) - vedouci"  << std::endl;
	std::cout << "\t\tJan Hrboticky (xhrbot01)\n"  << std::endl;
	std::cout << "Popis:"  << std::endl;
	std::cout << "\t\tProgram provadi simulaci vlakove trate mezi mesty Veseli nad Moravou, Bucovicemi, Slavkovem u Brna a Brnem. Samotny program je modelem teto trate v jazyce C++.\n\t\tZa pomoci tohoto modelu se snazime modelovat efektivitu a zaplnenost vlaku na teto trati a snazime se zjistit, zda by se zde \"neuzivil\" jeste jeden vlak.\n\n"  << std::endl;
	std::cout << "Argumenty programu:"  << std::endl;
	std::cout << "\t\t-h, --help\t\tNapoveda tohoto programu (aktualne vytistena)"  << std::endl;
	std::cout << "\t\t-t HHMM\t\t\tAktivaci tohoto parametru se prida vlak v urcity cas, pro jednodusi modelovani tohoto vlaku.\n\t\t\t\t\tCas musi byt ve formatu HHMM, kde HH jsou hodiny, ve kterych se aktivuje a MM jsou minuty ve kterych se vlak aktivuje"  << std::endl;
	exit(EXIT_SUCCESS);
}


/*
 * Function parses arguments. If no arguments are given, program starts standard simulation. If -h or --help is given, program prints help and ends its work. If -t with HHMM value is passed, the program creates new train at the time given in HHMM
 *
 * @param int argc
 * @param char *argv[]
 * @param int *newTrainTime
 * @param int *arg
 * @param bool *newTrain
 */
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
				for (unsigned int j = 0; j < strlen(str); j++) {
					if (j == 0) {
						hours += ((int) (str[j]-48))*10;
					} else if (j == 1) {
						hours += ((int) (str[j]-48));
					} else if (j == 2) {
						minutes += ((int) (str[j]-48))*10;
					} else {
						minutes += ((int) (str[j]-48));
					}
					*newTrainTime = HOUR*hours+MIN*minutes;
				}
				*newTrain = true;
			}
		}
	}
}


/*
 * Function prints an error message on stderr and exits program
 *
 * @param const char *message message to be printed to stderr
 */
void throwException(const char *message) {
	std::cerr << message << std::endl;
	exit(EXIT_FAILURE);
}


/*
 * Function returns time of current day
 *
 * @returns integer representation of time from current day
 */
int TimeOfDay(double timerino) {
	int time = ((int) timerino % 86400);
	return time;
}


/*
 * Function returns string name of station from array namesOfStations
 *
 * @param int station integer representation of station from array namesOfStations
 * @returns std::string name of station
 */
std::string getNameOfStation(int station) {
	return namesOfStations[station];
}


/*
 * Function returns part of day. Day is split into three parts - Peak time, Non-peak time and night where as Peak time is 1, Non-peak time is 2 and night is 0
 *
 * @param int time integer value of time in current day
 * @returns int integer representation of current part of day
 */
int getPartOfDay(int time) {
	if ((time >= 4*HOUR) && (time <= 8*HOUR+30*MIN)) {
		return 1;
	} else if ((time > (8*HOUR+30*MIN)) && (time <= 21*HOUR)) {
		return 2;
	} else {
		return 0;
	}
}


/*
 * Function inserts new train in
 */
void insertNewDepartureTime(int time) {

}


class Train : public Process {
public:
	Train(int time, unsigned int size) : Process() {
		this->vectorIndex = size;
		this->initDepartureTime = time;
		this->store = new Store((amountOfWagons)*amountOfSpacesToSitInWagon);
		this->currentTime = initDepartureTime;
	}


	void Behavior() {

		currentStation = -1;
		int passengersLeft = 0;
		int entered = 0;
		for (int i = 0; i < amountOfStations; i++) {
			Seize(Stations[i]);
			if (this->getUsed() > 0) {
				passengersLeft = passengersLeaveTrain();
			}
			currentStation = i;
			if (currentStation != amountOfStations-1  && !waitingRooms[currentStation].Empty()) {
				while (!waitingRooms[currentStation].Empty() && !this->store->Full()) {
					waitingRooms[currentStation].GetFirst()->Activate();
					entered += 1;
				}
			}
			Wait((passengersLeft+entered)/amountOfWagons/amountOfEntersIntoWagon); // vlak ceka ve stanici po dobu nastupovani
			Release(Stations[i]);
			this->filledIn[i] = this->getUsed();
			this->currentTime = TimeOfDay(Time);
			currentStation = -1;
			if (i < amountOfStations-1) {
				double usage = 100*(double)this->getUsed()/(double)this->getCapacity();
				Print("| Train starting at %02d:%02d | left the station:\t%s at %02d:%02d \t| used: %d\t| capacity: %d\t| usage: %.2f % \t\t\t\t|\n", getInitDepartureTime()/HOUR, (getInitDepartureTime()%HOUR)/MIN, getNameOfStation(i).c_str(), getCurrentTime()/HOUR, (getCurrentTime()%HOUR)/MIN, this->getUsed(), this->getCapacity(), usage);
				Wait(routes[i]);
			}

			if (i == amountOfStations-1) {
				AllPassengersLeaveTrain();
				double usage = 100*(double)this->getUsed()/(double)this->getCapacity();
				Print("| Train starting at %02d:%02d | ended in station:\t%s at %02d:%02d \t\t| used: %d\t| capacity: %d\t| usage: %.2f %\t\t\t\t\t|\n", getInitDepartureTime()/HOUR, (getInitDepartureTime()%HOUR)/MIN, getNameOfStation(i).c_str(), getCurrentTime()/HOUR, (getCurrentTime()%HOUR)/MIN, this->getUsed(), this->getCapacity(), usage);
				double trainFullness = this->getTrainFullness();
				if(trainFullness >= 70.0) {
					Print("| Train starting at %02d:%02d will be fulfilled on majority of route and therefore it is good to run it on this track with a coefficient of: %.1f \t\t|\n", getInitDepartureTime()/HOUR, (getInitDepartureTime()%HOUR)/MIN, trainFullness);
				} else {
					Print("| Train starting at %02d:%02d will not be fulfilled on majority of route and therefore it is not good to run it in this time with a coefficient of: %.1f \t|\n", getInitDepartureTime()/HOUR, (getInitDepartureTime()%HOUR)/MIN, trainFullness);
				}
				trains.erase(trains.begin());
			}
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


	int passengersLeaveTrain() {
		int passengers = this->getUsed();
		int passengersLeft = 0;
		for (int i = 0; i < passengers; i++) {
			if (Random() < 0.2) {
				try {
					this->store->Leave(1);
				} catch (std::exception const &e) {
					passengersLeftInTrain += 1;
				}
			}
			passengersLeft += 1;
		}
		return passengersLeft;
	}

	void AllPassengersLeaveTrain() {
		while(!this->store->Empty()) {
			try {
				this->store->Leave(1);
			} catch (std::exception const &e) {
				passengersLeftInTrain += 1;
			}

		}
	}

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



/*
 * Function returns index of train that is currently in station
 *
 * @param int station integer representation of station
 * @returns integer representation of train that is in station given in param
 */
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
		Passivate();
		if (Stations[station].Busy() && !trains.at(getTrainInStation(station))->isFull()) {
			Enter(*trains.at(getTrainInStation(station))->getStore());
			inTrain = true;
		} else {
			goto enterTrain;
		}


/*
		Into(waitingRooms[station]);
	enterTrain:
		if (isInQueue()) {
			Print("Jsem v cekarne: %d", station);
		}
		Passivate();
		if (Stations[station].Busy() && !trains.at(getTrainInStation(station))->isFull()) {
			if (waitingRooms[station].Length() > 0) {
				Enter(*trains.at(getTrainInStation(station))->getStore());
			}
			inTrain = true;
		} else {
			goto enterTrain;
		}


*/
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
		int time = TimeOfDay(Time);
		if (getPartOfDay(time) == 1) {
			Activate(Time+Exponential(PeakTimeInterval/MIN/((passengers[station]*PeakTimeParameter)/PeakTime)));
		} else if (getPartOfDay(time) == 2) {
			Activate(Time+Exponential(NonPeakTimeInterval/MIN/((passengers[station]*(1.00-PeakTimeParameter))/NonPeakTime)));
		} else {
			Activate(Time+Normal(HOUR, 55));
		}

		(new Passenger(station))->Activate();
	}
	int station;
};


class TrainGenerator : public Event {

	void Behavior() {
		int time = TimeOfDay(Time);
		if (getPartOfDay(time) == 1 && time > 4*HOUR+50*MIN) {
			Train *train = (new Train(time, (unsigned) trains.size()));
			trains.push_back(train);
			train->Activate(Time);
			Activate(Time+PeakTimeInterval);
		} else if (getPartOfDay(time) == 2) {
			Train *train = (new Train(time,  (unsigned) trains.size()));
			trains.push_back(train);
			train->Activate(Time);
			Activate(Time+NonPeakTimeInterval);
		} else {
			Activate(Time+1);
		}
	}
};


/*
 * Main function of program
 */
int main(int argc, char *argv[]) {

	/* Default value of amount of days the simulation will run */
	double amountOfDays = 1.0;

	int trainTime = 0;
	bool newTrainFlag = false;
	int arg = 1;

	parseArguments(argc, argv, &trainTime, &arg, &newTrainFlag);

	Print("Model vlakove trasy Veseli - Brno\n");
	if (newTrainFlag) {
		Print("Simulace se pokusi vlozit novy vlak v case: %s\n", argv[arg]);
		insertNewDepartureTime(trainTime);
	}

	RandomSeed(time(NULL));
	Init(0, (DAY*amountOfDays));

	// Passenger and train generators
	(new TrainGenerator())->Activate();
	(new PassengerGenerator(0))->Activate();
	(new PassengerGenerator(1))->Activate();
	(new PassengerGenerator(2))->Activate();

	// Run simulation
	Print("|-------------------------------------------------------------------------------------------------------------------------------------------------------|\n");
	Run();
	Print("|-------------------------------------------------------------------------------------------------------------------------------------------------------|");
	// Print output for all stations and waiting rooms in these stations
	Print("\n\n");
	for (unsigned int i = 0; i < amountOfStations; i++) {
		Print("\nStanice: %s\n", getNameOfStation(i).c_str());
		Stations[i].Output();
		if (i < amountOfStations-1) {
			Print("Cekarna ve stanici: %s\n", getNameOfStation(i).c_str());
			waitingRooms[i].Output();
		}
	}

	return 0;
};