#include "simlib.h"
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>




const double TrasaA = 10.0; // doba cesty na trase Bucovice - Slavkov
const double TrasaB = 25.0; // doba cesty na trase Slavkov - Brno
const double Spicka = 180.5; // doba trvani spicky v minutach
const double Nespicka = 780.5; // doba trvani nespicky v minutach(cas mezi spickou a noci)
const double Noc = 420.0; // doba trvani noci v minutach(zadne vlaky nejezdi)

const double SpickaIntervalVlaku = 60.0; // interval mezi vlaky ve spicce
const double NespickaIntervalVlaku = 120.0; // interval mezi vlaky mimo spickuS

const double VlakyVeSpicce = Spicka/SpickaIntervalVlaku;
const double VlakyMimoSpicku = Nespicka/NespickaIntervalVlaku;

const int CestujiciVeseli = 550;
const int CestujiciBucovice = 250;
const int CestujiciSlavkov = 200;

const int PocetStanic = 3;
Queue cekaniBucovice("Cekani ve stanici Bucovice");
Queue cekaniSlavkov("Cekani ve stanici Slavkov");

Facility Stanice[PocetStanic];

Histogram Table("Table", 0,25,20);

int TimeOfDay(){
	int time = ((int) Time % 86400);
	return time;
}

class Cestujici : public Process {
public:
	Cestujici(int Stanice) : Process() {
		stanice = Stanice;
	}

	void Behavior() {
		double Prichod = Time;

		if (stanice == 0) {
			Into(cekaniBucovice);
			Wait(Exponential(10));
		} else if (stanice == 1) {
			Into(cekaniSlavkov);
			Wait(Exponential(10));
		}
		Table(Time-Prichod);
	}

	int stanice;
};


class Vlak : public Process {

};


class Generator : public Event {
public:
	Generator(int Stanice) : Event() {
		stanice = Stanice; // stanice 0 (false) jsou Bucovice, stanice 1 (true) je Slavkov
	}

	void Behavior() {
		int time = TimeOfDay();
		(new Cestujici(stanice))->Activate();
		if ((time >= 5*3600) && (time < 8*3600+1800)) {
			if (stanice == 0) {
				Activate(Time+Normal(92, 92));
			} else if (stanice == 1) {
				Activate(Time+Normal(116,116));
			}
		} else if ((time >= (8*3600+1800)) && (time < 22*3600)) {
			if (stanice == 0) {
				Activate(Time+Normal(400, 400));
			} else if(stanice == 1) {
				Activate(Time+Normal(514, 514));
			}
		} else {
			// wtf v noci nechodi s nejakym timem?
		}
	}

	bool stanice;

};

int main() {
	Print("Model vlakove trasy Bucovice - Brno\n");
	SetOutput("model.out");
	RandomSeed(time(NULL));
	Init(0,3600*24);
	(new Generator(false))->Activate();
	(new Generator(true))->Activate();
	Run();
	Table.Output();
	cekaniBucovice.Output();
	cekaniSlavkov.Output();
	return 0;
}