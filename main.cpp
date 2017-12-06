
#include "simlib.h"
#include "main.h"

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
	std::cout << "\t\t-p MINUTES\t\tAktivaci tohoto parametru se upravi interval vyjezdu vlaku ve spicce. Vlaky ve vychozim stavu jezdi ve spicce každych " << PeakTimeIntervalTrain/MIN << " minut, jinak vyjíždí podle tohoto argumentu."  << std::endl;
	std::cout << "\t\t-n MINUTES\t\tAktivaci tohoto parametru se upravi interval vyjezdu vlaku mimo spicku. Vlaky ve vychozim stavu jezdi mimo spicku kazdych " << NonPeakTimeIntervalTrain/MIN << " minut, jinak vyjíždí podle tohoto argumentu."  << std::endl;
	exit(EXIT_SUCCESS);
}

int safe_toInt(std::string s) {
	int ret;
	std::stringstream sStream(s);
	for (char c:s) {
		if (!isdigit(c)) {
			throw std::invalid_argument("unknown argument value");
		}
	}
	if (sStream >> ret) {
		return ret;
	} else {
		throw std::invalid_argument("unknown argument value");
	}
}


/*
 * Function parses arguments. If no arguments are given, program starts standard simulation. If -h or --help is given, program prints help and ends its work. If -t with HHMM value is passed, the program creates new train at the time given in HHMM
 *
 * @param int argc
 * @param char *argv[]
 */
