#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>

#define BUFFSIZE 1000

void verificaNrArgumente(int arg){
  if(arg != 2){
    perror("Trebuie să specifici doar un singur argument");
    exit(10);
  }
}

struct stat infoFisier(char* filePath, struct stat file_info){
  if(stat(filePath, &file_info) != 0){
    perror("Nu se pot obține informații despre fișierul specificat.");
    exit(11);
  }

  return file_info;
}

void verificaReg(mode_t sMode){
  if(S_ISREG(sMode) == 0){
    perror("Acesta nu este un fișier regular.");
    exit(12);
  }
}

int deschidereFisier(char* filePath){
  int fin;
  if((fin = open(filePath, O_RDONLY)) < 0){
    perror("Nu se poate deschide fișierul de intrare!");
    exit(13);
  }
  return fin;
}

int creareFisier(char* fileName){
  int fout;
  if((fout = open("statistica.txt", O_WRONLY | O_TRUNC | O_CREAT,S_IRWXU)) == -1 ){
    perror("Eroare la crearea fișierului de iesire.");
    exit(14);
  }
  return fout;
}

uint32_t latimeBMP(int fin){
  off_t offset = 18;
  uint32_t width;
  lseek(fin, offset, SEEK_SET);
  read(fin, &width, sizeof(uint32_t));
  return width;
}

uint32_t inaltimeBMP(int fin){
  off_t offset = 20;
  uint32_t heigth;
  lseek(fin, offset, SEEK_SET);
  read(fin, &heigth, sizeof(uint32_t));
  return heigth;
}

uint32_t dimensiuneFisier(int fin){
  off_t offset = 2;
  uint32_t size;
  lseek(fin, offset, SEEK_SET);
  read(fin, &size, sizeof(uint32_t));
  return size;
}

void printToX(int fout, char* buffer){
   if((write(fout, buffer, strlen(buffer)))<0){
    perror("Nu se poate scrie în fișier");
    exit(10);
  }
}

void timpulUltimeiModificari(struct stat file_info, int fout){
  char time_str[20], buffer[BUFFSIZE];
  strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&file_info.st_mtime));
  sprintf(buffer, "timpul ultimei modificari: %s\n", time_str);

  printToX(fout, buffer);
}

void afizareHW(int fin, int fout){
  uint32_t heigth, width;
  char buffer[BUFFSIZE];
  width = latimeBMP(fin);
  heigth = inaltimeBMP(fin);

  sprintf(buffer, "inaltime: %d\n", heigth);
  sprintf(buffer + strlen(buffer), "lungime: %d\n", width);
  
  printToX(fout, buffer);
}

void afisareSize(int fin, int fout){
  uint32_t size;
  char buffer[BUFFSIZE];
  size = dimensiuneFisier(fin);

  sprintf(buffer, "dimensiune: %d\n", size);

  printToX(fout, buffer);
}

void afisareID(struct stat file_info, int fout){
  char buffer[BUFFSIZE];
  sprintf(buffer, "identificatorul utilizatorului: %ld\n", file_info.st_dev);

  printToX(fout, buffer);
}

void afisareCleg(struct stat file_info, int fout){
  char buffer[BUFFSIZE];
  sprintf(buffer, "contorul de legaturi: %ld\n", file_info.st_nlink);

  printToX(fout, buffer);
}

void afisareAccesUser(mode_t sMode, int fout){
  char buffer[BUFFSIZE];
  sprintf(buffer, "drepturi de acces user: ");
  sprintf(buffer + strlen(buffer), (sMode & S_IRUSR) ? "r" : "-");
  sprintf(buffer + strlen(buffer), (sMode & S_IWUSR) ? "w" : "-");
  sprintf(buffer + strlen(buffer), "%s \n", (sMode & S_IXUSR) ? "x" : "-");

  printToX(fout, buffer);
}

void afisareAccesGrup(mode_t sMode, int fout){
  char buffer[BUFFSIZE];
  sprintf(buffer, "drepturi de acces grup: ");
  sprintf(buffer + strlen(buffer), (sMode & S_IRGRP) ? "r" : "-");
  sprintf(buffer + strlen(buffer), (sMode & S_IWGRP) ? "w" : "-");
  sprintf(buffer + strlen(buffer), "%s \n", (sMode & S_IXGRP) ? "x" : "-");

  printToX(fout, buffer);
}

void afisareAccesAltii(mode_t sMode, int fout){
  char buffer[BUFFSIZE];
  sprintf(buffer, "drepturi de acces altii: ");
  sprintf(buffer + strlen(buffer), (sMode & S_IROTH) ? "r" : "-");
  sprintf(buffer + strlen(buffer), (sMode & S_IWOTH) ? "w" : "-");
  sprintf(buffer + strlen(buffer), "%s \n", (sMode & S_IXOTH) ? "x" : "-");

  printToX(fout, buffer);
}

void afisareNumefile(char* pathFile, int fout){
  char buffer[BUFFSIZE];
  sprintf(buffer, "Nume fisier: %s\n", pathFile);

  printToX(fout, buffer);
}

int main(int argc, char* argv[]){

  int fin, fout;
  struct stat file_info;
  mode_t sMode;

  verificaNrArgumente(argc);

  file_info = infoFisier(argv[1], file_info);
  sMode = file_info.st_mode;

  verificaReg(sMode);

  fin = deschidereFisier(argv[1]);
  fout = creareFisier("statistica.txt");

  afisareNumefile(argv[1], fout);
  afizareHW(fin, fout);
  afisareSize(fin, fout);
  afisareID(file_info, fout);
  timpulUltimeiModificari(file_info, fout);
  afisareCleg(file_info, fout);
  afisareAccesUser(sMode, fout);
  afisareAccesGrup(sMode, fout);
  afisareAccesAltii(sMode, fout);

  close(fin);
  close(fout);

  return 0;
}