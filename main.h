//
// Created by skalin on 2.12.17.
//

#ifndef IMS_MAIN_H
#define IMS_MAIN_H



const int DAY = 86400;
const int HOUR = 3600;
const int MIN = 60;

const double routes[3] = {60.0*MIN, 10.0*MIN, 25.0*MIN}; // doba cesty na jednotlivych usecich - Veseli - Bucovice, Bucovice - Slavkov, Slavkov - Brno
const double PeakTime = 180.5; // doba trvani spicky v minutach
const double NonPeakTime = 780.5; // doba trvani nespicky v minutach(cas mezi spickou a noci)

const double PeakTimeInterval = HOUR/MIN; // interval mezi vlaky ve spicce
const double NonPeakTimeInterval = 2*HOUR/MIN; // interval mezi vlaky mimo spicku
const int timeToEnter = 2;
const int amountOfWagons = 3;
const double amountOfSpacesToSitInWagon = 34;
const int amountOfEntersIntoWagon = amountOfWagons*2;

const double PeakTimeParameter = 0.54; // procent lidi ve spicce
const double TrainsInPeakTime = PeakTime/PeakTimeInterval;
const double TrainsInNonPeakTime = NonPeakTime/NonPeakTimeInterval;

const int passengers[3] = {550, 250, 200};
const int amountOfStations = 4;

int departures[] = {4*HOUR+50*MIN, 6*HOUR+MIN, 7*HOUR+MIN, 9*HOUR+MIN, 11*HOUR+MIN, 13*HOUR+MIN, 15*HOUR+MIN, 17*HOUR+MIN, 19*HOUR+MIN, 21*HOUR+MIN};

int getPartOfDay(int time);
int TimeOfDay();

class Passenger;
class Train;
class PassengerGenerator;
class TrainGenerator;

#endif //IMS_MAIN_H