void parseArguments(int argc, char *argv[], int *peakTimeInterval, bool *peakTimeFlag, int *nonPeakTimeInterval, bool *nonPeakTimeFlag) {

	for (int i = 1; i < argc; i++){
		if (strcmp(argv[i],"-h") == 0 || strcmp(argv[i], "--help") == 0) {
			printHelp();
		}

		if (strcmp(argv[i], "-p") == 0) {
			int tmp = *peakTimeInterval;
			try {
				tmp = safe_toInt(argv[i+1]);
			} catch (std::exception const &exception) {
				throwException("ERROR: Wrong format of peak time interval");
			}
			*peakTimeInterval = tmp*MIN;
			*peakTimeFlag = true;
		}

		if (strcmp(argv[i], "-n") == 0) {
			int tmp = *nonPeakTimeInterval;
			try {
				tmp = safe_toInt(argv[i+1]);
			} catch (std::exception const &exception) {
				throwException("ERROR: Wrong format of non-peak time interval");
			}
			*nonPeakTimeInterval = tmp*MIN;
			*nonPeakTimeFlag = true;
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
int TimeOfDay(double timeDouble) {
	int time = ((int) timeDouble % 86400);
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
	} else if ((time > (8*HOUR+30*MIN)) && (time <= 21*HOUR+2*MIN)) {
		return 2;
	} else {
		return 0;
	}
}


class Train : public Process {
public:
	explicit Train(int time) : Process() {
		this->initDepartureTime = time;
		this->store = new Store((amountOfWagons)*amountOfSpacesToSitInWagon);
		this->currentTime = initDepartureTime;
		this->currentStation = -1;
		this->passengersLeft = 0;
		this->entered = 0;
	}


	void Behavior() override {
		for (int i = 0; i < amountOfStations; i++) {
			this->setEntered(0);
			Seize(Stations[i]);
			if (this->getUsed() > 0) {
				this->setPassengersLeft(passengersLeaveTrain());
			}
			this->setCurrentStation(i);
			if (currentStation != amountOfStations-1  && !waitingRooms[currentStation].Empty()) {
				while (!waitingRooms[currentStation].Empty() && !this->store->Full()) {
					waitingRooms[currentStation].GetFirst()->Activate();
					this->setEntered(this->getEntered()+1);
				}
			}
			Wait(((this->passengersLeft+entered)/(amountOfWagons*amountOfEntersIntoWagon)*timeToEnter) + 20); // vlak ceka ve stanici po dobu nastupovani
			this->setFilledIn(i, this->getUsed());
			this->setCurrentTime(TimeOfDay(Time));
			Release(Stations[i]);
			this->setCurrentStation(-1);
			if (i < amountOfStations-1) {
				double usage = 100*(double)this->getUsed()/(double)this->getCapacity();
				Print("| Train starting at %02d:%02d | left the station:\t%s at %02d:%02d \t| used: %d\t| capacity: %d\t| usage: %.2f % \t\t\t\t|\n", getInitDepartureTime()/HOUR, (getInitDepartureTime()%HOUR)/MIN, getNameOfStation(i).c_str(), getCurrentTime()/HOUR, (getCurrentTime()%HOUR)/MIN, this->getUsed(), this->getCapacity(), usage);
				Wait(routes[i]);
			}

			if (i == amountOfStations-1) {
				this->AllPassengersLeaveTrain();
				double usage = 100*(double)this->getUsed()/(double)this->getCapacity();
				Print("| Train starting at %02d:%02d | ended in station:\t%s at %02d:%02d \t\t| used: %d\t| capacity: %d\t| usage: %.2f %\t\t\t\t\t|\n", getInitDepartureTime()/HOUR, (getInitDepartureTime()%HOUR)/MIN, getNameOfStation(i).c_str(), getCurrentTime()/HOUR, (getCurrentTime()%HOUR)/MIN, this->getUsed(), this->getCapacity(), usage);
				double trainFullness = this->getTrainFullness();
				if(trainFullness >= 70.0) {
					Print("| Train starting at %02d:%02d will be fulfilled on majority of route and therefore it is good to run it on this track with a coefficient of: %.1f \t\t|\n", getInitDepartureTime()/HOUR, (getInitDepartureTime()%HOUR)/MIN, trainFullness);
				} else {
					Print("| Train starting at %02d:%02d will not be fulfilled on majority of route and therefore it is not good to run it in this time with a coefficient of: %.1f \t|\n", getInitDepartureTime()/HOUR, (getInitDepartureTime()%HOUR)/MIN, trainFullness);
				}
				this->removeTrain();
			}
		}
	}


	void removeTrain() {
		trains.erase(trains.begin());
	}

	void setPassengersLeft(int passengersLeft) {
		Train::passengersLeft = passengersLeft;
	}

	void setInitDepartureTime(int initDepartureTime) {
		Train::initDepartureTime = initDepartureTime;
	}

	void setStore(Store *store) {
		Train::store = store;
	}

	void setCurrentTime(int currentTime) {
		Train::currentTime = currentTime;
	}

	void setCurrentStation(int currentStation) {
		Train::currentStation = currentStation;
	}

	void setEntered(int entered) {
		Train::entered = entered;
	}

	void setFilledIn(int i, unsigned long amount) {
		this->filledIn[i] = amount;
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

	int getPassengersLeft() const {
		return passengersLeft;
	}

	int getEntered() const {
		return entered;
	}

	Store *getStore() const {
		return store;
	}

	double getTrainFullness() const {
		double sumOfFullness = 0;
		double sumOfCapacity = 0;
		for (unsigned int i = 0; i < amountOfStations-1; i++) {
			sumOfCapacity += this->getCapacity()*(int)routes[i];
			sumOfFullness += this->getFilledIn(i)*(int)routes[i];
		}

		return (100*sumOfFullness/sumOfCapacity);
	}

	unsigned long getUsed() const {
		return this->getStore()->Used();
	}

	unsigned long getCapacity() const {
		return this->getStore()->Capacity();
	}

	unsigned long getFilledIn(int i) const {
		return filledIn[i];
	}

	bool isFull() {
		return this->store->Full();
	}

	int passengersLeaveTrain() {
		unsigned long passengers = this->getUsed();
		int passengersLeft = 0;
		for (unsigned long i = 0; i < passengers; i++) {
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

private:
	int passengersLeft;
	unsigned long filledIn[amountOfStations];
	int initDepartureTime;
	Store *store;
	int currentTime;
	int currentStation;
	int entered;
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
	explicit Passenger(int Station) : Process() {
		station = Station;
	}


	void Behavior() override {

		int time = TimeOfDay(Time);

		if (getPartOfDay(time) == 0) {
			Wait(HOUR);
			time = TimeOfDay(Time);
			if (getPartOfDay(time) == 0) {
				goto leave;
			}
		}
		Into(waitingRooms[station]);
	enterTrain:
		Passivate();
		if (Stations[station].Busy() && !trains.at((unsigned) getTrainInStation(station))->isFull()) {
			Enter(*trains.at((unsigned) getTrainInStation(station))->getStore());
		} else {
			goto enterTrain;
		}
	leave:;
	}
	int station;
};



class PassengerGenerator : public Event {
public:
	explicit PassengerGenerator(int Station) : Event() {
		station = Station; // stanice 0 je Veseli, stanice 1 je Bucovice, 2 je Slavkov (negenerujeme nic)
	}

	void Behavior() override {
		int time = TimeOfDay(Time);
		if (getPartOfDay(time) == 1) {
			Activate(Time+Exponential(PeakTimeIntervalPassenger/MIN/((passengers[station]*PeakTimeParameter)/PeakTime)));
		} else if (getPartOfDay(time) == 2) {
			Activate(Time+Exponential(NonPeakTimeIntervalPassenger/MIN/((passengers[station]*(1.00-PeakTimeParameter))/NonPeakTime)));
		} else {
			Activate(Time+Normal(HOUR, 55));
		}

		(new Passenger(station))->Activate();
	}
	int station;
};


class TrainGenerator : public Event {

	void Behavior() override {
		int time = TimeOfDay(Time);
		if (getPartOfDay(time) == 1 && ((time > 4*HOUR+50*MIN && time < 5*HOUR+1*MIN) || (time > 6*HOUR+1*MIN))) {
			auto *train = (new Train(time));
			trains.push_back(train);
			train->Activate(Time);
			Activate(Time+PeakTimeIntervalTrain);
		} else if (getPartOfDay(time) == 2) {
			auto *train = (new Train(time));
			trains.push_back(train);
			train->Activate(Time);
			Activate(Time+NonPeakTimeIntervalTrain);
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

	bool modifiedPeakTimeInterval = false;
	bool modifiedNonPeakTimeInterval = false;
	parseArguments(argc, argv, &PeakTimeIntervalTrain, &modifiedPeakTimeInterval, &NonPeakTimeIntervalTrain, &modifiedNonPeakTimeInterval);

	Print("Model vlakove trasy Veseli n.M. - Brno\n");
	if (modifiedPeakTimeInterval) {
		Print("Simulace nebezi s vychozim casem intervalu vyjezdu vlaku ve spicce, vlaky ve spicce vyjedou kazdych: %d minut\n", PeakTimeIntervalTrain/MIN);
	}
	if (modifiedNonPeakTimeInterval) {
		Print("Simulace nebezi s vychozim casem intervalu vyjezdu vlaku mimo spicku, vlaky mimo spicku vyjedou kazdych: %d minut\n", NonPeakTimeIntervalTrain/MIN);
	}

	RandomSeed(time(nullptr));
	Init(0, (DAY*amountOfDays));

	// Passenger and train generators
	(new TrainGenerator())->Activate();
	(new PassengerGenerator(0))->Activate();
	(new PassengerGenerator(1))->Activate();
	(new PassengerGenerator(2))->Activate();

	// Run simulation and print fancy output
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