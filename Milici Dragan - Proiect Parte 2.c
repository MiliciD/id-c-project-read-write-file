#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_CONTURI 100
#define NUME_FISIER "conturi_bancare.csv" // Numele fisierului pentru date

// Am sters mare parte din comentarile vechi sa fie ceva mai citet codul sursa
// Am adaugat comentarii in general la ce era si intainte si mai multe detalii la functiile noi


// Structura client
typedef struct {
    int id_cont;
    char numar_cont[7]; // Format ROXXXX
    char nume_client[50];
    double sold;
} ContBancar;

//  Variabile Globale 
ContBancar conturi[MAX_CONTURI];
int id_conturi = 0; // Numarul curent de conturi in memorie
int id_maxim_cont = 0; // Urmareste cel mai mare ID folosit


// Functia pentru compararea conturilor după ID (folosită de qsort)
// Am vrut ca atunci cand se afiseaza toate conturile sa le sortez dupa ID
// pentru lizibilitate
// Functia asta e o functie ajutatoare
int compara_conturi_dupa_id(const void *a, const void *b) {
    ContBancar *contA = (ContBancar *)a;
    ContBancar *contB = (ContBancar *)b;
    return contA->id_cont - contB->id_cont;
}

// Funcție pentru sortarea conturilor dupa ID
void sorteaza_conturi_dupa_id() {
    if (id_conturi > 1) {
        qsort(conturi, id_conturi, sizeof(ContBancar), compara_conturi_dupa_id);
    }
}

// Functia pentru curatarea bufferului de intrare 
void curata_buffer() {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}

// Functia pentru curatarea ecranului
// Aici am intampinat o problema, daca folosesc system('cls') programul afiseaza corect doar in .exe
// Dar daca folosesc secnventa ANSI programul afiseaza corect in consold, dar nu in .exe
// Din moment ce utilizatorul ar folosi .exe am lasat varianta aceea si am comentat-o pe cealalta
// Dar am lasat-o in cod pentru testari
void curatare_ecran() {
    #ifdef _WIN32
        system("cls");
    #else
        // system("clear"); 
        printf("\033[H\033[J"); 
        fflush(stdout);
    #endif
}

// Pauza pana la apasarea Enter
void pauza() {
    printf("\nApasati Enter pentru a continua...");
    getchar(); // Modificat pentru a asigura că așteaptă corect după Enter
}

// Algoritmul pt generarea unui nr de cont
void generare_cont(char *numar_cont) {
    sprintf(numar_cont, "RO%04d", (rand() % 9000) + 1000); // Genereaza numere intre RO1000 si RO9999
}

// Functie pt a formata numele 
void formateaza_nume(char* nume) {
    char buffer[50];
    int j = 0;
    int capitalize = 1;
    int found_non_space = 0; // Flag sa vedem daca numele contine altceva decat spatii

    for (int i = 0; nume[i] != '\0'; i++) {
        if (isspace((unsigned char)nume[i])) {
            if (j > 0 && buffer[j - 1] != ' ') {
                buffer[j++] = ' ';
            }
            capitalize = 1;
        } else {
            found_non_space = 1; // Am gasit un caracter non-spatiu
            if (capitalize) {
                buffer[j++] = toupper((unsigned char)nume[i]);
                capitalize = 0;
            } else {
                buffer[j++] = tolower((unsigned char)nume[i]);
            }
            // Prevent buffer overflow
             if (j >= sizeof(buffer) - 1) {
                 break;
             }
        }
    }
    // Stergere spatiu gol ramas
    if (j > 0 && buffer[j - 1] == ' ') {
        j--;
    }
    buffer[j] = '\0'; 

    // Copiaza inapoi doar daca s-a gasit cel putin un caracter non-spatiu
    if (found_non_space && j > 0) {
        strcpy(nume, buffer);
    } else {
        // Input era gol sau doar spatii
        nume[0] = '\0';
    }
}


