/* NE PAS TOUCHER */
#include <iostream>
#include <vector>
#include <list>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <chrono>
#include <string>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
using namespace std;
#define MAX 7                     // On définie le nombre de thread qu'on veut utiliser
bool done = 0;                    // Variable qui sert à savoir si on a terminé de créer tous les objets
string a[MAX] = {"Free", "Free", "Free", "Free", "Free", "Free", "Free"}; // Tableaux tampons pour recevoir les matériaux pour 
string b[MAX] = {"Free", "Free", "Free", "Free", "Free", "Free", "Free"}; // la fabrication et l'affichage.

/////////////////////////////////////////////////////////////
//AJOUTER VOS INCLUDE ET AUTRES ICI!!!!!!!!!!!!!!!!!!!!!!!!//
/////////////////////////////////////////////////////////////

#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <fstream>

// Protection des materiaux et des compteurs.
mutex mutexMateriaux;

// Protection des affichages et du fichier texte.
mutex mutexFichier;

// Reveil des threads en attente de materiaux.
condition_variable cvMateriaux;

// Gestion de l'ordre des demandes.
unsigned long long prochainTicket = 0;
unsigned long long ticketThread[MAX] = {};
bool threadEnAttente[MAX] = {};


 // CHRONOMETRAGE GLOBAL DES CYCLES

 // Mutex protegeant les mesures partagees.
 mutex mutexChrono;

 // Debut du cycle en cours.
 chrono::steady_clock::time_point debutCycle;

 // Nombre de threads entres et termines dans le cycle.
 int threadsEntres = 0;
 int threadsTermines = 0;

 // Numero du cycle et duree cumulee.
 int numeroCycle = 0;
 long long tempsTotalCycles = 0;

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! NE PAS TOUCHER !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/**/typedef struct {                                                                                                                           /**/
/**/    string name[MAX]{"Argent", "Or", "Bois", "Metal", "Diamant", "Fer", "Jade"};        // Nom des matériaux                              /**/
/**/    list<string> liste{"Argent", "Or", "Bois", "Metal", "Diamant", "Fer", "Jade"};      // Nom des matériaux dans une liste               /**/
/**/    string craft_name[MAX]{"Coupe", "Porte", "Table", "Bague", "Chandelier", "Epee", "Bracelet"};   // Nom des objets fabriqués                /**/
/**/    int craft[MAX]{0,0,0,0,0,0,0};                                 // /**/
/**/} craft_t;                                                                                                                                 /**/
/**/                                                                                                                                           /**/
/**/Display *dis;                                                           // Sert pour l'affichage du GUI                                    /**/
/**/int screen;                                                             // Sert pour l'affichage du GUI                                    /**/
/**/Window win;                                                             // Sert à créer la fenêtre pour le GUI                             /**/
/**/GC gc;                                                                  // graphic context                                                 /**/
/**/unsigned long black, white, red, blue, green;                           // Sert pour les couleurs dans le GUI                              /**/
/**/int x, y;                                                               // Sert pour les coordonnées                                       /**/
/**/                                                                                                                                           /**/
/**/void init();                                                            // Fonction pour initialiser le GUI                                /**/
/**/void close();                                                           // Fonction pour fermer le GUI et le programme                     /**/
/**/void refresh();                                                         // Fonction pour rafraîchir l'affichage graphique                  /**/
/**/void menu();                                                            // Fonction pour afficher le menu dans le GUI                      /**/
/**/void afficher(craft_t *c);                                              // Fonction pour afficher les ressources et les objets fabriqués   /**/
/**/unsigned long RGB(int r, int g, int b);                                 // Fonction pour convertir les nombres en couleurs RGB             /**/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/



