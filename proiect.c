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
#include <dirent.h>

#define BUFFSIZE 512

void checkNrOfArguments(int number0fArguments){
  if(number0fArguments != 2){
    perror("You must provide only one argument!");
    exit(10);
  }
}

struct stat getFileInfo(char* filePath, struct stat file_info){
  if(lstat(filePath, &file_info) != 0){
    perror("Can't get info about the provided file");
    exit(11);
  }
  return file_info;
}

//

int isBMP(int fin){
  off_t offset = 0;
  char tmp[5];
  lseek(fin, offset, SEEK_SET);
  read(fin, &tmp, sizeof(uint32_t));
  if(tmp[0] == 'B' && tmp[1] == 'M')
    return 1;
    else return 0;
}

int getFileType(mode_t sMode, int fin){
  if(isBMP(fin)){
    return 1;
  }
  else if(S_ISDIR(sMode)){
    return 2;
  }
  else if(S_ISLNK(sMode)){
    return 3;
  }
  else if(S_ISREG(sMode)){
    return 0;
  }
  return 4;
}

//

int tryToOpenFile(char* filePath){
  int fin;
  if((fin = open(filePath, O_RDONLY)) < 0){
    perror("Input file can not be opened!");
    exit(13);
  }
  return fin;
}

int tryToOpenOutputFile(char* fileName){
  int fout;
  if((fout = open("statistica.txt", O_WRONLY | O_TRUNC | O_CREAT,S_IRWXU)) == -1 ){
    perror("Error at creating the output file");
    exit(14);
  }
  return fout;
}

uint32_t getWidthOfBMPFile(int fin){
  off_t offset = 18;
  uint32_t width;
  lseek(fin, offset, SEEK_SET);
  read(fin, &width, sizeof(uint32_t));
  return width;
}

uint32_t getHeigthOfBMPFile(int fin){
  off_t offset = 22;
  uint32_t heigth;
  lseek(fin, offset, SEEK_SET);
  read(fin, &heigth, sizeof(uint32_t));
  return heigth;
}

uint32_t getSizeOfFile(struct stat file_info){
  off_t size = file_info.st_size;
  return size;
}

void printToX(int fout, char* buffer){
   if((write(fout, buffer, strlen(buffer)))<0){
    perror("Can not write in file");
    exit(15);
  }
}

void printLastModifiedTime(struct stat file_info, int fout){
  char time_str[20], buffer[BUFFSIZE];
  strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&file_info.st_mtime));
  sprintf(buffer, "timpul ultimei modificari: %s\n", time_str);

  printToX(fout, buffer);
}

void printHeigthAndWidth(int fin, int fout){
  uint32_t heigth, width;
  char buffer[BUFFSIZE];
  width = getWidthOfBMPFile(fin);
  heigth = getHeigthOfBMPFile(fin);

  sprintf(buffer, "inaltime: %d\n", heigth);
  sprintf(buffer + strlen(buffer), "lungime: %d\n", width);
  
  printToX(fout, buffer);
}

void printSizeOfFile(int fout, struct stat file_info){
  uint32_t size;
  char buffer[BUFFSIZE];
  size = getSizeOfFile(file_info);

  sprintf(buffer, "dimensiune: %d\n", size);

  printToX(fout, buffer);
}

void printUserID(struct stat file_info, int fout){
  char buffer[BUFFSIZE];
  sprintf(buffer, "identificatorul utilizatorului: %ld\n", file_info.st_dev);

  printToX(fout, buffer);
}

void printLinksCount(struct stat file_info, int fout){
  char buffer[BUFFSIZE];
  sprintf(buffer, "contorul de legaturi: %ld\n", file_info.st_nlink);

  printToX(fout, buffer);
}

void printUserPermissions(mode_t sMode, int fout){
  char buffer[BUFFSIZE];
  sprintf(buffer, "drepturi de acces user: ");
  sprintf(buffer + strlen(buffer), (sMode & S_IRUSR) ? "r" : "-");
  sprintf(buffer + strlen(buffer), (sMode & S_IWUSR) ? "w" : "-");
  sprintf(buffer + strlen(buffer), "%s \n", (sMode & S_IXUSR) ? "x" : "-");

  printToX(fout, buffer);
}

void printGroupPermissions(mode_t sMode, int fout){
  char buffer[BUFFSIZE];
  sprintf(buffer, "drepturi de acces grup: ");
  sprintf(buffer + strlen(buffer), (sMode & S_IRGRP) ? "r" : "-");
  sprintf(buffer + strlen(buffer), (sMode & S_IWGRP) ? "w" : "-");
  sprintf(buffer + strlen(buffer), "%s \n", (sMode & S_IXGRP) ? "x" : "-");

  printToX(fout, buffer);
}