ContBancar* gaseste_cont(const char* input) {

    char* endptr;
    long id_cautat = strtol(input, &endptr, 10);
    int este_numar_valid = (endptr != input && *endptr == '\0');

    for (int i = 0; i < id_conturi; i++) {
        // 2. Compara cu ID-ul daca inputul a fost un numar valid
        if (este_numar_valid && conturi[i].id_cont == id_cautat) {
            // printf("DEBUG: Gasit dupa ID: %d\n", conturi[i].id_cont);
            return &conturi[i];
        }

        // 3. Compara cu Numarul de Cont
        if (strcmp(conturi[i].numar_cont, input) == 0) {
            // printf("DEBUG: Gasit dupa Numar Cont: %s\n", conturi[i].numar_cont);
            return &conturi[i];
        }

        // 4. Compara cu Numele Clientului (case-insensitive)
        #ifdef _WIN32
            if (_stricmp(conturi[i].nume_client, input) == 0) { // Windows
        #else
            if (strcasecmp(conturi[i].nume_client, input) == 0) { // POSIX
        #endif
            // printf("DEBUG: Gasit dupa Nume: %s\n", conturi[i].nume_client);
            return &conturi[i];
        }
    }
    // printf("DEBUG: Nu s-a gasit contul pentru input: '%s'\n", input);
    return NULL; // Nu a fost gasit
}



// Functia pentru incarcarea conturilor din fisier
void incarca_conturi() {
    FILE *fisier = fopen(NUME_FISIER, "r");
    if (fisier == NULL) {
        id_conturi = 0;
        id_maxim_cont = 0;
        return;
    }

    char linie[200];
    id_conturi = 0;
    id_maxim_cont = 0;

    while (fgets(linie, sizeof(linie), fisier) != NULL && id_conturi < MAX_CONTURI) {
        linie[strcspn(linie, "\n")] = 0; // Elimina newline

        if (strlen(linie) < 5) continue; // Ignora liniile foarte scurte/goale

        char *id_str = strtok(linie, ",");
        char *nr_cont_str = strtok(NULL, ",");
        char *nume_str = strtok(NULL, ",");
        char *sold_str = strtok(NULL, ",");

        // Verifica daca toate componentele au fost gasite
        if (id_str && nr_cont_str && nume_str && sold_str) {
            conturi[id_conturi].id_cont = atoi(id_str);


            strncpy(conturi[id_conturi].numar_cont, nr_cont_str, sizeof(conturi[id_conturi].numar_cont) - 1);
            conturi[id_conturi].numar_cont[sizeof(conturi[id_conturi].numar_cont) - 1] = '\0';

            strncpy(conturi[id_conturi].nume_client, nume_str, sizeof(conturi[id_conturi].nume_client) - 1);
            conturi[id_conturi].nume_client[sizeof(conturi[id_conturi].nume_client) - 1] = '\0';

            conturi[id_conturi].sold = atof(sold_str);

            // Actualizam id_maxim_cont
            if (conturi[id_conturi].id_cont > id_maxim_cont) {
                id_maxim_cont = conturi[id_conturi].id_cont;
            }

            id_conturi++;
        } else {
             printf("Warning: Linie ignorata in %s (format invalid): %s...\n", NUME_FISIER, linie);
        }
    }

    fclose(fisier);
    
    // Sortarea conturilor după ID
    sorteaza_conturi_dupa_id();
    
    if (id_conturi > 0) {
        printf("Info: %d cont(uri) incarcate din %s. ID maxim: %d\n", id_conturi, NUME_FISIER, id_maxim_cont);
    }
     if (id_conturi >= MAX_CONTURI && !feof(fisier)) {
        printf("Warning: Numarul maxim de conturi (%d) a fost atins. Unele date din fisier nu au fost incarcate.\n", MAX_CONTURI);
    }
}

// Functia pentru salvarea conturilor in fisier
void salveaza_conturi() {
    // Sortarea inainte de salvare
    sorteaza_conturi_dupa_id();
    
    FILE *fisier = fopen(NUME_FISIER, "w");
    if (fisier == NULL) {
        perror("Eroare critica la salvarea datelor");
        printf("Eroare: Nu s-a putut deschide fisierul %s pentru scriere.\n", NUME_FISIER);
        pauza(); // Asteapta ca utilizatorul sa vada eroarea
        return;
    }

    for (int i = 0; i < id_conturi; i++) {
        fprintf(fisier, "%d,%s,%s,%.2f\n",
                conturi[i].id_cont,
                conturi[i].numar_cont,
                conturi[i].nume_client,
                conturi[i].sold);
    }

    fclose(fisier);
}



// 1. Afisare toate conturile
void afisare_conturi() {
    curatare_ecran();
    printf("=== LISTA CONTURI ===\n\n");
    if (id_conturi == 0) {
        printf("Nu exista conturi inregistrate.\n");
    } else {
        // Sortăm conturile înainte de afișare
        sorteaza_conturi_dupa_id();
        
        printf("%-5s %-10s %-30s %-15s\n", "ID", "Nr. Cont", "Nume Client", "Sold");
        printf("--------------------------------------------------------------\n");
        for (int i = 0; i < id_conturi; i++) {
            printf("%-5d %-10s %-30s %15.2f\n", // Aliniat sold la dreapta
                  conturi[i].id_cont, conturi[i].numar_cont, conturi[i].nume_client, conturi[i].sold);
        }
         printf("--------------------------------------------------------------\n");
         printf("Total conturi: %d\n", id_conturi);
    }
    pauza();
}


void adauga_cont() {
    curatare_ecran();
    printf("=== ADAUGARE CONT NOU ===\n\n");

    if (id_conturi >= MAX_CONTURI) {
        printf("Nu se mai pot adauga conturi (limita maxima %d atinsa).\n", MAX_CONTURI);
        pauza();
        return;
    }

    ContBancar cont_nou;  //

    // Cauta  ID disponibil 
    int id_disponibil = 1; // Începem de la 1
    int id_gasit = 0;
    
    while (!id_gasit) {
        id_gasit = 1; // var auxiliara
        
        // verificare ID
        for (int i = 0; i < id_conturi; i++) {
            if (conturi[i].id_cont == id_disponibil) {
                id_disponibil++; // Se incearca urmatorul
                id_gasit = 0;    // continua cutarea
                break;
            }
        }
    }
    
    // noul id disponibil
    cont_nou.id_cont = id_disponibil;
    
    // Se actualizeaza id maxim daca e necesar
    if (id_disponibil > id_maxim_cont) {
        id_maxim_cont = id_disponibil;
    }
    

    generare_cont(cont_nou.numar_cont);

    // Citire nume
    char nume_temp[50];
    printf("Introduceti numele si prenumele clientului (0 = Anulare): ");
    if (fgets(nume_temp, sizeof(nume_temp), stdin) == NULL) {
        printf("\nEroare la citirea numelui. Operatie anulata.\n");
        pauza();
        return;
    }
    nume_temp[strcspn(nume_temp, "\n")] = 0; // Elimina newline

    if (strcmp(nume_temp, "0") == 0) {
        printf("\nOperatie anulata de utilizator.\n");
        pauza();
        return;
    }

    formateaza_nume(nume_temp); // Formateaza numele

    if (strlen(nume_temp) == 0) {
        printf("\nNume invalid (gol sau contine doar spatii). Operatie anulata.\n");
        pauza();
        return;
    }
    strcpy(cont_nou.nume_client, nume_temp);

    // Citeste soldul initial
    double sold_initial = -1.0;
    char input_sold[50];
    printf("Introduceti soldul initial (>= 0): ");
    while (sold_initial < 0) {
        if (fgets(input_sold, sizeof(input_sold), stdin) == NULL) {
            printf("Eroare la citire. Incercati din nou: ");
            continue;
        }
        input_sold[strcspn(input_sold, "\n")] = 0; // Elimina newline

        char *endptr;
        sold_initial = strtod(input_sold, &endptr);

        if (endptr == input_sold || *endptr != '\0' || sold_initial < 0) {
            printf("Suma invalida. Introduceti un numar pozitiv sau 0: ");
            sold_initial = -1.0; // Reseteaza pentru a reintra in bucla
        }
    }
    cont_nou.sold = sold_initial;

    // Adauga contul nou in array
    conturi[id_conturi] = cont_nou;
    id_conturi++;
    
    // Sotare conturi
    sorteaza_conturi_dupa_id();

    printf("\nCont adaugat cu succes!\n");
    printf("ID: %d | Numar: %s | Nume: %s | Sold: %.2f\n",
           cont_nou.id_cont, cont_nou.numar_cont, cont_nou.nume_client, cont_nou.sold);

    salveaza_conturi(); // Salveaza  dupa adaugare
    pauza();
}

void sterge_cont() {
    curatare_ecran();
    printf("=== STERGERE CONT ===\n\n");

    if (id_conturi == 0) {
        printf("Nu exista conturi de sters.\n");
        pauza();
        return;
    }

    char input[50];
    printf("Introduceti ID, Nr. Cont sau Nume pentru stergere (0 = Anulare): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
         printf("\nEroare la citire. Operatie anulata.\n");
         pauza();
         return;
    }
    input[strcspn(input, "\n")] = 0; // Elimina newline

    if (strcmp(input, "0") == 0) {
        printf("\nOperatie anulata.\n");
        pauza();
        return;
    }

    ContBancar* cont_gasit = gaseste_cont(input);

    if (cont_gasit == NULL) {
        printf("\nContul '%s' nu a fost gasit.\n", input);
    } else {
        printf("\nCont gasit:\n ID: %d | Numar: %s | Nume: %s | Sold: %.2f\n",
               cont_gasit->id_cont, cont_gasit->numar_cont, cont_gasit->nume_client, cont_gasit->sold);

        printf("Confirmati stergerea? (da/nu): ");
        char confirmare[10];
        if (fgets(confirmare, sizeof(confirmare), stdin) == NULL) {
             printf("\nEroare la citire confirmare. Stergere anulata.\n");
             pauza();
             return;
        }
        confirmare[strcspn(confirmare, "\n")] = 0; // Elimina newline

        int sterge = 0;
        #ifdef _WIN32
            if (_stricmp(confirmare, "da") == 0) sterge = 1;
        #else
            if (strcasecmp(confirmare, "da") == 0) sterge = 1;
        #endif

        if (sterge) {
            int index_sters = -1;
            for (int i = 0; i < id_conturi; i++) {
                if (&conturi[i] == cont_gasit) {
                    index_sters = i;
                    break;
                }
            }

            if (index_sters != -1) {
                // Muta elementele ramase
                for (int i = index_sters; i < id_conturi - 1; i++) {
                    conturi[i] = conturi[i + 1];
                }
                id_conturi--;
                printf("\nCont sters cu succes!\n");
                salveaza_conturi(); // Salveaza dupa stergere
            } else {
                printf("\nEroare interna la stergere (index negasit).\n");
            }
        } else {
            printf("\nStergere anulata.\n");
        }
    }
    pauza();
}

// Functie de ajutor (Depunere/Retragere)
void operatiune_suma(int tip_operatiune) {
    curatare_ecran();
    printf("=== %s ===\n\n", (tip_operatiune == 1) ? "DEPUNERE NUMERAR" : "RETRAGERE NUMERAR");

     if (id_conturi == 0) {
        printf("Nu exista conturi inregistrate.\n");
        pauza();
        return;
    }

    char input[50];
    printf("Introduceti ID, Nr. Cont sau Nume (0 = Anulare): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
         printf("\nEroare la citire. Operatie anulata.\n");
         pauza();
         return;
     }
    input[strcspn(input, "\n")] = 0;

     if (strcmp(input, "0") == 0) {
        printf("\nOperatie anulata.\n");
        pauza();
        return;
    }

    ContBancar* cont = gaseste_cont(input);
    if (cont == NULL) {
        printf("\nContul '%s' nu a fost gasit.\n", input);
    } else {
         printf("\nCont gasit:\n ID: %d | Nume: %s | Sold curent: %.2f\n", cont->id_cont, cont->nume_client, cont->sold);

         double suma = -1.0;
         char input_suma[50];
         printf("Introduceti suma de %s (> 0): ", (tip_operatiune == 1) ? "depus" : "retras");

         while (suma <= 0) { // Suma trebuie sa fie strict pozitiva
             if (fgets(input_suma, sizeof(input_suma), stdin) == NULL) {
                 printf("Eroare la citire. Incercati din nou: ");
                 continue;
             }
             input_suma[strcspn(input_suma, "\n")] = 0; // Elimina newline

             char *endptr;
             suma = strtod(input_suma, &endptr);

             if (endptr == input_suma || *endptr != '\0' || suma <= 0) {
                 printf("Suma invalida. Introduceti un numar strict pozitiv (> 0): ");
                 suma = -1.0; // Reseteaza pentru a reintra in bucla
             }
         }

         if (tip_operatiune == 1) { // Depunere
            cont->sold += suma;
            printf("\nDepunere reusita! Sold nou: %.2f\n", cont->sold);
            salveaza_conturi();
         } else { // Retragere
             if (cont->sold >= suma) {
                 cont->sold -= suma;
                 printf("\nRetragere reusita! Sold nou: %.2f\n", cont->sold);
                 salveaza_conturi();
             } else {
                 printf("\nFonduri insuficiente! Sold curent: %.2f, Suma ceruta: %.2f\n", cont->sold, suma);
             }
         }
    }
    pauza();
}

// Depunere
void depune_suma() {
    operatiune_suma(1);
}

// Retragere
void retrage_suma() {
    operatiune_suma(2);
}

// Cautare cont 
void cauta_cont() {
    curatare_ecran();
    printf("=== CAUTARE CONT ===\n\n");

     if (id_conturi == 0) {
        printf("Nu exista conturi pentru cautare.\n");
        pauza();
        return;
    }

    char input[50];
    printf("Introduceti ID, Nr. Cont sau Nume (0 = Anulare): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
         printf("\nEroare la citire. Operatie anulata.\n");
         pauza();
         return;
     }
    input[strcspn(input, "\n")] = 0;

     if (strcmp(input, "0") == 0) {
        printf("\nOperatie anulata.\n");
        pauza();
        return;
    }

    ContBancar* cont = gaseste_cont(input);
    if (cont != NULL) {
        printf("\n--- Cont Gasit ---\n");
        printf("ID Cont     : %d\n", cont->id_cont);
        printf("Numar Cont  : %s\n", cont->numar_cont);
        printf("Nume Client : %s\n", cont->nume_client);
        printf("Sold        : %.2f\n", cont->sold);
        printf("------------------\n");
    } else {
        printf("\nNu a fost gasit niciun cont care sa corespunda criteriului '%s'.\n", input);
    }
    pauza();
}


int main() {
    srand(time(NULL)); 
    curatare_ecran();

    incarca_conturi(); // Incarca datele la pornire
    if (id_conturi > 0) { // Afiseaza mesajul doar daca s-au incarcat conturi
      pauza(); // Pauza sa vedem mesajul de incarcare
    }


    int optiune = -1;
    do {
        curatare_ecran();
        printf("\n=== Banca Mea ===\n");
        printf("Conturi existente: %d / %d\n", id_conturi, MAX_CONTURI);
        printf("---------------------------\n");
        printf("1. Afisare toate conturile\n");
        printf("2. Adaugare cont nou\n");
        printf("3. Stergere cont existent\n");
        printf("4. Depunere numerar\n");
        printf("5. Retragere numerar\n");
        printf("6. Cautare cont specific\n");
        printf("0. Iesire si Salvare\n");
        printf("---------------------------\n");
        printf("Alegeti o optiune: ");

        char input_optiune[10];
        if (fgets(input_optiune, sizeof(input_optiune), stdin) == NULL) {
            optiune = -1; // Eroare la citire
            printf("\nEroare la citirea optiunii.\n");
            continue;
        }
        
        input_optiune[strcspn(input_optiune, "\n")] = 0; // Elimina newline
        
        char *endptr;
        optiune = strtol(input_optiune, &endptr, 10);
        if (endptr == input_optiune || *endptr != '\0') {
            optiune = -1; // Inputul gresit
            printf("\nOptiune invalida! '%s' nu este o optiune valida.\n", input_optiune);
            pauza();
            continue;
        }

        switch (optiune) {
            case 1: afisare_conturi(); break;
            case 2: adauga_cont(); break;
            case 3: sterge_cont(); break;
            case 4: depune_suma(); break;
            case 5: retrage_suma(); break;
            case 6: cauta_cont(); break;
            case 0:
                printf("\nSe salveaza datele...\n");
                // Salvarea finala pt fisier
                break;
            default:
                printf("\nOptiune invalida! '%s' nu este o optiune recunoscuta.\n", input_optiune);
                pauza();
                break;
        }
    } while (optiune != 0);

    salveaza_conturi(); // Salveaza datele la iesirea normala
    printf("\nDatele au fost salvate. La revedere!\n");

    return 0;
}