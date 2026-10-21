#include <iostream>
#include <random>
#include <curses.h>                                                                                                                         // fennetre et couleurs
#include <cstdlib>                                                                                                                          // Pour utiliser abs().
#include <fstream>                                                                                                                          // Pour lire les fichiers de niveaux.
#include <string>                                                                                                                           // Pour utiliser des string
#include <vector>                                                                                                                           // Pour stocker la carte dans un tableau dynamique.
#include <windows.h>                                                                                                                        // Nécessaire pour Windows API, musique
#include <mmsystem.h>                                                                                                                       // Nécessaire pour le système multimédia, musique
using namespace std;                                                                                                                        // simplifie l'écriture pas besoint de std:: partout



int Largeur = 0;
int Hauteur = 0;
int spawnX = 0;                                                                                                                             // Position X du spawn du heros.
int spawnY = 0;                                                                                                                             // Position Y du spawn du heros.
int sortieX = -1;                                                                                                                           // Position X de la sortie.
int sortieY = -1;                                                                                                                           // Position Y de la sortie.
int niveau = 0;
bool gameOver = false;                                                                                                                      // boucle pour que le jeux continue
bool Etat = false;                                                                                                                          // pour lancer le sort
int NB_ENNEMIS = 1;                                                                                                                         // Nombre d'ennemis pour le moment const plutart variables
int niveauMax = 4;                                                                                                                          // Nombre de niveau max pour le moment const plutart variables
bool sonEnnemies = false;
bool sonCacher = false;
bool heroMortEnnemi = false;
bool heroMortHoxygene = false;



enum tile_type {                                                                                                                            // diférent terrain
    ground = 0,
    river = 1,
    mountain = 2
};

vector<tile_type> map;                                                                                                                      // Tableau a UNE dimension contenant les terrains de la carte et dynamique grace aux vecteurs.


struct HeroStat
{
    int x;
    int y;
    bool hidden;
    int pv;
	int oxygene;
};


struct enemiesStat
{
    int x;
    int y;
    int pv;
};

struct pointeurEnnemis
{
    int x;
	int y;
};

struct CurseurPosition {
    int x;
    int y;
};


int indice(int x, int y)                                                                                                                        // Transforme les coordonnees (x, y) en indice du tableau
{
    return y * Largeur + x;
}


