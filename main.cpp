#include "simlib.h"
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <cmath>



const double TrasaA = 3600.0; // doba cesty na trase Veseli - Bucovice
const double TrasaB = 10.0*60.0; // doba cesty na trase Bucovice - Slavkov
const double TrasaC = 25.0*60.0; // doba cesty na trase Slavkov - Brno
const double Spicka = 180.5; // doba trvani spicky v minutach
const double Nespicka = 780.5; // doba trvani nespicky v minutach(cas mezi spickou a noci)
const double Noc = 420.0; // doba trvani noci v minutach(zadne vlaky nejezdi)

const double SpickaIntervalVlaku = 60; // interval mezi vlaky ve spicce
const double NespickaIntervalVlaku = 120; // interval mezi vlaky mimo spickuS
const int dobaNastupu = 2;
const int pocetVagonu = 3;
const double pocetMistVeVagonu = 33.3;
const int pocetVstupu = pocetVagonu*2;

const double pocetLidiVeSpicce = 0.54; // procent lidi ve spicce
const double VlakyVeSpicce = Spicka/SpickaIntervalVlaku;
const double VlakyMimoSpicku = Nespicka/NespickaIntervalVlaku;

const int cestujici[3] = {550, 250, 200};

int prijezdy[] = {5*3600, 6*3600, 7*3600, 8*3600, 10*3600, 12*3600, 14*3600, 16*3600, 18*3600, 20*3600, 22*3600};


const int PocetStanic = 4;
Queue cekarna[3];

Facility Stanice[PocetStanic];

Histogram Table("Cestujici", 0, 600,20);
Histogram Trains("Vlaky", 0, 3600,20);
int TimeOfDay(){
	int time = ((int) Time % 86400);
	return time;
}



int castDne(int time) {
	if ((time >= 4*3600+1800) && (time <= 8*3600+1800)) {
		return 1;
	} else if ((time > (8*3600+1800)) && (time <= 22*3600)) {
		return 2;
	} else {
		return 0;
	}
}


class Vlak : public Process {
public:
	Vlak(int time) : Process() {
		cas = time;
		store = new Store((unsigned int) (pocetVagonu*pocetMistVeVagonu));
		aktualniCas = cas;
	}


	void Behavior() {

		aktualniCas = TimeOfDay();
		Seize(Stanice[0]);

		if (this->store->Free()) {

			int volnychMist = (this->store->Free() - (unsigned int)cekarna[0].Length());
			Print("Volnych mist: %d\n", volnychMist);
			if (volnychMist <= 0) {
				Print("Pocet volnych mist: %d\n", this->store->Free());
				Wait(2);

			} else {
				Wait(3);
			}
			Print("Po cekani\n");
		}

		//Wait(((this->store->Free() > cekarna[0].Length()) ? (cekarna[0].Length()*pocetVagonu/pocetVstupu) : (this->store->Free())*pocetVagonu/pocetVstupu));
		Release(Stanice[0]);
		//Print("---------------------------------------\n");
		//Print("Vlak v %d jede do Bucovic, vyjizdi v %f\n", cas/3600, Time/3600);
		Wait(TrasaA);
		//Print("Vlak v %d | Trvani: %.0f (min) | Dojel v %.5f (hod) \n", cas/3600, TrasaA/60.0, (Time)/3600);

		//Print("---------------------------------------\n");
		aktualniCas = TimeOfDay();
		Seize(Stanice[1]);
		//Wait((this->store->Free() > cekarna[1].Length() ? cekarna[1].Length() : this->store->Free())*pocetVagonu/pocetVstupu);
		Release(Stanice[1]);
		//Print("---------------------------------------\n");
		//Print("Vlak v %d jede do Slavkova\n", cas/3600);
		Wait(TrasaB);
//		Print("Vlak v %d | Trvani: %.0f (min) | Dojel v %.2f (hod)\n", cas/3600, TrasaB/60.0, (Time)/3600);
		//Print("---------------------------------------\n");
		aktualniCas = TimeOfDay();
		Seize(Stanice[2]);
		//Wait((this->store->Free() > cekarna[2].Length() ? cekarna[2].Length() : this->store->Free())*pocetVagonu/pocetVstupu);
		Release(Stanice[2]);
		//Print("---------------------------------------\n");
		//Print("Vlak v %d jede do Brna\n", cas/3600);
		Wait(TrasaC);
		//Print("Vlak v %d | Trvani: %.0f (min)| Dojel v %.2f (hod)\n", cas/3600, TrasaC/60.0, (Time)/3600);

		aktualniCas = TimeOfDay();
		Seize(Stanice[3]);

		Release(Stanice[3]);
	}