void create(int i, craft_t *c) {

    using Horloge = chrono::steady_clock;

    // =====================================================
    // 1. CHRONOMETRAGE DU CYCLE
    // =====================================================

    {
        lock_guard<mutex> garde(mutexChrono);

        if (threadsEntres == 0) {
            debutCycle = Horloge::now();
            ++numeroCycle;
        }

        ++threadsEntres;
    }

    // =====================================================
    // 2. IDENTIFICATION DES MATERIAUX
    // =====================================================

    const string materiau1 = c->name[i];
    const string materiau2 = c->name[(i + 1) % MAX];
    const string objet = c->craft_name[i];

    // =====================================================
    // 3. JOURNALISATION
    // =====================================================

    // Une seule ecriture a la fois dans le terminal
    // et dans le fichier sortieCreation.txt.

    auto journaliser = [&](const string& message) {

        lock_guard<mutex> garde(mutexFichier);

        cout << message << endl;

        ofstream fichier("sortieCreation.txt", ios::app);

        if (!fichier || !(fichier << message << '\n')) {
            cerr << "Erreur : ecriture dans sortieCreation.txt"
                 << endl;
        }
    };

    // =====================================================
    // 4. TERMINAISON ET CHRONOMETRAGE GLOBAL
    // =====================================================

    // Tous les chemins de sortie de create()
    // passent par cette fonction.

    auto terminerThread = [&]() {

        bool dernier = false;
        int cycle = 0;
        long long duree = 0;
        long long cumul = 0;

        {
            lock_guard<mutex> garde(mutexChrono);

            ++threadsTermines;

            if (threadsTermines == MAX) {

                duree = chrono::duration_cast<
                    chrono::milliseconds
                >(Horloge::now() - debutCycle).count();

                tempsTotalCycles += duree;

                cumul = tempsTotalCycles;
                cycle = numeroCycle;

                // Reinitialisation pour le prochain cycle.
                threadsEntres = 0;
                threadsTermines = 0;

                dernier = true;
            }
        }

        // Seul le dernier thread affiche le bilan.
        if (dernier) {

            journaliser(
                "Cycle " + to_string(cycle) +
                " - duree : " + to_string(duree) + " ms"
            );

            journaliser(
                "Temps total cumule : " +
                to_string(cumul) + " ms"
            );
        }
    };

    // =====================================================
    // 5. ACCES AUX RESSOURCES PARTAGEES
    // =====================================================

    unique_lock<mutex> verrou(mutexMateriaux);

    // Ne pas fabriquer plus de dix objets par type.
    if (c->craft[i] >= 10) {

        verrou.unlock();
        terminerThread();
        return;
    }

    // Attribution d'un ticket unique.
    ticketThread[i] = prochainTicket++;
    threadEnAttente[i] = true;

    // =====================================================
    // 6. VERIFICATION DES MATERIAUX
    // =====================================================

    // Cette fonction est utilisee uniquement lorsque
    // mutexMateriaux est verrouille.

    auto disponible = [&](const string& materiau) {

        return find(
            c->liste.begin(),
            c->liste.end(),
            materiau
        ) != c->liste.end();
    };

    // Determine si le thread peut commencer sa fabrication.

    auto peutFabriquer = [&]() {

        // Les deux materiaux doivent etre disponibles.
        if (!disponible(materiau1) ||
            !disponible(materiau2)) {

            return false;
        }

        // Respecter les demandes plus anciennes
        // lorsqu'elles utilisent un materiau commun.

        for (int j = 0; j < MAX; ++j) {

            if (j == i ||
                !threadEnAttente[j] ||
                ticketThread[j] >= ticketThread[i]) {

                continue;
            }

            const string& autre1 = c->name[j];
            const string& autre2 = c->name[(j + 1) % MAX];

            bool conflit =
                materiau1 == autre1 ||
                materiau1 == autre2 ||
                materiau2 == autre1 ||
                materiau2 == autre2;

            if (conflit) {
                return false;
            }
        }

        return true;
    };

    // =====================================================
    // 7. GESTION DE L'ATTENTE
    // =====================================================

    if (!peutFabriquer()) {

        string attendu;

        // Identifier le materiau indisponible.
        if (!disponible(materiau1)) {

            attendu = materiau1;
        }
        else if (!disponible(materiau2)) {

            attendu = materiau2;
        }
        else {

            // Les materiaux sont disponibles, mais
            // une demande plus ancienne est prioritaire.

            for (int j = 0; j < MAX; ++j) {

                if (j == i ||
                    !threadEnAttente[j] ||
                    ticketThread[j] >= ticketThread[i]) {

                    continue;
                }

                const string& autre1 = c->name[j];
                const string& autre2 = c->name[(j + 1) % MAX];

                if (materiau1 == autre1 ||
                    materiau1 == autre2) {

                    attendu = materiau1;
                    break;
                }

                if (materiau2 == autre1 ||
                    materiau2 == autre2) {

                    attendu = materiau2;
                    break;
                }
            }
        }

        // Securite defensive.
        if (attendu.empty()) {
            attendu = materiau1;
        }

        a[i] = "Waiting";
        b[i] = "Waiting";

        // Liberer le mutex avant la journalisation.
        verrou.unlock();

        journaliser(
            objet + " en attente de " + attendu + "."
        );

        verrou.lock();
    }

    // =====================================================
    // 8. ATTENTE AVEC CONDITION VARIABLE
    // =====================================================

    // wait() libere temporairement mutexMateriaux.
    // Au reveil, le predicat est reevalue sous mutex.

    cvMateriaux.wait(verrou, peutFabriquer);

    // =====================================================
    // 9. RESERVATION ATOMIQUE DES DEUX MATERIAUX
    // =====================================================

    // Le mutex est actuellement verrouille.
    // Aucun autre thread ne peut modifier la liste.

    c->liste.remove(materiau1);
    c->liste.remove(materiau2);

    // La demande est maintenant satisfaite.
    threadEnAttente[i] = false;

    // Mettre a jour les informations d'affichage.
    a[i] = materiau1;
    b[i] = materiau2;

    // CORRECTION HELGRIND :
    // Notification pendant que le mutex est detenu.
    cvMateriaux.notify_all();

    // Liberer le mutex avant la fabrication.
    verrou.unlock();

    // =====================================================
    // 10. FABRICATION DE L'OBJET
    // =====================================================

    journaliser(objet + " en creation.");

    // Mesure de la duree de fabrication.
    const auto debut = Horloge::now();

    // Simulation de 200 millisecondes.
    this_thread::sleep_for(
        chrono::milliseconds(200)
    );

    const auto fin = Horloge::now();

    const auto dureeFabrication =
        chrono::duration_cast<
            chrono::milliseconds
        >(fin - debut).count();

    // =====================================================
    // 11. RESTITUTION DES MATERIAUX
    // =====================================================

    verrou.lock();

    // Rendre les deux materiaux disponibles.
    c->liste.push_back(materiau1);
    c->liste.push_back(materiau2);

    // Incrementer le compteur du bon objet.
    const int nombre = ++c->craft[i];

    // =====================================================
    // 12. VERIFICATION DE L'OBJECTIF GLOBAL
    // =====================================================

    bool tousTermines = true;

    for (int j = 0; j < MAX; ++j) {

        if (c->craft[j] < 10) {

            tousTermines = false;
            break;
        }
    }

    // Tous les objets ont atteint leur objectif.
    if (tousTermines) {
        done = true;
    }

    // CORRECTION HELGRIND :
    // Reveiller les threads sous mutex apres restitution.
    cvMateriaux.notify_all();

    // Liberer le mutex.
    verrou.unlock();

    // =====================================================
    // 13. ENREGISTREMENT DES RESULTATS
    // =====================================================

    journaliser(
        objet + " cree : " + to_string(nombre)
    );

    journaliser(
        objet + " - duree de fabrication : " +
        to_string(dureeFabrication) + " ms"
    );

    // =====================================================
    // 14. FIN DU THREAD
    // =====================================================

    terminerThread();
}



                                                                                             // *** Fin fonction create() ******************* //

