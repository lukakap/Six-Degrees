using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
#include <cstring>

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

//we can use this struct for both compare method
struct key {
  void* file;
  void* toCmp;
};


int actCmp(const void* aim, const void* currOffset){
  char* actor = (char*)((struct key*)aim)->toCmp;

  char* actorToCmp = (char*)(((struct key*)aim)->file) + *(int*)currOffset;

  return strcmp(actor,actorToCmp);
}


film returnFilmStruct(int* film_offset, void* movieFile){
  char* film_pointer = (char*)movieFile + *film_offset;

  int film_length = strlen(film_pointer) + 1; // '\0'


  int year = (int)*(film_pointer + film_length) + 1900;

  film temp;
  temp.title = film_pointer;
  // cout << temp.title << endl;
  temp.year = year;

  return temp;
}

// you should be implementing these two methods right here... 
bool imdb::getCredits(const string& player, vector<film>& films) const {
  
  int actors = *(int*)actorFile;


  key key;
  key.toCmp = (void*)player.c_str();
  key.file = (void*) actorFile;

  int* offset = (int*) bsearch(&key, (int*)actorFile + 1, actors, sizeof(int), actCmp);

  if(offset != NULL) {
    char* player_point = (char*)actorFile + *(int*)offset;

    int name_length = (int) strlen(player_point);

    if(name_length % 2 == 0) {
        name_length += 2;  //'\0' '\0'
    } else {
        name_length += 1;  // '\0'
    }


    short* n_films = (short*)(player_point + name_length);
  
    int additionalToShort = 0;
    if(!((name_length + sizeof(short)) % 4 == 0)) {
       additionalToShort = 2;
    }
  
    int* films_pointer = (int*)((char*)player_point + name_length + sizeof(short) + additionalToShort);

    for(int i = 0; i < *n_films; i++){
      int * film_offset = (int*)films_pointer + i;

      film temp = returnFilmStruct(film_offset, (void*) movieFile);

      films.insert(films.end(), temp);
     }

    return true;
  }


  return false; 
  }




int movCmp(const void* aim, const void* currOffset){

  film toLook = *(film*)((key*)aim)->toCmp;

  film toCmp = returnFilmStruct((int*)currOffset, (void*)((key*)aim)->file);

  if (toLook == toCmp){
    return 0;
  }

  if(toLook < toCmp) {
    return -1;
  }

  return 1;
}


bool imdb::getCast(const film& movie, vector<string>& players) const { 
  
  int nMovies = *(int*)movieFile;

  struct key key;
  key.toCmp = (void*)&movie;
  key.file = (void*)movieFile;

  int* offset = (int*)bsearch(&key, (int*)movieFile+1, nMovies, sizeof(int), movCmp);

  if(offset != NULL){
    char* pointerToMovie = (char*)movieFile + *offset;
    int year_length = 1;
    int film_length = (int)strlen(pointerToMovie) + 1; // '\0'
    if((film_length + year_length)%2 == 1){
      year_length += 1;
    }


    short* nOfActors = (short*)(pointerToMovie + film_length + year_length);
    int additionalBytesForShort = 0;
    if((film_length + year_length + sizeof(short)) % 4 != 0){
      additionalBytesForShort = 2;
    }

    int* actors = (int*)((char*)pointerToMovie + film_length + year_length + additionalBytesForShort + sizeof(short));

    for(int i = 0; i < *nOfActors; i++){
      int * actorOffset = (actors + i);
      string actorName = string((char*)actorFile + *actorOffset);

      // cout << "i:   " << i << "   " << actorName << endl;

      players.insert(players.end(), actorName);
    }

    return true;

  }
  
  return false; 
  }

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