	int getCas() const {
		return cas;
	}

	int getAktualniCas() const {
		return aktualniCas;
	}

	Store *getStore() const {
		return store;
	}
	bool isFull() {
		return this->store->Full();
	}
private:
	int cas;
	Store *store;
	int aktualniCas;
};


std::vector<Vlak*> vlaky;


int nejblizsiVlak(int time) {
	int nearest = 0;
	for (unsigned int i = 0; i < vlaky.size(); i++) {
		if ((vlaky.at(i)->getAktualniCas() - time) < (vlaky.at(nearest)->getAktualniCas() - time)) {
			nearest = i;
		}
	}
	return nearest;
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
		unsigned long nearest = (unsigned long) nejblizsiVlak(TimeOfDay());


		Into(cekarna[stanice]);
	enterTrain:
		if (Stanice[stanice].Busy() && !vlaky.at(nearest)->isFull()) {
			cekarna[stanice].GetFirst();
			Enter(*vlaky.at(nearest)->getStore());
			inTrain = true;
		} else {
			WaitUntil(Stanice[stanice].Busy());
			goto enterTrain;
		}
/*
		if (!Stanice[stanice].Busy() || !vlaky.at(nearest)->isFull()) {
			Into(cekarna[stanice]);
			WaitUntil(Stanice[stanice].Busy() && !vlaky.at(nearest)->isFull());
			cekarna[stanice].GetFirst();
			goto enterTrain;
		}
		if (Stanice[stanice].Busy() && !isInTrain() && !vlaky.at(nearest)->isFull()) {
			Enter(*vlaky.at(nearest)->getStore());
			inTrain = true;
			Table(Time-Prichod);
		} else {

		}
*/


		for (int i = stanice+1; i < PocetStanic; i++) {
			WaitUntil(inTrain && Stanice[i].Busy());
			double vystup = Random();
			if (isInTrain() && vystup <= 0.1) {
				Leave(*vlaky.at(nearest)->getStore());
				inTrain = false;
			}
		}


		if (Stanice[2].Busy() && inTrain) {
			Leave(*vlaky.at(nearest)->getStore());
		}

	}

	bool inTrain;
	int stanice;
};



class GeneratorCestujici : public Event {
public:
	GeneratorCestujici(int Stanice) : Event() {
		stanice = Stanice; // stanice 0 je Veseli, stanice 1 je Bucovice, 2 je Slavkov (negenerujeme nic)
	}

	void Behavior() {
		int time = TimeOfDay();
		if (castDne(time) == 1) {
			Activate(Time+Exponential(SpickaIntervalVlaku/((cestujici[stanice]*pocetLidiVeSpicce)/Spicka)));
		} else if (castDne(time) == 2) {
			Activate(Time+Exponential(SpickaIntervalVlaku/((cestujici[stanice]*(1.00-pocetLidiVeSpicce))/Nespicka)));
		} else {
			Activate(Time+Normal(3600, 55));
		}

		(new Cestujici(stanice))->Activate();
	}

	int stanice;

};



class GeneratorVlak : public Process {
	void Behavior() {
		Vlak* vlak;
		int size = sizeof(prijezdy)/sizeof(prijezdy[0]);
		for (int i = 0; i < size; i++) {
		prijezd:
			int time = TimeOfDay();
			if (time == prijezdy[i]) {
				vlak = (new Vlak(prijezdy[i]));
				vlaky.push_back(vlak);
				vlak->Activate(Time);
			} else {
				Wait(1);
				goto prijezd;
			}
		}
	}

};

int main() {
	Print("Model vlakove trasy Bucovice - Brno\n");
	RandomSeed(time(NULL));
	Init(0, 86400);

	(new GeneratorCestujici(0))->Activate();
	(new GeneratorCestujici(1))->Activate();
	(new GeneratorCestujici(2))->Activate();
	(new GeneratorVlak())->Activate();
	Run();
	//Table.Output();
	//Trains.Output();
	Stanice[0].Output();
	Stanice[1].Output();
	Stanice[2].Output();
	Stanice[3].Output();
	cekarna[0].Output();
	cekarna[1].Output();
	cekarna[2].Output();
	for (unsigned int i = 0; i < vlaky.size(); i++) {
		vlaky.at(i)->getStore()->Output();
	}

	return 0;
};