void Menu()                                                                                                                                     // Affiche le menu principal
{
    initscr();                                                                                                                                  // Initialise PDCurses.
    cbreak();                                                                                                                                   // Lit les touches sans attendre Entree.
    noecho();                                                                                                                                   // affiche pas zqsd
    keypad(stdscr, TRUE);                                                                                                                       // pour les fléches.
    curs_set(0);                                                                                                                                // Masque le curseur, sert a pas grand chose mais c'est mieux pour l'esthétique

    string line;
    ifstream fichierMenu("map/Menu.txt");                                                                                                       // Ouvre le fichier

    while (getline(fichierMenu, line))
    {
        printw("%s\n", line.c_str());                                                                                                           // Affiche chaque ligne du menu. et transforme le string en char* pour l'affichage
    }
    fichierMenu.close();
    PlaySound(TEXT("C:\\Windows\\Media\\Ring09.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);                                               // musique car pourquoi pas

    refresh();                                                                                                                                  // Met a jour l'affichage du menu.

    bool play = false;
    while (!play)
    {
        int choix = getch();                                                                                                                    // Attend une touche du joueur.

        if (choix == 'a' || choix == 'A')
        {
            play = true;                                                                                                                        // Le joueur choisit de commencer.
        }
        else if (choix == 'r' || choix == 'R' || choix == 27)
        {
            play = true;                                                                                                                        // Le joueur choisit de quitter.
            gameOver = true;
        }
    }

    clear();                                                                                                                                    // Efface le menu avant de commencer la partie.
    refresh();
}


bool chargerCarte(const string& nomFichier, enemiesStat*& ennemis)                                                                                                     // Charge la map
{
    ifstream fichier(nomFichier);                                                                                                               // Ouvre le fichier du niveau.
    if (!fichier.is_open()) return false;
    vector<string> lignes;
    string ligne;

    while (getline(fichier, ligne))
    {
        if (!ligne.empty() && ligne.back() == '\r')                                                                                             // suprime les espaces et les retour a la ligne
            ligne.pop_back();                                                                                                                   // suprime le caractere

        if (!ligne.empty())                                                                                                                     // si la ligne n'est pas vide rajoute une ligne au tableaus dinamique par vecteur
            lignes.push_back(ligne);
    }
    fichier.close();
	int nombreEnnemis = 0;                                                                                                                      // Compte le nombre d'ennemis dans le fichier de niveau.
	for (const string& lignesCarte : lignes)                                                                                                    // Parcourt chaque ligne du fichier.
{
		for (char symbole : lignesCarte)                                                                                                        // Parcourt chaque symbole de la ligne.
		{
			if (symbole == 'Z')                                                                                                                 // Si le symbole est un ennemi.
				nombreEnnemis++;                                                                                                                // Incrémente le compteur d'ennemis.
		}
	}
	delete[] ennemis;                                                                                                                           // Supprime l'ancien tableau d'ennemis.
	NB_ENNEMIS = nombreEnnemis;                                                                                                                 // Met à jour le nombre d'ennemis.
	ennemis = new enemiesStat[NB_ENNEMIS]{};                                                                                                    // Crée un nouveau tableau d'ennemis avec la taille appropriée.

    Hauteur = static_cast<int>(lignes.size());                                                                                                  // Calcule la hauteur avec le nombre de lignes, convertit ce nombre en entier car ce n'est pas des int
    Largeur = static_cast<int>(lignes[0].size());                                                                                               // Calcule la largeur avec la premiere ligne, convertit ce nombre en entier car ce n'est pas des int

    map.clear();                                                                                                                                // supprime tous les terrains de la carte précédente

    spawnX = -1;                                                                                                                                // Reinitialise la position de depart.
    spawnY = -1;
    sortieX = -1;                                                                                                                               // Reinitialise la position de sortie.                
    sortieY = -1;
    int i = 0;
    for (int y = 0; y < Hauteur; y++)
    {
        for (int x = 0; x < Largeur; x++)
        {
            char symbole = lignes[y][x];                                                                                                        // Lit le symbole de la case actuelle.

            if (symbole == 'S')
            {
                spawnX = x;                                                                                                                     // Enregistre la coordonnee X du depart.        
                spawnY = y;                                                                                                                     // Enregistre la coordonnee Y du depart.
                map.push_back(ground);                                                                                                          // Le depart est du sol dans le tableau de terrain.
            }
            else if (symbole == 'E')
            {
                sortieX = x;                                                                                                                    // Enregistre la coordonnee X de la sortie.
                sortieY = y;                                                                                                                    // Enregistre la coordonnee Y de la sortie.
                map.push_back(ground);                                                                                                          // La sortie reste du sol dans le tableau de terrain.
            }
            else if (symbole == '.')
                map.push_back(ground);                                                                                                          // Ajoute une case de sol.
            else if (symbole == '~')
                map.push_back(river);                                                                                                           // Ajoute une case de riviere.
            else if (symbole == 'A')
                map.push_back(mountain);                                                                                                        // Ajoute une case de montagne.
            else if (symbole == 'Z') {
                ennemis[i] = { x, y, 1 };
                i++;
                map.push_back(ground);                                                                                                          // Le depart est du sol dans le tableau de terrain.
            }
            else
                return false;                                                                                                                   // Refuse les symboles inconnus.
        }
    }
	return true;                                                                                                                                // La carte a ete chargee avec succes.
}


bool ennemiPresent(const enemiesStat* ennemis, int x, int y)                                                                                   // Verifie si un ennemi vivant occupe une case.
{
    for (int i = 0; i < NB_ENNEMIS; i++)
    {
		const enemiesStat* ennemi = &ennemis[i];                                                                                                // Reference pour simplifier l'ecriture.
        if (ennemi->x == x && ennemi->y == y && ennemi->pv > 0)
            return true;                                                                                                                        // Un ennemi vivant se trouve sur cette case.
    }
    return false;                                                                                                                               // Aucun ennemi vivant sur cette case.
}

void afficherCarte(const HeroStat& hero, const enemiesStat ennemis[])                                                                           // Affiche le terrain, le heros et les ennemis.
{
    erase();                                                                                                                                    // Efface l'affichage precedent.
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);                                                                                                     // Couleur du heros visible.
    init_pair(2, COLOR_BLUE, COLOR_BLACK);                                                                                                      // Couleur du heros cacher.
    init_pair(3, COLOR_RED, COLOR_BLACK);                                                                                                       // Couleur de l'ennemi.
    init_pair(4, COLOR_GREEN, COLOR_BLACK);                                                                                                     // Couleur de la sortie.
    init_pair(5, COLOR_YELLOW, COLOR_BLACK);                                                                                                    // Couleur du sol.
    init_pair(6, COLOR_CYAN, COLOR_BLACK);                                                                                                      // Couleur de la riviere.
    init_pair(7, COLOR_MAGENTA, COLOR_BLACK);                                                                                                   // Couleur de la montagne.

    for (int y = 0; y < Hauteur; y++)
    {
        for (int x = 0; x < Largeur; x++)
        {
            char symbole = '.';
            int i = indice(x, y);                                                                                                               // Calcule les coordonées

            if (hero.x == x && hero.y == y)
            {
                if (hero.hidden)
                    symbole = 'S';                                                                                                              // S = heros cacher
                else
                    symbole = 'H';                                                                                                              // H = heros visible
            }
            else if (ennemiPresent(ennemis, x, y))
                symbole = 'X';                                                                                                                  // X = ennemi
            else if (x == sortieX && y == sortieY)
                symbole = 'E';                                                                                                                  // E = sortie
            else if (map[i] == ground)
                symbole = '.';                                                                                                                  // . =  sol.
            else if (map[i] == river)
                symbole = '~';                                                                                                                  // ~ = riviere.
            else if (map[i] == mountain)
                symbole = 'A';                                                                                                                  // A =  montagne.


            if (has_colors() == true)
            {
                if (symbole == 'H')
                    attron(COLOR_PAIR(1));                                                                                                      // Couleur du heros visible.
                else if (symbole == 'S')
                    attron(COLOR_PAIR(2));                                                                                                      // Couleur du heros cacher.
                else if (symbole == 'X')
                    attron(COLOR_PAIR(3));                                                                                                      // Couleur de l'ennemi.
                else if (symbole == 'E')
                    attron(COLOR_PAIR(4));                                                                                                      // Couleur de la sortie.
                else if (symbole == '.')
                    attron(COLOR_PAIR(5));                                                                                                      // Couleur du sol.
                else if (symbole == '~')
                    attron(COLOR_PAIR(6));                                                                                                      // Couleur de la riviere.
                else if (symbole == 'A')
                    attron(COLOR_PAIR(7));                                                                                                      // Couleur de la montagne.
            }
            mvaddch(y, x, symbole);                                                                                                             // Affiche symbole aux coordonnees (y, x), aprés les couleurs pour les appliquer
        }
    }

    mvprintw(Hauteur + 1, 0, "Z : haut S : bas Q : gauche D : droite");
    mvprintw(Hauteur + 2, 0, "E : sort R : quitter");
    mvprintw(Hauteur + 3, 0, "PV : %d", hero.pv);                                                                                               // Affiche les points de vie restants.
    mvprintw(Hauteur + 5, 0, "Oxygene : %d", hero.oxygene);      

    if (hero.hidden)
        mvprintw(Hauteur + 4, 0, "Le heros est cache dans la riviere.");
    else
        mvprintw(Hauteur + 4, 0, "Le heros est visible.");

    mvprintw(Hauteur + 6, 0, "Niveau : %d", niveau);
    mvprintw(Hauteur + 7, 0, "Objectif : atteindre la sortie E.");
    if (has_colors() == false) {
        mvprintw(Hauteur + 8, 0, "Votre terminal ne supporte pas les couleurs.");
    }
    refresh();                                                                                                                                  // Met a jour l'ecran.
}


void colisionDetection(HeroStat& hero, enemiesStat ennemis[])                                                                                   // Detecte les collisions entre le heros et les ennemis.
{
    for (int i = 0; i < NB_ENNEMIS; i++)
    {
        if (hero.x == ennemis[i].x && hero.y == ennemis[i].y && ennemis[i].pv > 0)
        {
            hero.pv -= 1;                                                                                                                       // Le heros perd un point de vie et meurt pour l'instant.
            ennemis[i].pv = 0;                                                                                                                  // Elimine l'ennemi qui a touche le heros au cas ou on rajoute des pv au hers.
            Beep(300, 300);
        }
    }
}


void mouvementHero(HeroStat& hero, int mouvement)
{
    int nouveauX = hero.x;
    int nouveauY = hero.y;

    if (mouvement == 'z' || mouvement == 'Z' || mouvement == KEY_UP)
        nouveauY--;                                                                                                                             // Deplacement vers le haut.
    else if (mouvement == 's' || mouvement == 'S' || mouvement == KEY_DOWN)
        nouveauY++;                                                                                                                             // Deplacement vers le bas.
    else if (mouvement == 'q' || mouvement == 'Q' || mouvement == KEY_LEFT)
        nouveauX--;                                                                                                                             // Deplacement vers la gauche.
    else if (mouvement == 'd' || mouvement == 'D' || mouvement == KEY_RIGHT)
        nouveauX++;                                                                                                                             // Deplacement vers la droite.
    else if (mouvement == 'e' || mouvement == 'E')
    {
        Etat = true;                                                                                                                            // Demande de lancer un sort.
        return;
    }
    else
        return;                                                                                                                                 // Ignore les autres touches et passe le tour

    if (nouveauX < 0 || nouveauX >= Largeur || nouveauY < 0 || nouveauY >= Hauteur)                                                             // Empeche le heros de sortir des limites de la carte.
        return;

    if (map[indice(nouveauX, nouveauY)] == mountain)                                                                                            // Empeche le heros de traverser les montagnes.
        return;

    hero.x = nouveauX;                                                                                                                          // Actualise la position X du heros si les conditions sont remplies.
    hero.y = nouveauY;                                                                                                                          // Actualise la position Y du heros si les conditions sont remplies.
    hero.hidden = (map[indice(hero.x, hero.y)] == river);                                                                                       // si heros est sur la riviere, il est cache
}


void mouvementEnnemie(enemiesStat ennemis[], const HeroStat& hero)                                                                              // Deplace les ennemis vers le heros s'il est proche ou au hasard.
{
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> direction(0, 3);                                                                                              // 0 droite, 1 gauche, 2 bas, 3 haut.

    for (int i = 0; i < NB_ENNEMIS; i++)
    {
        if (ennemis[i].pv <= 0)
            continue;                                                                                                                           // Ignore les ennemis morts sinon il réaparaise.

        int differenceX = hero.x - ennemis[i].x;
        int differenceY = hero.y - ennemis[i].y;
        int nouveauEnnemiX = ennemis[i].x;
        int nouveauEnnemiY = ennemis[i].y;

        if (!hero.hidden && abs(differenceX) + abs(differenceY) <= 4) {
            if (sonEnnemies == false)
                PlaySound(TEXT("C:\\Windows\\Media\\Windows Notify System Generic.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
            sonEnnemies = true;
            if (abs(differenceX) > abs(differenceY)) {
                if (differenceX > 0) {
                    nouveauEnnemiX += 1;
                }
                else {
                    nouveauEnnemiX -= 1;
                }
            }
            else if (differenceY != 0) {
                if (differenceY > 0) {
                    nouveauEnnemiY += 1;
                }
                else {
                    nouveauEnnemiY -= 1;
                }
            }
        }
        else {
            if (sonEnnemies == true)
                PlaySound(NULL, NULL, 0);
            sonEnnemies = false;
            sonEnnemies = false;
            int choix = direction(gen);                                                                                                         // Choisit une direction au hasard.
            if (choix == 0) nouveauEnnemiX++;
            else if (choix == 1) nouveauEnnemiX--;
            else if (choix == 2) nouveauEnnemiY++;
            else nouveauEnnemiY--;
        }

        if (nouveauEnnemiX < 0 || nouveauEnnemiX >= Largeur || nouveauEnnemiY < 0 || nouveauEnnemiY >= Hauteur)                                 // Ignore un deplacement hors de la carte.
            continue;                                                                                                                           // passe au prochain ennemi sans changer la position de l'ennemi actuel
        if (map[indice(nouveauEnnemiX, nouveauEnnemiY)] == mountain || map[indice(nouveauEnnemiX, nouveauEnnemiY)] == river)                    // Les ennemis ne traversent ni montagne ni riviere.
            continue;
        if (nouveauEnnemiX == sortieX && nouveauEnnemiY == sortieY)                                                                             // L'ennemi ne va pas sur la sortie.
            continue;
        if (ennemiPresent(ennemis, nouveauEnnemiX, nouveauEnnemiY) && !(ennemis[i].x == nouveauEnnemiX && ennemis[i].y == nouveauEnnemiY))      // Deux ennemis ne partagent pas une case.
            continue;

        ennemis[i].x = nouveauEnnemiX;                                                                                                          // Actualise la position X de l'ennemi.
        ennemis[i].y = nouveauEnnemiY;                                                                                                          // Actualise la position Y de l'ennemi.
    }
}

bool mouvementCurseur(HeroStat& hero, int touche, enemiesStat* ennemis, pointeurEnnemis* pointeur, CurseurPosition& curseur)                                                                                                 // Déplace le curseur sur les ennemis adjacents.
{
	static int curseurX = curseur.x;
	static int curseurY = curseur.y;
    mvaddch(curseurY, curseurX, '^');
	if (touche == 'z' || touche == 'Z' || touche == KEY_UP)
		curseurY--;
	else if (touche == 's' || touche == 'S' || touche == KEY_DOWN)
		curseurY++;
	else if (touche == 'q' || touche == 'Q' || touche == KEY_LEFT)
		curseurX--;
	else if (touche == 'd' || touche == 'D' || touche == KEY_RIGHT)
		curseurX++;
	else if (touche == 32)                                                                                                                     // Espace pour lancer le sort.
	{
		for (int i = 0; i < NB_ENNEMIS; i++)
		{
			if (curseurX == pointeur[i].x && curseurY == pointeur[i].y)
			{
				return true;
			}
		}
		return false;
	}
	if (curseurX < hero.x - 1) curseurX = hero.x - 1;
	if (curseurX > hero.x + 1) curseurX = hero.x + 1;
	if (curseurY < hero.y - 1) curseurY = hero.y - 1;
	if (curseurY > hero.y + 1) curseurY = hero.y + 1;
	move(curseurY, curseurX);                                                                                                                 // Déplace le curseur à la nouvelle position.
}

void Spell(HeroStat& hero, enemiesStat* ennemis, pointeurEnnemis*& pointeur, CurseurPosition curseur)                                                                                               // Lance un sort sur un ennemi adjacent.
{
    pointeur = new pointeurEnnemis[9] {};
	int i = 0;
    for (int y = 0; y < 3; y++){
        for (int x = 0; x < 3; x++) {
            if (ennemiPresent(ennemis, hero.x + x - 1, hero.y + y - 1)) {

                pointeur[i].x = hero.x + x - 1;
                pointeur[i].y = hero.y + y - 1;
                i++;
            }
        }
    }
	if (i == 0) {
		delete[] pointeur;
        mvprintw(10, 10, "Aucun ennemi adjacent, le sort ne peut pas être lancé.");
		return;                                                                                                                                // Aucun ennemi adjacent, le sort ne peut pas être lancé.
	}
	curseur.x = pointeur[0].x;
	curseur.y = pointeur[0].y;
    bool jouer = false;
    while (!jouer) {
        afficherCarte(hero, ennemis);
        for (int j = 0; j < i; j++) {
            mvaddch(pointeur[j].y, pointeur[j].x, '*');
        }
        mvaddch(curseur.y, curseur.x, '^');
        int touche = getch();
        if (mouvementCurseur(hero, touche, ennemis, pointeur, curseur) == true) {
			int x = curseur.x;
			int y = curseur.y;
			for (int j = 0; j < NB_ENNEMIS; j++) {
				if (ennemis[j].x == x && ennemis[j].y == y) {
					ennemis[j].pv = 0;                                                                                                           // L'ennemi ciblé perd tous ses PV.
					break;
				}
			}
            jouer = true;
        }
    }
}


void heroMort(HeroStat& hero)                                                                                                                        // Gère la mort du heros.
{

	if (hero.oxygene <= 0)
	{
		heroMortHoxygene = true;                                                                                                               // Le heros est mort par manque d'oxygene.
	}
    else
    {
        heroMortEnnemi = true;                                                                                                                 // Le heros est mort par un ennemi.
    }
}


void heroHoxygene(HeroStat& hero)                                                                                                               // Gère l'oxygene du heros.
{
	if (map[indice(hero.x, hero.y)] == river)
	{
		hero.oxygene -= 1;                                                                                                                      // Le heros est dans la riviere, il perd de l'oxygene.
	}
	else
	{
		if (hero.oxygene < 3)                                                                                                                   // Le heros est hors de la riviere, il regagne de l'oxygene jusqu'a un maximum de 3.
		{
			hero.oxygene += 1;                                                                                                                  // Le heros est hors de la riviere, il gagne de l'oxygene.
		}
	}
	if (hero.oxygene <= 0)
	{
		hero.pv -= 1;                                                                                                                           // Le heros pert ses points de vie par manque d'oxygene.
	}
}

int main()                                                                                                                                      // Fonction principale : gere la partie.
{
#pragma comment(lib, "winmm.lib")
    Menu();                                                                                                                                     // Affiche le menu et initialise PDCurses.
    PlaySound(NULL, NULL, 0);
    while (!gameOver)
    {
        niveau += 1;
        if (niveau > niveauMax)
        {
            gameOver = true;
            break;
        }
        string nomFichier = "map/Niveau" + to_string(niveau) + ".txt";                                                                          // Construit le nom du fichier du niveau, to string convertit le int en string
        enemiesStat* ennemis = nullptr;
        if (!chargerCarte(nomFichier, ennemis)) {
            delete[] ennemis;
			gameOver = true;
            break;
        }
        HeroStat hero = { spawnX, spawnY, false, 1, 3};

        bool niveauFini = false;
        while (!gameOver && !niveauFini)
        {
            afficherCarte(hero, ennemis);                                                                                                       // Affiche la carte.
            int touche = getch();                                                                                                               // Attend une touche du joueur.

            if (touche == 'r' || touche == 'R' || touche == 27)
            {
                gameOver = true;
                break;                                                                                                                          // Ferme la boucle et la partie
            }

            if (touche == 'n' || touche == 'N')
            {
                niveauFini = true;
                break;
            }
            mouvementHero(hero, touche);                                                                                                        // Demande le mouvement ou sort.
            if (hero.hidden == true && sonCacher == false)
            {
                PlaySound(TEXT("C:\\Windows\\Media\\Ring01.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);                                   //TEXT sert à rendre le code compatible avec les configurations Windows
                sonCacher = true;                                                                                                               // NULL signifie qu'on ne fournit pas de ressource contenant le son.
            }                                                                                                                                   // SND_FILENAME indique que le premier argument est un nom de fichier.
            else if (hero.hidden == false && sonCacher == true)                                                                                 // SND_ASYNC fait que l'on est pas obligé d'attendre la fin du son pour continuer le programme.
            {                                                                                                                                   // SND_LOOP fait que le son se répète en boucle jusqu'à ce qu'on l'arrête.
                PlaySound(NULL, NULL, 0);
                sonCacher = false;
            }
            heroHoxygene(hero);
            
            if (Etat)
            {
                pointeurEnnemis* pointeur = nullptr;
                CurseurPosition curseur = { 0, 0 };
                Spell(hero, ennemis, pointeur, curseur);                                                                                        // Lance le sort demande.
                Etat = false;                                                                                                                   // Reinitialise l'etat du sort.
                delete[] pointeur;
                pointeur = nullptr;
				/*delete curseur;
                curseur = nullptr;*/
            }
            
            colisionDetection(hero, ennemis);                                                                                                   // Verifie les collisions apres l'action du heros.
            if (hero.pv <= 0)                                                                                                                   // Termine la partie si le heros n'a plus de PV.
            {
                heroMort(hero);
                gameOver = true;
                break;
            }

            if (hero.x == sortieX && hero.y == sortieY)                                                                                         // Le heros a atteint la sortie, il a fini le niveaux
            {
                niveauFini = true;
                PlaySound(TEXT("C:\\Windows\\Media\\Windows Proximity Notification.wav"), NULL, SND_FILENAME | SND_ASYNC);
                break;
            }

            mouvementEnnemie(ennemis, hero);                                                                                                    // Fait bouger les ennemis.
            colisionDetection(hero, ennemis);                                                                                                   // Verifie les collisions apres le mouvement des ennemis.
            if (hero.pv <= 0)                                                                                                                   // Termine la partie si le heros n'a plus de PV.
            {
                heroMort(hero);
                gameOver = true;
            }
        }
		delete[] ennemis;                                                                                                                       // Libere la memoire allouee pour les ennemis du niveau.]
    }
    if (niveau > niveauMax)
    {
        std::cout << "\n\n\n\n                              " << " Congratulation! You have completed all the levels." << endl;
    }
	if (heroMortHoxygene == true)
	{
		std::cout << "\n\n\n\n                                        " << " You ran out of oxygen." << endl;
	}
	else if (heroMortEnnemi == true){
		std::cout << "\n\n\n\n                                             " << " You died." << endl;
	}
    endwin();                                                                                                                                   // Termine PDCurses et restaure la console.
    std::cout << "\n\n\n\n                                              " << "Game Over\n\n\n" << endl;                                              // petit message de fin a améliorer peut etre
    return 0;                                                                                                                                   // Termine le programme.
}



// SPELS = appelle plusieurs fonction qui, prennent toute les position et vérifient si un ennemies est présent, posse en boucle une étoile au niveau des ennmies, et place un curseur sur une étoile qui se déplace d'étoile en étoile avec les fléches, on appuie sur espace pour lancer le sort, qui réduit la vie de l'ennemie a 0, on modifie les fonction précédente pour que tout ce qui a un rapport avec les ennmies ne marche que si ennemies est pv >0
//pathfending détecte les obstacle.