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
#include <sys/wait.h>
#include <math.h>

#define BUFFSIZE 512
#define LIMITPROCESS 100


void checkNrOfArguments(int number0fArguments){
  if(number0fArguments != 3){
    perror("You must provide only two arguments!");
    exit(10);
  }
}

struct stat getFileInfo(char* filePath, struct stat file_info){
  if(lstat(filePath, &file_info) != 0){
    perror("Can't get info about the provided file!!");
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
  char buffer[BUFFSIZE];
  sprintf(buffer, "%s", fileName);

  if((fout = open(buffer, O_WRONLY | O_TRUNC | O_CREAT,S_IRWXU)) == -1 ){
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

void printToX(int fout, char* buffer, int r){
   if((write(fout, buffer, r))<0){
    perror("Can not write in file");
    exit(15);
  }
}

void printLastModifiedTime(struct stat file_info, int fout){
  char time_str[20], buffer[BUFFSIZE];
  strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&file_info.st_mtime));
  sprintf(buffer, "timpul ultimei modificari: %s\n", time_str);

  printToX(fout, buffer, strlen(buffer));
}

void printHeigthAndWidth(int fin, int fout){
  uint32_t heigth, width;
  char buffer[BUFFSIZE];
  width = getWidthOfBMPFile(fin);
  heigth = getHeigthOfBMPFile(fin);

  sprintf(buffer, "inaltime: %d\n", heigth);
  sprintf(buffer + strlen(buffer), "lungime: %d\n", width);
  
  printToX(fout, buffer, strlen(buffer));
}

void printSizeOfFile(int fout, struct stat file_info){
  uint32_t size;
  char buffer[BUFFSIZE];
  size = getSizeOfFile(file_info);

  sprintf(buffer, "dimensiune: %d\n", size);

  printToX(fout, buffer, strlen(buffer));
}

void printUserID(struct stat file_info, int fout){
  char buffer[BUFFSIZE];
  sprintf(buffer, "identificatorul utilizatorului: %ld\n", file_info.st_dev);

  printToX(fout, buffer, strlen(buffer));
}

void printLinksCount(struct stat file_info, int fout){
  char buffer[BUFFSIZE];
  sprintf(buffer, "contorul de legaturi: %ld\n", file_info.st_nlink);

  printToX(fout, buffer, strlen(buffer));
}

void printUserPermissions(mode_t sMode, int fout){
  char buffer[BUFFSIZE];
  sprintf(buffer, "drepturi de acces user: ");
  sprintf(buffer + strlen(buffer), (sMode & S_IRUSR) ? "r" : "-");
  sprintf(buffer + strlen(buffer), (sMode & S_IWUSR) ? "w" : "-");
  sprintf(buffer + strlen(buffer), "%s \n", (sMode & S_IXUSR) ? "x" : "-");

  printToX(fout, buffer, strlen(buffer));
}

void printGroupPermissions(mode_t sMode, int fout){
  char buffer[BUFFSIZE];
  sprintf(buffer, "drepturi de acces grup: ");
  sprintf(buffer + strlen(buffer), (sMode & S_IRGRP) ? "r" : "-");
  sprintf(buffer + strlen(buffer), (sMode & S_IWGRP) ? "w" : "-");
  sprintf(buffer + strlen(buffer), "%s \n", (sMode & S_IXGRP) ? "x" : "-");

  printToX(fout, buffer, strlen(buffer));
}

void printOthersPermissions(mode_t sMode, int fout){
  char buffer[BUFFSIZE];
  sprintf(buffer, "drepturi de acces altii: ");
  sprintf(buffer + strlen(buffer), (sMode & S_IROTH) ? "r" : "-");
  sprintf(buffer + strlen(buffer), (sMode & S_IWOTH) ? "w" : "-");
  sprintf(buffer + strlen(buffer), "%s \n", (sMode & S_IXOTH) ? "x" : "-");

  printToX(fout, buffer, strlen(buffer));
}

void printFileName(char* filePath, int fout){
  char buffer[BUFFSIZE];
  sprintf(buffer, "nume fisier: %s\n", filePath);

  printToX(fout, buffer, strlen(buffer));
}

void printBMPInfo(char* filePath, int fin, struct stat file_info, mode_t sMode, int fout){
  printFileName(filePath, fout);
  printHeigthAndWidth(fin, fout);
  printSizeOfFile(fout, file_info);
  printUserID(file_info, fout);
  printLastModifiedTime(file_info, fout);
  printLinksCount(file_info, fout);
  printUserPermissions(sMode, fout);
  printGroupPermissions(sMode, fout);
  printOthersPermissions(sMode, fout);
}

void printRegFileInfo(char* filePath, struct stat file_info, mode_t sMode, int fout){
  printFileName(filePath, fout);
  printSizeOfFile(fout, file_info);
  printUserID(file_info, fout);
  printLastModifiedTime(file_info, fout);
  printLinksCount(file_info, fout);
  printUserPermissions(sMode, fout);
  printGroupPermissions(sMode, fout);
  printOthersPermissions(sMode, fout);
}

void printDirInfo(char* filePath, struct stat file_info, mode_t sMode, int fout){
  printFileName(filePath, fout);
  printUserID(file_info, fout);
  printUserPermissions(sMode, fout);
  printGroupPermissions(sMode, fout);
  printOthersPermissions(sMode, fout);
}

//
void printSymbLinkInfo(char* filePath, struct stat file_info, mode_t sMode, int fout){
  printFileName(filePath, fout);
   struct stat tmp;
  if(stat(filePath, &tmp) != 0){
    perror("Can't get info about the provided file");
    exit(11);
  }
  printSizeOfFile(fout, file_info);
  char buffer[BUFFSIZE];
  sprintf(buffer, "Fisier target: ");
  printToX(fout, buffer, strlen(buffer));
  printSizeOfFile(fout, tmp);
  printUserPermissions(sMode, fout);
  printGroupPermissions(sMode, fout);
  printOthersPermissions(sMode, fout);
}
//

void duplicateBMP(int fin, char* filePath){
  int fout, r;
  fout = tryToOpenOutputFile(filePath);
  char buffer[BUFFSIZE];

  lseek(fin, 0, SEEK_SET);

  while((r = read(fin, buffer, 8)) > 0){
    printToX(fout, buffer, r);
  }

  close(fout);
}

void convertToGrayscaleOver8(int fin){
  off_t offset = 10;
  uint32_t bytesBeforeRasterData;
  char buffer[BUFFSIZE];

  lseek(fin, offset, SEEK_SET);
  read(fin, &bytesBeforeRasterData, sizeof(uint32_t));
  lseek(fin, bytesBeforeRasterData, SEEK_SET);

  while(read(fin, buffer, 3) > 0){
    int red = buffer[0];
    int green = buffer[1];
    int blue = buffer[2];

    char gray = (char)(0.299 * red + 0.587 * green + 0.114 * blue);

    buffer[0] = buffer[1] = buffer[2] = gray;

    lseek(fin, -3, SEEK_CUR);
    if((write(fin, buffer, 3))<0){
      perror("Can not write in file");
      exit(15);
    }
  }
}

void convertToGrayscaleUnder8(int fin, int bitCount){
  off_t offset = 54;
  char buffer[BUFFSIZE];
  int i=0;

  lseek(fin, offset, SEEK_SET);

  while(read(fin, buffer, 4) > 0 && i < pow(2, bitCount)){
    int red = buffer[0];
    int green = buffer[1];
    int blue = buffer[2];

    char gray = (char)(0.299 * red + 0.587 * green + 0.114 * blue);

    buffer[0] = buffer[1] = buffer[2] = gray;

    lseek(fin, -4, SEEK_CUR);
    if((write(fin, buffer, 3))<0){
      perror("Can not write in file");
      exit(15);
    }
    lseek(fin, 1, SEEK_CUR);
  }
}

void convertToGrayscale(char* filePath){
  off_t offset = 28;
  uint16_t bitCount;

  int fin;
  if((fin = open(filePath, O_RDWR)) < 0){
    perror("Input file can not be opened!");
    exit(13);
  }

  lseek(fin, offset, SEEK_SET);
  read(fin, &bitCount, sizeof(uint16_t));
  if(bitCount > 8){
    convertToGrayscaleOver8(fin);
  }
  else{
    convertToGrayscaleUnder8(fin, bitCount);
  }

  close(fin);
  printf("BMP file converted to grayscale successfully.\n");
}

void extractFilePath(char* filePath, char* buffer){
  for(int i=0; i<strlen(filePath); i++){
    if(filePath[i] == '/'){
      for(int j=0; j<strlen(filePath)-i; j++){
        buffer[j] = filePath[j+i+1];
      }
    }
  }
}

int printFileInfo(char* filePath, char* outputPath){
  int fin, fout, numberOfLinesWritten=0;
  struct stat file_info;
  mode_t sMode;
  char outputName[BUFFSIZE], buffer[BUFFSIZE-50];

  file_info = getFileInfo(filePath, file_info);
  sMode = file_info.st_mode;
  fin = tryToOpenFile(filePath);

  extractFilePath(filePath, buffer);
  sprintf(outputName, "%s/%s_statistica.txt",outputPath, buffer);
  fout = tryToOpenOutputFile(outputName);

  int type = getFileType(sMode, fin);
  if(type == 1){
    char buffer1[BUFFSIZE];
    sprintf(buffer1, "%s/%s_duplicate.bmp",outputPath, buffer);
    duplicateBMP(fin, buffer1);
    convertToGrayscale(buffer1);
    
    printBMPInfo(filePath, fin, file_info, sMode, fout);
    numberOfLinesWritten = 10;
  }
  else if(type == 0){
    printRegFileInfo(filePath, file_info, sMode, fout);
    numberOfLinesWritten = 8;
  }
  else if(type == 2){
    printDirInfo(filePath, file_info, sMode, fout);
    numberOfLinesWritten = 5;
  }
  else if(type == 3){
    printSymbLinkInfo(filePath, file_info, sMode, fout);
    numberOfLinesWritten = 6;
  }
  else{};

  close(fout);
  close(fin);
  return numberOfLinesWritten;
}


DIR *tryToOpenDir(char* dirPath){
  DIR *tmp;
  if((tmp = opendir(dirPath)) == NULL){
    perror("Directory path does not exist!");
    exit(20);
  }
  return tmp;
}

//
void crossDir(DIR* dir_path, char *dir_name, int argc, char* dirOut_path){
  struct dirent *dir_entry;
  int pid[LIMITPROCESS], i=0, numberOfLinesWritten;
  checkNrOfArguments(argc);

  while((dir_entry = readdir(dir_path)) != NULL){
    char entry_path[BUFFSIZE], output_path[BUFFSIZE];
    sprintf(entry_path, "%s/%s", dir_name, dir_entry->d_name);
    sprintf(output_path, "%s/%s", dir_name, dir_entry->d_name);
    if((pid[i] = fork()) < 0){
      perror("Ops, it looks like we can't create the process!");
      exit(errno);
    }
    if(pid[i] == 0){
      numberOfLinesWritten = printFileInfo(entry_path, dirOut_path);
      exit(numberOfLinesWritten);
    }

    i++;
  }

  int status;
  for(int j=0; j<i; j++){
    int fiu = wait(&status);
    if(fiu < 0){
      perror("..");
      exit(errno);
    }
    if(WIFEXITED(status)){
      printf("S-a incheiat procesul cu pid-ul %d si codul %d\n", fiu, WEXITSTATUS(status));
    }
  }

  closedir(dir_path);
}
//

int main(int argc, char* argv[]){

checkNrOfArguments(argc);

DIR *dir_path;
dir_path = tryToOpenDir(argv[1]);
tryToOpenDir(argv[2]);
crossDir(dir_path, argv[1], argc, argv[2]);

return 0;
}
