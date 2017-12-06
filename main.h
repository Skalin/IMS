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

const double routes[3] = {60.0*MIN, 10.0*MIN, 25.0*MIN}; // doba cesty na jednotlivych usecich - Veseli - Bucovice, Bucovice - Slavkov, Slavkov - Brno
const double PeakTime = 3.5*MIN; // doba trvani spicky v minutach
const double NonPeakTime = 13.5*MIN; // doba trvani nespicky v minutach (cas mezi spickou a noci)

int PeakTimeIntervalPassenger = HOUR; // interval mezi vlaky ve spicce
int NonPeakTimeIntervalPassenger = 2*HOUR; // interval mezi vlaky mimo spicku
int PeakTimeIntervalTrain = PeakTimeIntervalPassenger; // interval mezi vlaky ve spicce
int NonPeakTimeIntervalTrain = NonPeakTimeIntervalPassenger; // interval mezi vlaky mimo spicku
const int timeToEnter = 2;
const int amountOfWagons = 3;
const int amountOfSpacesToSitInWagon = 34;
const int amountOfEntersIntoWagon = amountOfWagons*2;

const double PeakTimeParameter = 0.54; // procent lidi ve spicce

const int passengers[3] = {550, 250, 200};
const int amountOfStations = 4;

void printHelp();
int safe_toInt(std::string s);
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
