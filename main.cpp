#include "simlib.h"
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>



const double TrasaA = 1.0; // doba cesty na trase Veseli - Bucovice
const double TrasaB = 10.0; // doba cesty na trase Bucovice - Slavkov
const double TrasaC = 25.0; // doba cesty na trase Slavkov - Brno
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

const int PocetStanic = 4;
Queue cekaniVeseli("Cekani ve stanici Veseli");
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


int castDne(int time) {
	if ((time >= 5*3600) && (time < 8*3600+1800)) {
		return 1;
	} else if ((time >= (8*3600+1800)) && (time < 22*3600)) {
		return 2;
	} else {
		return 0;
	}
}


class Cestujici : public Process {
public:
	Cestujici(int Stanice) : Process() {
		stanice = Stanice;
	}

	bool isInTrain() {
		return inTrain;
	}

	void Behavior() {
		double Prichod;
		Prichod = Time;
		inTrain = false;


enterTrain:
		if (Stanice[stanice].Busy() && !isInTrain() && !Vagony.Full()) {
			Enter(Vagony);
			inTrain = true;
			Table(Time-Prichod);
		} else {
			if (!Stanice[stanice].Busy() || !Vagony.Full()) {
				if (stanice == 0) {
					Into(cekaniVeseli);
				} else if (stanice == 1) {
					Into(cekaniBucovice);
				} else {
					Into(cekaniSlavkov);
				}
				WaitUntil(Stanice[stanice].Busy() && !Vagony.Full());
				if (stanice == 0) {
					cekaniVeseli.GetFirst();
				} else if (stanice == 1) {
					cekaniBucovice.GetFirst();
				} else {
					cekaniSlavkov.GetFirst();
				}
				goto enterTrain;
			}
		}

		for (int i = stanice+1; i < PocetStanic; i++) {
			WaitUntil(inTrain && Stanice[i].Busy());
			double vystup = Random();
			if (isInTrain() && vystup <= 0.1) {
				Leave(Vagony);
				inTrain = false;
			}
		}


		if (Stanice[2].Busy()) {
			Leave(Vagony);
		}
	}

	bool inTrain;
	int stanice;
};



class Vlak : public Process {

	void Behavior() {
		double Prijezd = Time;
		Seize(Stanice[0]);
		Wait(Time+(Vagony.Free() > cekaniVeseli.Length() ? cekaniVeseli.Length() : Vagony.Free())*2);
		Release(Stanice[0]);
		Wait(Time+TrasaA*60);


		Seize(Stanice[1]);
		Wait(Time+(Vagony.Free() > cekaniBucovice.Length() ? cekaniBucovice.Length() : Vagony.Free())*2);
		Release(Stanice[1]);
		Wait(Time+TrasaB*60);

		Seize(Stanice[2]);
		Wait(Time+(Vagony.Free() > cekaniSlavkov.Length() ? cekaniSlavkov.Length() : Vagony.Free())*2);
		Release(Stanice[2]);
		Wait(Time+TrasaC*60);

		Seize(Stanice[3]);
		Wait(Time+Vagony.Used()*4);
		Release(Stanice[3]);


		Table(Time-Prijezd);
	}
};

class GeneratorCestujici : public Event {
public:
	GeneratorCestujici(int Stanice) : Event() {
		stanice = Stanice; // stanice 0 je Bucovice, stanice 1 je Slavkov, 2 je Brno (negenerujeme nic)
	}

	void Behavior() {
		double Prijezd = Time;
		int time = TimeOfDay();
		if (castDne(time) == 1) {
			if (stanice == 0) {
				Activate(Time+(SpickaIntervalVlaku/((CestujiciVeseli*pocetLidiVeSpicce)/Spicka)));
			} else if (stanice == 1) {
			 	Activate(Time+(SpickaIntervalVlaku/((CestujiciBucovice*pocetLidiVeSpicce)/Spicka)));
			} else if (stanice == 2) {
				Activate(Time+(SpickaIntervalVlaku/((CestujiciSlavkov*pocetLidiVeSpicce)/Spicka)));
			}
		} else if (castDne(time) == 2) {
			if (stanice == 0) {
				Activate(Time+(NespickaIntervalVlaku/((CestujiciVeseli*(1.00-pocetLidiVeSpicce))/Nespicka)));
			} else if (stanice == 1) {
				Activate(Time+(NespickaIntervalVlaku/((CestujiciBucovice*(1.00-pocetLidiVeSpicce))/Nespicka)));
			} else if(stanice == 2) {
				Activate(Time+(NespickaIntervalVlaku/((CestujiciSlavkov*(1.00-pocetLidiVeSpicce))/Nespicka)));
			}
		} else {
			Activate(Time+Normal(3600, 55));
		}

		(new Cestujici(stanice))->Activate();
		Table(Time-Prijezd);
	}

	int stanice;

};



class GeneratorVlak : public Event {
	void Behavior() {

		double Prijezd = Time;
		int time = TimeOfDay();
		(new Vlak())->Activate();
			if (castDne(time) == 1) {
				Activate(Time+(SpickaIntervalVlaku*60.0));
			} else if (castDne(time == 2)) {
				Activate(Time+(NespickaIntervalVlaku*60.0));
			} else {
			Activate(Time+(Noc*60.0));
			}
		Table(Time-Prijezd);
	}
};

int main() {
	Print("Model vlakove trasy Bucovice - Brno\n");
	SetOutput("model.out");
	RandomSeed(time(NULL));
	Init(0,86400);
	(new GeneratorVlak())->Activate();
	(new GeneratorCestujici(0))->Activate();
	(new GeneratorCestujici(1))->Activate();
	(new GeneratorCestujici(2))->Activate();
	Run();
	Table.Output();
	cekaniVeseli.Output();
	cekaniBucovice.Output();
	cekaniSlavkov.Output();
	Vagony.Output();

	return 0;
}