/*!!!!!!!!!! NE PAS TOUCHER !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
void start(craft_t *c){                                                                         // *** Début fonction start() ****************** //
  vector<thread> threads(MAX);                                                                  // On crée un vecteur de threads.
                                                                                                // 
  for (int i = 0; i < MAX; ++i) {                                                               ///////////////////////////////////////////////////
     threads[i] = thread([i, c] { create(i, c);});                                              // La première boucle crée et utilise un thread
  }                                                                                             // et d'appeler la fonction create().                                             
  for (auto& t : threads) {                                                                     // La deuxième boucle permet de joindre les threads
    t.join();                                                                                   // au fur et à mesure.
  }                                                                                             ///////////////////////////////////////////////////
  cout << "_____________________________________________"<< endl;                               // Ligne permettant de séparer dans le CLI.
}                                                                                               // *** Fin fonction start() ******************** //


/*!!!!!!!!!! NE PAS TOUCHER !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
int main() {                                                                                    // *** Début fonction main() ******************* //
  craft_t craft;                                                                                // Permet de créer l'objet craft, qui va stocker nos informations tout au long du programme.    
  x = 8; y = 10;                                                                                // Permet de configurer les coordonnées de bases pour le GUI.
  init();                                                                                       // Initilialise le GUI.
  XEvent event;                                                                                 // Permet de savoir le type d'evenement.
  char text[1];                                                                                 // Permet d'entreposer la valeur du clavier.
                                                                                                //
  while (1){                                                                                    ///////////////////////////////////////////////////
    afficher(&craft);                                                                           // On affiche dans le GUI.
    y = 10;                                                                                     // On remet le y à 10 pour les prochaines affichages.
    XNextEvent(dis, &event);                                                                    // Se prépare à recevoir l'evenement.
    if(event.type==KeyPress && XLookupString(&event.xkey, text, 255, NULL,0)==1){               // Si il y a eu un evenement (en appuyant sur le clavier):
        if(text[0]=='s'){                                                                       // Option "s":
          if (done != 1){                                                                       // Tant que la fabrication n'est pas terminé,
            start(&craft);                                                                      // On commence un cycle de fabrication d'objet.
          }                                                                                     //
        }                                                                                       // _____________________________________________ //
        if(text[0]=='r'){                                                                       // Option "r":
          if (done != 1){                                                                       // Si la frabrication d'objet n'est pas terminé,
            afficher(&craft);                                                                   // On "rafraîchit" l'affichage.
          }                                                                                     //
        }                                                                                       // _____________________________________________ //
        if(text[0]=='t'){                                                                       // Option "t":
          if (done != 1){                                                                       // Si la frabrication d'objet n'est pas terminé,
            for (int i = 0; i < MAX; ++i) {                                                     // On "reset" la file en remettant chaque ressources,
              a[i] = b[i] = "Free";                                                             // dans un cas où c'est buggé.
              craft.liste ={"Argent", "Or", "Bois", "Metal", "Diamant", "Fer", "Jade"};          //
            }                                                                                   //
          }                                                                                     //
        }                                                                                       // _____________________________________________ //
        if(text[0]=='q'){                                                                       // Option "q":
          close();                                                                              // On ferme le programme.
        }                                                                                       //
    }                                                                                           //
  }                                                                                             ///////////////////////////////////////////////////
                                                                                                //
  return 0;                                                                                     //
}                                                                                               // *** Fin fonction main() ********************* //

/*!!!!!!!!!! NE PAS TOUCHER !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/**/void init(){                                                                                /**/
/**/    x = 100; y = 100;                                                                       /**/
/**/    dis = XOpenDisplay((char *)0);                                                          /**/
/**/    screen = DefaultScreen(dis);                                                            /**/
/**/    black = BlackPixel(dis, screen);                                                        /**/
/**/    white = WhitePixel(dis, screen);                                                        /**/
/**/    red = RGB(255,0,0);                                                                     /**/
/**/    green = RGB(0,255,0);                                                                   /**/
/**/    blue = RGB(0,0,255);                                                                    /**/
/**/    win = XCreateSimpleWindow(dis, DefaultRootWindow(dis),0,0,350,350,5, black, white);     /**/
/**/    XSetStandardProperties(dis,win,"Lab 2 - 8INF342",NULL, None, NULL, 0, NULL);            /**/
/**/    XSelectInput(dis, win, ExposureMask | ButtonPressMask | KeyPressMask);                  /**/
/**/    gc = XCreateGC(dis, win, 0, 0);                                                         /**/
/**/    XSetBackground(dis,gc,white);                                                           /**/
/**/    XSetForeground(dis,gc,black);                                                           /**/
/**/    XClearWindow(dis,win);                                                                  /**/
/**/    XMapRaised(dis, win);                                                                   /**/
/**/}                                                                                           /**/
/**/                                                                                            /**/
/**/void close(){                                                                               /**/
/**/    XFreeGC(dis, gc);                                                                       /**/
/**/    XDestroyWindow(dis,win);                                                                /**/
/**/    XCloseDisplay(dis);                                                                     /**/
/**/    exit(0);                                                                                /**/
/**/}                                                                                           /**/
/**/                                                                                            /**/
/**/void refresh(){                                                                             /**/
/**/    XClearWindow(dis, win);                                                                 /**/
/**/    x = 8;                                                                                  /**/
/**/    y = 10;                                                                                 /**/
/**/}                                                                                           /**/
/**/                                                                                            /**/
/**/void menu(){                                                                                /**/
/**/    x = 8;                                                                                  /**/
/**/    y = 10;                                                                                 /**/
/**/    XSetForeground(dis,gc,BlackPixel(dis, screen));                                         /**/
/**/    XDrawString(dis,win,gc,x,y+200,"Appuyer sur 's' pour cycle",26);                        /**/
/**/    XDrawString(dis,win,gc,x,y+210,"Appuyer 'r' pour refresh page",25);                         /**/
/**/    XDrawString(dis,win,gc,x,y+220,"Appuyer 't' pour reset statut",29);                         /**/
/**/    XDrawString(dis,win,gc,x,y+230,"Appuyer 'q' pour quit",21);                                 /**/
/**/    XDrawString(dis,win,gc,x,y+250,"Recipes:",8);                                           /**/
/**/    XDrawString(dis,win,gc,x,y+260,"Argent + Or = Coupe",19);                            /**/
/**/    XDrawString(dis,win,gc,x,y+270,"Or + Bois = Porte",17);                             /**/
/**/    XDrawString(dis,win,gc,x,y+280,"Bois + Metal = Table",20);                            /**/
/**/    XDrawString(dis,win,gc,x,y+290,"Metal + Diamant = Bague",23);                             /**/
/**/    XDrawString(dis,win,gc,x,y+300,"Diamant + Fer = Chandelier",26);                         /**/
/**/    XDrawString(dis,win,gc,x,y+310,"Fer + Jade = Epee",17);                           /**/
/**/    XDrawString(dis,win,gc,x,y+320,"Jade + Argent = Bracelet",24);                           /**/
/**/}                                                                                           /**/
/**/                                                                                            /**/
/**/void afficher(craft_t *c){                                                                  /**/
/**/  y = 10;                                                                                   /**/
/**/  XClearWindow(dis, win);                                                                   /**/
/**/  for(int k = 0; k <7; k++){                                                                /**/
/**/    string nbr = to_string(c->craft[k]);                                                    /**/
/**/    XSetForeground(dis,gc,BlackPixel(dis, screen));                                         /**/
/**/    XDrawString(dis,win,gc,x,y,c->craft_name[k].c_str(),c->craft_name[k].size());           /**/
/**/    XDrawString(dis,win,gc,x+42,y," : ",3);                                                 /**/
/**/    XSetForeground(dis,gc,blue);                                                            /**/
/**/    XDrawString(dis,win,gc,x+60,y,nbr.c_str(),nbr.size());                                  /**/
/**/    y = y + 10;                                                                             /**/
/**/  }                                                                                         /**/
/**/  y = y +30;                                                                                /**/
/**/  for(int k = 0; k <7; k++){                                                                /**/
/**/    string kk = to_string(k+1);                                                             /**/
/**/    XSetForeground(dis,gc,BlackPixel(dis, screen));                                         /**/
/**/    XDrawString(dis,win,gc,x,y,"Objet ",6);                                          /**/
/**/    XDrawString(dis,win,gc,x+72,y,kk.c_str(),1);                                            /**/
/**/    XDrawString(dis,win,gc,x+76,y," : ",3);                                                 /**/
/**/    XSetForeground(dis,gc,red);                                                             /**/
/**/    XDrawString(dis,win,gc,x+102,y,a[k].c_str(),a[k].size());a[k] = "Free";                 /**/
/**/    XDrawString(dis,win,gc,x+152,y,b[k].c_str(),b[k].size());b[k] = "Free";                 /**/
/**/    y = y + 10;                                                                             /**/
/**/  }                                                                                         /**/
/**/  if (done == 1){                                                                           /**/
/**/    XSetForeground(dis,gc,green);                                                           /**/
/**/    XDrawString(dis,win,gc,x,y+10,"Items completes!",15);                             /**/
/**/  }                                                                                         /**/
/**/  menu();                                                                                   /**/
/**/}                                                                                           /**/
/**/                                                                                            /**/
/**/unsigned long RGB(int r, int g, int b){                                                     /**/
/**/    return b + (g<<8) + (r<<16);                                                            /**/
/**/}                                                                                           /**/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/