void printOthersPermissions(mode_t sMode, int fout){
  char buffer[BUFFSIZE];
  sprintf(buffer, "drepturi de acces altii: ");
  sprintf(buffer + strlen(buffer), (sMode & S_IROTH) ? "r" : "-");
  sprintf(buffer + strlen(buffer), (sMode & S_IWOTH) ? "w" : "-");
  sprintf(buffer + strlen(buffer), "%s \n", (sMode & S_IXOTH) ? "x" : "-");

  printToX(fout, buffer);
}

void printFileName(char* pathFile, int fout){
  char buffer[BUFFSIZE];
  sprintf(buffer, "nume fisier: %s\n", pathFile);

  printToX(fout, buffer);
}

void printBMPInfo(char* pathFile, int fin, struct stat file_info, mode_t sMode, int fout){
  printFileName(pathFile, fout);
  printHeigthAndWidth(fin, fout);
  printSizeOfFile(fout, file_info);
  printUserID(file_info, fout);
  printLastModifiedTime(file_info, fout);
  printLinksCount(file_info, fout);
  printUserPermissions(sMode, fout);
  printGroupPermissions(sMode, fout);
  printOthersPermissions(sMode, fout);
}

void printRegFileInfo(char* pathFile, struct stat file_info, mode_t sMode, int fout){
  printFileName(pathFile, fout);
  printSizeOfFile(fout, file_info);
  printUserID(file_info, fout);
  printLastModifiedTime(file_info, fout);
  printLinksCount(file_info, fout);
  printUserPermissions(sMode, fout);
  printGroupPermissions(sMode, fout);
  printOthersPermissions(sMode, fout);
}

void printDirInfo(char* pathFile, struct stat file_info, mode_t sMode, int fout){
  printFileName(pathFile, fout);
  printUserID(file_info, fout);
  printUserPermissions(sMode, fout);
  printGroupPermissions(sMode, fout);
  printOthersPermissions(sMode, fout);
}

//
void printSymbLinkInfo(char* pathFile, struct stat file_info, mode_t sMode, int fout){
  printFileName(pathFile, fout);
   struct stat tmp;
  if(stat(pathFile, &tmp) != 0){
    perror("Can't get info about the provided file");
    exit(11);
  }
  printSizeOfFile(fout, file_info);
  printToX(fout, "Fisier target ");
  printSizeOfFile(fout, tmp);
  printUserPermissions(sMode, fout);
  printGroupPermissions(sMode, fout);
  printOthersPermissions(sMode, fout);
}
//

void printFileInfo(char* pathFile, int argc, int fout){
  int fin;
  struct stat file_info;
  mode_t sMode;

  checkNrOfArguments(argc);

  file_info = getFileInfo(pathFile, file_info);
  sMode = file_info.st_mode;
  fin = tryToOpenFile(pathFile);

  int type = getFileType(sMode, fin);
  if(type == 1){
    printBMPInfo(pathFile, fin, file_info, sMode, fout);
  }
  else if(type == 0){
    printRegFileInfo(pathFile, file_info, sMode, fout);
  }
  else if(type == 2){
    printDirInfo(pathFile, file_info, sMode, fout);
  }
  else if(type == 3){
    printSymbLinkInfo(pathFile, file_info, sMode, fout);
  }
  else{};

  close(fin);
}


DIR *tryToOpenDir(char* dirPath){
  DIR *tmp;
  if((tmp = opendir(dirPath)) == NULL){
    perror("Directory path does not exist!");
    exit(20);
  }
  return tmp;
}

void crossDir(DIR* dir_path, char *dir_name){
  struct dirent *dir_entry;
  int fout;
  fout = tryToOpenOutputFile("statistica.txt");
  while((dir_entry = readdir(dir_path)) != NULL){
    char entry_path[BUFFSIZE];
    sprintf(entry_path, "%s/%s", dir_name, dir_entry->d_name);
    printFileInfo(entry_path, 2, fout);
    printToX(fout, "\n\n");
  }

  close(fout);
  closedir(dir_path);
}

int main(int argc, char* argv[]){

checkNrOfArguments(argc);

DIR *dir_path;
dir_path = tryToOpenDir(argv[1]);
crossDir(dir_path, argv[1]);

return 0;
}
