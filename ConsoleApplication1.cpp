// Librairies utilisées: PDCurses, iostream, random.

#include <iostream>
#include <random>
#include <curses.h>                                                                                             // Affichage dans la console et lecture du clavier avec PDCurses.
#include <cstdlib>                                                                                              // Pour les valeurs absolue
#include <fstream>                                                                                              // prendre fishier niveau
#include <string>

using namespace std;                                                                                            // Permet d'utiliser cout au lieu de std::cout.



// Constantes et variables globales du jeu.
const int Largeur = 20;
const int Hauteur = 10;
const int NB_ENNEMIS = 5;
int NerfEnnemie = 4;
int niveau = 0;
bool gameOver = false;



// Structure et terrain
enum tile_type                                                                                                  // Types de terrain possibles. SANS heros ni ennemis !!!!
{
    ground = 0,                                                                     // j'utilise plus les chifres
    river = 1,
    mountain = 2,
    Exit = 3,
};

tile_type map[Largeur * Hauteur];                                                                               // Tableau a UNE !!!  dimension contenant les terrains de la carte.


struct HeroStat // Regroupe la position et l'etat du heros.
{
    int x;
    int y;
    bool hidden;
    int pv;
};


struct enemiesStat                                                                                              // Regroupe la position et les points de vie d'un ennemi.
{
    int x;
    int y;
};



// Fonctions du jeu.
int indice(int x, int y)                                                                                        // Transforme les coordonnees (x, y) en indice du tableau, convertie 2d en tableau 1d
{
    return y * Largeur + x;
}



void Menu() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    string line;
    ifstream Menu("map/Menu.txt");
    int x = 0;
    while (getline(Menu, line)) {
        printw("%s\n", line.c_str());
    }
    bool play = false;
    while (!play) {
        int Choix = getch();

        if (Choix == 'a' || Choix == 'A'){
            play = true;
        }
        if (Choix == 'r' || Choix == 'R' || Choix == 27)
        {
            play = true;
            gameOver = true;
        }
    clear();
    refresh();

    }
}


void initialiserCarte(const int niveau)                                                                         // Créent les differents terrains de la carte
{
    string terrain;
    if (niveau == 1) {

        ifstream Niveaux1("map/Niveau1.txt");
        int x = 0;
        while (getline(Niveaux1, terrain)) {
            x++;
            for (int j = 0; j < Largeur; j++) {
                if (terrain[j] == '.') {
                    map[indice(j, x)] = ground;
                }
                else if (terrain[j] == 'A') {
                    map[indice(j, x)] = mountain;
                }
                else if (terrain[j] == '~') {
                    map[indice(j, x)] = river;
                }
                map[indice(17, 9)] = Exit;
                
            }
        }
        Niveaux1.close();

    }
    if (niveau == 2) {

        
        ifstream Niveaux2("map/Niveau2.txt");
        int x = 0;
        while (getline(Niveaux2, terrain)) {
            x++;
            for (int j = 0; j < Largeur; j++) {
                if (terrain[j] == '.') {
                    map[indice(j, x)] = ground;
                }
                else if (terrain[j] == 'A') {
                    map[indice(j, x)] = mountain;
                }
                else if (terrain[j] == '~') {
                    map[indice(j, x)] = river;
                }                
                map[indice(17, 9)] = Exit;
            }
        }
        Niveaux2.close();
    }
}




bool ennemiPresent(const enemiesStat ennemis[], int x, int y)                                                   // Verifie si un ennemi occupe une case, a la demande de l'intervenant
{

    for (int i = 0; i < NB_ENNEMIS; i++)
    {
        if (ennemis[i].x == x && ennemis[i].y == y)                                                             // Compare les coordonnees de chaque ennemi.
        {
            return true;
        }
    }
    return false;
}



