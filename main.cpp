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
int aktualniStanice = -1;

int spickaPrijezdyBucovice[] = {5*3600, 6*3600, 7*3600, 8*3600};
int spickaPrijezdySlavkov[] = {5*3600+(int)TrasaA*60, 6*3600+(int)TrasaA*60, 7*3600+(int)TrasaA*60, 8*3600+(int)TrasaA*60};
int nespickaPrijezdyBucovice[] = {10*3600, 12*3600, 14*3600, 16*3600, 18*3600, 20*3600, 22*3600};
int nespickaPrijezdySlavkov[] = {10*3600+(int)TrasaA*60, 12*3600+(int)TrasaA*60, 14*3600+(int)TrasaA*60, 16*3600+(int)TrasaA*60, 18*3600+(int)TrasaA*60, 20*3600+(int)TrasaA*60, 22*3600+(int)TrasaA*60};

const int PocetStanic = 3;
Queue cekaniBucovice("Cekani ve stanici Bucovice");
Queue cekaniSlavkov("Cekani ve stanici Slavkov");

Facility Stanice[PocetStanic];

Histogram Table("Table", 0,25,20);
Store Vlak("Vlak", 100);

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


class VeseliBucovice : public Event {

	void Behavior() {
		double Prijezd = Time;

		Table(Time-Prijezd);
	}
};

class BucoviceSlavkov : public Event {

	void Behaviour() {
		double Prijezd = Time;
		//int time = TimeOfDay();

		Table(Time-Prijezd);
	}

};



class SlavkovBrno : public Event {

	void Behaviour() {
		double Prijezd = Time;
		//int time = TimeOfDay();

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
		int time = TimeOfDay();

		if (castDne(time) == 1) {

			if (stanice == 0) {
				if (time == dalsiVlak(time, spickaPrijezdyBucovice)) {
					if (Vlak.Free()) {
						Enter(Vlak);
					} else {
						Into(cekaniBucovice);
						Passivate();
						WaitUntil(castDne(time) == 1 && time == dalsiVlak(time, spickaPrijezdyBucovice) && Vlak.Free());
						Enter(Vlak);
					}
				} else {
					Into(cekaniBucovice);
					Passivate();
					WaitUntil(castDne(time) == 1 && time == dalsiVlak(time, spickaPrijezdyBucovice) && Vlak.Free());
					Enter(Vlak);
				}
			} else if (stanice == 1) {
				if (time == dalsiVlak(time, spickaPrijezdySlavkov)) {
					if (Vlak.Free()) {
						Enter(Vlak);
					} else {
						Into(cekaniSlavkov);
						Passivate();
						WaitUntil(castDne(time) == 1 && time == dalsiVlak(time, spickaPrijezdySlavkov) && Vlak.Free());
						Enter(Vlak);
					}
				} else {
					Into(cekaniSlavkov);
					Passivate();
					WaitUntil(castDne(time) == 1 && time == dalsiVlak(time, spickaPrijezdySlavkov) && Vlak.Free());
					Enter(Vlak);
				}
			}
		} else if (castDne(time) == 2) {
			if (stanice == 0) {
				if (time == dalsiVlak(time, nespickaPrijezdyBucovice)) {
					if (Vlak.Free()) {
						Enter(Vlak);
					} else {
						Into(cekaniBucovice);
						Passivate();
						WaitUntil(castDne(time) == 2 && time == dalsiVlak(time, nespickaPrijezdyBucovice) && Vlak.Free());
						Enter(Vlak);
					}
				} else {
					Into(cekaniBucovice);
					Passivate();
					WaitUntil(castDne(time) == 2 && time == dalsiVlak(time, nespickaPrijezdyBucovice) && Vlak.Free());
					Enter(Vlak);
				}
			} else if (stanice == 1) {
				if (time == dalsiVlak(time, nespickaPrijezdySlavkov)) {
					if (Vlak.Free()) {
						Enter(Vlak);
					} else {
						Into(cekaniSlavkov);
						Passivate();
						WaitUntil(castDne(time) == 2 && time == dalsiVlak(time, nespickaPrijezdySlavkov) && Vlak.Free());
						Enter(Vlak);
					}
				} else {
					Into(cekaniSlavkov);
					Passivate();
					WaitUntil(castDne(time) == 2 && time == dalsiVlak(time, nespickaPrijezdySlavkov) && Vlak.Free());
					Enter(Vlak);
				}
			}
		} else {
			Activate(Time+Exponential(3600));
			if (stanice == 0) {
				Into(cekaniBucovice);
			} else if (stanice == 1) {
				Into(cekaniSlavkov);
			}
			Passivate();
		}
		Table(Time-Prichod);
	}

	int stanice;
};


class Generator : public Event {
public:
	Generator(int Stanice) : Event() {
		stanice = Stanice; // stanice 0 (false) jsou Bucovice, stanice 1 (true) je Slavkov
	}

	void Behavior() {
		int time = TimeOfDay();
		if (castDne(time) == 1) {
			if (stanice == 0) {
				Activate(Time+Exponential((int)SpickaIntervalVlaku/CestujiciBucovice*pocetLidiVeSpicce/Spicka));
			} else if (stanice == 1) {
				Activate(Time+Exponential((int)SpickaIntervalVlaku/CestujiciSlavkov*pocetLidiVeSpicce/Spicka));
			}
		} else if (castDne(time) == 2) {
			if (stanice == 0) {
				Activate(Time+Exponential((int)NespickaIntervalVlaku/CestujiciBucovice*(1.00-pocetLidiVeSpicce)/Nespicka));
			} else if(stanice == 1) {
				Activate(Time+Exponential((int)NespickaIntervalVlaku/CestujiciSlavkov*(1.00-pocetLidiVeSpicce)/Nespicka));
			}
		} else {
			Activate(Time+Normal(3600, 55));
		}

		(new Cestujici(stanice))->Activate();
	}

	int stanice;

};

int main() {
	Print("Model vlakove trasy Bucovice - Brno\n");
	SetOutput("model.out");
	RandomSeed(time(NULL));
	Init(0,86400);
	(new Generator(0))->Activate();
	(new Generator(1))->Activate();
	Run();
	Table.Output();
	cekaniBucovice.Output();
	cekaniSlavkov.Output();
	Vlak.Output();
	Stanice[2].Output();
	Stanice[1].Output();
	Stanice[0].Output();

	return 0;
}