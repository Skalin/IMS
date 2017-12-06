//
// Created by skalin on 2.12.17.
//

#ifndef IMS_MAIN_H
#define IMS_MAIN_H

#include <iostream>
#include <cstring>
#include <sstream>
#include <vector>

const int DAY = 86400;
const int HOUR = 3600;
const int MIN = 60;

const double ROUTES[3] = {60.0*MIN, 10.0*MIN, 25.0*MIN}; // doba cesty na jednotlivych usecich - Veseli - Bucovice, Bucovice - Slavkov, Slavkov - Brno
const double PEAK_TIME = 3.5*MIN; // doba trvani spicky v minutach
const double NON_PEAK_TIME = 13.5*MIN; // doba trvani nespicky v minutach (cas mezi spickou a noci)

const int PEAK_TIME_INTERVAL_PASSENGER = HOUR; // interval mezi vlaky ve spicce
const int NON_PEAK_TIME_INTERVAL_PASSENGER = 2*HOUR; // interval mezi vlaky mimo spicku
int PeakTimeIntervalTrain = PEAK_TIME_INTERVAL_PASSENGER; // interval mezi vlaky ve spicce
int NonPeakTimeIntervalTrain = NON_PEAK_TIME_INTERVAL_PASSENGER; // interval mezi vlaky mimo spicku
const int TIME_TO_ENTER = 2; // doba vstupu jednoho cestujiciho do vlaku v sekundach
const int AMOUNT_OF_WAGONS = 3; // pocet vagonu na jeden vlak
const int AMOUNT_OF_SPACES_TO_SIT_IN_WAGON = 34; // pocet mist ve vagonu
const int AMOUNT_OF_ENTERS_INTO_TRAIN = AMOUNT_OF_WAGONS*2; // pocet vstupu do VLAKU (vzdy dva vstupy na jeden vagon)

const double PEAK_TIME_PARAMETER = 0.54; // procent lidi ve spicce

const int AMOUNT_OF_STATIONS = 4; // celkovy pocet stanic
const int PASSENGERS[AMOUNT_OF_STATIONS-1] = {550, 250, 200}; // priblizny pocet pasazeru ve stanicich Veseli ([0]), Bucovice ([1]) a Slavkov([2])

void printHelp();
int stringToInt(std::string s);
void parseArguments(int argc, char *argv[], int *peakTimeInterval, bool *peakTimeFlag, int *nonPeakTimeInterval, bool *nonPeakTimeFlag);
int getPartOfDay(int time);
int TimeOfDay(double timeDouble);
std::string getNameOfStation(int station);
int getTrainInStation(int station);
void throwException(const char *message);

class Passenger;
class Train;
class PassengerGenerator;
class TrainGenerator;

#endif //IMS_MAIN_H