void definirEnnemies(enemiesStat ennemis[], const HeroStat& hero)                                               // Place aleatoirement les ennemis sur la carte
{
    random_device rd;                                                                                           // Fournit une valeur pour initialiser mt19937
    mt19937 generateur(rd());                                                                                   // Génère les nombres pseudo-aléatoires
    // mt19937 est l'algorithme qui génère les nombres.
    uniform_int_distribution<int> aleatoireX(0, Largeur - 1);                                                   // Attribution aléoratoire des coordonnées x et y pour les ennemis
    uniform_int_distribution<int> aleatoireY(0, Hauteur - 1);

    for (int i = 0; i < NB_ENNEMIS; i++)
    {
        int x, y;
        do                                                                                                      // Tire des coordonnees jusqu'a trouver une case valide.
        {
            x = aleatoireX(generateur);
            y = aleatoireY(generateur);
        } while (map[indice(x, y)] != ground || (x == hero.x && y == hero.y) || ennemiPresent(ennemis, x, y));  //Ne pas apparaitre sur qulequchose
        ennemis[i] = { x, y};                                                                               // Definit ennemie
    };
}



void afficherCarte(const HeroStat& hero, const enemiesStat ennemis[])                                           // Affiche le terrain, le heros et les ennemis
{
    erase();                                                                                                    // PDCurses efface l'affichage precedent pour preparer le suivant.
    for (int y = 0; y < Hauteur; y++)
    {
        for (int x = 0; x < Largeur; x++)
        {
            char symbole;
            int i = indice(x, y);
            if (hero.x == x && hero.y == y)
            {
                if (hero.hidden) {                                                                              // vérifie si le héros est cacher ou non
                    symbole = 'S';
                }
                else {
                    symbole = 'H';
                }
            }
            else if (ennemiPresent(ennemis, x, y))
            {
                symbole = 'X';
            }
            else if (map[i] == ground)
            {
                symbole = '.';
            }
            else if (map[i] == river)
            {
                symbole = '~';
            }
            else if (map[i] == mountain)
            {
                symbole = 'A';
            }
            else
            {
                symbole = 'E';
            }

            mvaddch(y, x, symbole);                                                                             // PDCurses affiche un caractere aux coordonnees (y, x).
        }
    }

    mvprintw(Hauteur + 1, 5, "Z: haut | S: bas | Q: gauche | D: droite");
    mvprintw(Hauteur + 2, 15, "R : quitter");
    mvprintw(Hauteur + 3, 0, "PV : %d", hero.pv);                                                                    // affiche les  PV restant

    if (hero.hidden) {
        mvprintw(Hauteur + 4, 0, "Le heros est cache dans la riviere.");
    }
    else {
        mvprintw(Hauteur + 4, 30, "Le heros est visible.");
    }
    mvprintw(Hauteur + 6, 0, "Objectif atteindre la sortie Exit(E)");
    refresh();                                                                                                  // Met a jour l'ecran pour rendre visibles les changements.
}


void colisionDetection(HeroStat& hero, enemiesStat ennemis[]) {
    int x = ennemis[1].x;
    for (int i = 0; i < NB_ENNEMIS; i++) {
        if (hero.x == ennemis[i].x && hero.y == ennemis[i].y) {
            hero.pv -= 1;
        }

    }

}



void mouvementHero(HeroStat& hero, int mouvement)                                                              // Génere les deplacements et les collisions du heros
{
    int nouveauX = hero.x;
    int nouveauY = hero.y;

    if (mouvement == 'z' || mouvement == 'Z' || mouvement == KEY_UP)
    {
        nouveauY--;
    }
    else if (mouvement == 's' || mouvement == 'S' || mouvement == KEY_DOWN)
    {
        nouveauY++;
    }
    else if (mouvement == 'q' || mouvement == 'Q' || mouvement == KEY_LEFT)
    {
        nouveauX--;
    }
    else if (mouvement == 'd' || mouvement == 'D' || mouvement == KEY_RIGHT)
    {
        nouveauX++;
    }
    else
    {
        return;
    }

    if (nouveauX < 0 || nouveauX >= Largeur || nouveauY < 0 || nouveauY >= Hauteur)                             // Empeche le heros de sortir des limites de la carte.
    {
        return;
    }


    if (map[indice(nouveauX, nouveauY)] == mountain)                                                            // On ne peut pas traverser les montagnes.
    {
        return;
    }


    hero.x = nouveauX;                                                                                          // On actualise la position du heros.
    hero.y = nouveauY;

    hero.hidden = (map[indice(hero.x, hero.y)] == river);                                                       // Le heros devient invisible dans la riviere
}


