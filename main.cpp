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

const double pocetLidiVeSpicce = 0.54; // procent lidi ve spicce
const double VlakyVeSpicce = Spicka/SpickaIntervalVlaku;
const double VlakyMimoSpicku = Nespicka/NespickaIntervalVlaku;

const int CestujiciVeseli = 550;
const int CestujiciBucovice = 250;
const int CestujiciSlavkov = 200;

int spickaPrijezdyBucovice[] = {5*3600, 6*3600, 7*3600, 8*3600};

Store Vagony("Vlak", 100);

const int PocetStanic = 3;
Queue cekaniBucovice("Cekani ve stanici Bucovice");
Queue cekaniSlavkov("Cekani ve stanici Slavkov");

Facility Stanice[PocetStanic];

Histogram Table("Table", 0,25,20);
int TimeOfDay(){
	int time = ((int) Time % 86400);
	return time;
}

int nejblizsiVlakVeSpicce(int time, int *array) {
	int nearest = 0;
	for (unsigned int i = 1; i < (sizeof(array)/sizeof(array[0])); i++) {
		if ((array[i] - time) < (array[i - 1] - time)) {
			nearest = i;
		}
	}
	return nearest;
}

int dalsiVlak(int time, int *array) {
	return array[nejblizsiVlakVeSpicce(time, array)];
}

int castDne(int time) {
	if ((time >= 5*3600) && (time < 8*3600+1800)) {
		return 1;
	} else if ((time >= (8*3600+1800)) && (time < 22*3600)) {
		return 2;
	} else {
		return 0;
	}
}


class Vlak : public Process, public Store {

	void Behavior() {
		double Prijezd = Time;

		Seize(Stanice[0]);
		Wait(Time+cekaniBucovice.Length()*4);
		Release(Stanice[0]);
		Wait(Time+TrasaA*60);

		Seize(Stanice[1]);
		Wait(Time+cekaniSlavkov.Length()*4);
		Release(Stanice[1]);
		Wait(Time+TrasaB*60);

		Seize(Stanice[2]);
		Wait(Time+Vagony.Used()*4);
		Release(Stanice[2]);


		Table(Time-Prijezd);
	}
};



class Cestujici : public Process {
public:
	Cestujici(int Stanice) : Process() {
		stanice = Stanice;
	}

	void Behavior() {
		double Prichod = Time;


		if (Stanice[stanice].Busy() && !Vagony.Full()) { // pokud je stanice busy, vlak je ve stanici
			Enter(Vagony);
		} else {
			if (stanice == 0)
				Into(cekaniBucovice);
			else if (stanice == 1)
				Into(cekaniSlavkov);
			Passivate();
			WaitUntil(Stanice[stanice].Busy() && !Vagony.Full());
			Enter(Vagony);
		}
		// cestujici musi s urcitou pravdepodobnosti do vlaku nastoupit a vystoupit (0.1 pravdepodobnost)
		double vystup = Random();

		if (vystup <= 0.1) {
			Leave(Vagony);
		} // jinak zustane ve vlaku

		Table(Time-Prichod);
	}

	int stanice;
};


class GeneratorCestujici : public Event {
public:
	GeneratorCestujici(int Stanice) : Event() {
		stanice = Stanice; // stanice 0 je Bucovice, stanice 1 je Slavkov, 2 je Brno (negenerujeme nic)
	}

	void Behavior() {
		int time = TimeOfDay();
		if (castDne(time) == 1) {
			if (stanice == 0) {
			 	Activate(Time+Exponential((int) SpickaIntervalVlaku/((CestujiciBucovice*pocetLidiVeSpicce)/Spicka)));
			} else if (stanice == 1) {
				Activate(Time+Exponential((int) SpickaIntervalVlaku/((CestujiciSlavkov*pocetLidiVeSpicce)/Spicka)));
			}
		} else if (castDne(time) == 2) {
			if (stanice == 0) {
				Activate(Time+Exponential((int) NespickaIntervalVlaku/((CestujiciBucovice*(1.00-pocetLidiVeSpicce))/Nespicka)));
			} else if(stanice == 1) {
				Activate(Time+Exponential((int) NespickaIntervalVlaku/((CestujiciSlavkov*(1.00-pocetLidiVeSpicce))/Nespicka)));
			}
		} else {
			Activate(Time+Normal(3600, 55));
		}

		(new Cestujici(stanice))->Activate();
	}

	int stanice;

};



class GeneratorVlak : public Event {
	void Behavior() {

		int time = TimeOfDay();
		if (castDne(time) == 1 || castDne(time) == 2) {

			if (castDne(time) == 1) {
				(new Vlak())->Activate();
				Activate(Time+SpickaIntervalVlaku*60);
			} else if (castDne(time == 2)) {
				(new Vlak())->Activate();
				Activate(Time+NespickaIntervalVlaku*60);
			} else {
				Activate(Time+Noc*60);
			}

		}
	}
};

int main() {
	Print("Model vlakove trasy Bucovice - Brno\n");
	SetOutput("model.out");
	RandomSeed(time(NULL));
	Init(0,86400);
	(new GeneratorCestujici(0))->Activate();
	(new GeneratorCestujici(1))->Activate();
	(new GeneratorVlak())->Activate();
	Run();
	Table.Output();
	cekaniBucovice.Output();
	cekaniSlavkov.Output();
	Vagony.Output();

	return 0;
}