void mouvementEnnemie(enemiesStat ennemis[], HeroStat& hero) {
    for (int i = 0; i < NB_ENNEMIS; i++) {
        int x = ennemis[i].x - hero.x;
        int y = ennemis[i].y - hero.y;
        int nouveauX = ennemis[i].x;
        int nouveauY = ennemis[i].y;

        if (std::abs(x) + std::abs(y) <= 3 && hero.hidden == false) {
            if (std::abs(x) == std::abs(y)) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> distr(0, 1);
                int a = distr(gen);
            }
            if (std::abs(x) > std::abs(y)) {
                if (x < 0)
                    nouveauX += 1;
                else {
                    nouveauX -= 1;
                }
            }
            else if (std::abs(x) < std::abs(y)) {
                if (y < 0) {
                    nouveauY += 1;
                }
                else {
                    nouveauY -= 1;
                }
            }
        }
        else {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distr(0, 3);
            int b = distr(gen);

            if (b == 0) {
                nouveauX += 1;
            }
            else if (b == 1) {
                nouveauX -= 1;
            }
            else if (b == 2) {
                nouveauY += 1;
            }
            else if (b == 3) {
                nouveauY -= 1;
            }
        }

        if (nouveauX < 0 || nouveauX >= Largeur || nouveauY < 0 || nouveauY >= Hauteur)                              // Empeche le heros de sortir des limites de la carte.
        {
            return;
        }


        if (map[indice(nouveauX, nouveauY)] == mountain)                                                            // On ne peut pas traverser les montagnes.
        {
            return;
        }

        if (map[indice(nouveauX, nouveauY)] == river)                                                               // On ne peut pas traverser les montagnes.
        {
            return;
        }
        if (map[indice(nouveauX, nouveauY)] == Exit)                                                                // On ne peut pas traverser les montagnes.
        {
            return;
        }

        ennemis[i].x = nouveauX;
        ennemis[i].y = nouveauY;
    }
}



int main()
{
    Menu();
    while (!gameOver) {
        niveau += 1;
        initialiserCarte(niveau);
        HeroStat hero = { 0, 0, false, 1 };
        NerfEnnemie -= 1;
        enemiesStat ennemis[NB_ENNEMIS] = {};
        definirEnnemies(ennemis, hero);


        // PDCURSES (ingnoble)
        initscr();                                                                                                  // Initialise PDCurses et l'ecran.
        cbreak();                                                                                                   // Lit les touches sans attendre Entree.
        noecho();                                                                                                   // N'affiche pas automatiquement les touches.
        keypad(stdscr, TRUE);                                                                                       // Active la reconnaissance des touches speciales.
        curs_set(0);                                                                                                // Masque le curseur de texte.

        bool niveauFini = false;
        while (!gameOver && !niveauFini)
        {
            // RENDER : affiche l'etat actuel du jeu.
            afficherCarte(hero, ennemis);
            // INPUT : attend une touche du clavier.
            int touche = getch();

            // UPDATE : traite la touche recue.
            if (touche == 'r' || touche == 'R' || touche == 27)                                                     // 27 est le code ASCII de la touche Echap
            {
                gameOver = true;
            }
            {
                mouvementHero(hero, touche);
                colisionDetection(hero, ennemis);
                mouvementEnnemie(ennemis, hero);
                colisionDetection(hero, ennemis);
            }
            if (hero.pv <= 0) {
                gameOver = true;
            }
            if (map[indice(hero.x, hero.y)] == Exit)                                                            // On ne peut pas traverser les montagnes.
            {
                niveauFini = true;
            }
        }
    }
    endwin();                                                                                                   // Termine PDCurses et restaure la console.
    std::cout << "\n\n\n\n                                              " << "Game Over\n\n\n" << endl;
    return 0;
}

//crer une structure de map qui continnent la map de facon dinamique
//alocation dinamique de tableau
// pour que la carte, les ennemis et le spawn ne soit pas fixes