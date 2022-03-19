#include <vector>
#include <list>
#include <set>
#include <string>
#include <iostream>
#include <iomanip>
#include "imdb.h"
#include "path.h"
using namespace std;



// void printPath(path path, string start){
//   path.reverse();

//   while(path.getLength() != 0){
//     string actor = path.getLastPlayer();
//     film movie = path.getLastMovie();
//     path.undoConnection();
//     string nextActor = path.getLastPlayer();
//     cout << actor << " was in \""<< movie.title <<"\" (" << movie.year << ") with " << nextActor << "." << endl;
//   }
// }

bool generateShortestPath(string start, string target, const imdb& db){
  list<path> partialPaths;
  set<string>previouslySeenActors;
  set<film> previouslySeenFilms;

  path startActorPartialPath (start);
  partialPaths.push_back(startActorPartialPath);

  while(!partialPaths.empty() && partialPaths.front().getLength() <= 5){
    path frontPath = partialPaths.front();
    partialPaths.pop_front();

    string lastActorName = frontPath.getLastPlayer();
    vector<film> lastActorMovies;
    db.getCredits(lastActorName, lastActorMovies);

    for(int i = 0 ; i < lastActorMovies.size(); i++){
      if(previouslySeenFilms.find(lastActorMovies[i]) == previouslySeenFilms.end()){
        film movie = lastActorMovies[i];
        previouslySeenFilms.insert(movie);
        vector<string> movieCast;
        db.getCast(movie, movieCast);

        for(int j = 0; j < movieCast.size(); j++){
          string actor = movieCast[j];
          if(previouslySeenActors.find(actor) == previouslySeenActors.end()){
            previouslySeenActors.insert(actor);

            path frontPathCopy = frontPath;
            frontPathCopy.addConnection(movie,actor);

            if(movieCast[j] == target) {
              cout << frontPathCopy;
              return true;
            } else {
              partialPaths.push_back(frontPathCopy);
            }
          }
        }
      }
    }

    
  }

  return false;
}




/**
 * Using the specified prompt, requests that the user supply
 * the name of an actor or actress.  The code returns
 * once the user has supplied a name for which some record within
 * the referenced imdb existsif (or if the user just hits return,
 * which is a signal that the empty string should just be returned.)
 *
 * @param prompt the text that should be used for the meaningful
 *               part of the user prompt.
 * @param db a reference to the imdb which can be used to confirm
 *           that a user's response is a legitimate one.
 * @return the name of the user-supplied actor or actress, or the
 *         empty string.
 */

static string promptForActor(const string& prompt, const imdb& db)
{
  // vector<string> pl;
  // film temp;
  // temp.title = "Fight Club";
  // temp.year = 1999;
  // db.getCast(temp,pl);
  string response;
  while (true) {
    cout << prompt << " [or <enter> to quit]: ";
    getline(cin, response);
    if (response == "") return "";
    vector<film> credits;
    if (db.getCredits(response, credits)) return response;
    cout << "We couldn't find \"" << response << "\" in the movie database. "
	 << "Please try again." << endl;
  }
  return "";
}

/**
 * Serves as the main entry point for the six-degrees executable.
 * There are no parameters to speak of.
 *
 * @param argc the number of tokens passed to the command line to
 *             invoke this executable.  It's completely ignored
 *             here, because we don't expect any arguments.
 * @param argv the C strings making up the full command line.
 *             We expect argv[0] to be logically equivalent to
 *             "six-degrees" (or whatever absolute path was used to
 *             invoke the program), but otherwise these are ignored
 *             as well.
 * @return 0 if the program ends normally, and undefined otherwise.
 */

int main(int argc, const char *argv[])
{
  imdb db(determinePathToData(argv[1])); // inlined in imdb-utils.h
  if (!db.good()) {
    cout << "Failed to properly initialize the imdb database." << endl;
    cout << "Please check to make sure the source files exist and that you have permission to read them." << endl;
    return 1;
  }
  
  while (true) {
    string source = promptForActor("Actor or actress", db);
    if (source == "") break;
    string target = promptForActor("Another actor or actress", db);
    if (target == "") break;
    if (source == target) {
      cout << "Good one.  This is only interesting if you specify two different people." << endl;
    } else {
      // replace the following line by a call to your generateShortestPath routine... 
      if(!generateShortestPath(source, target, db)){
        cout << endl << "No path between those two people could be found." << endl << endl;
      }
    }
  }
  
  cout << "Thanks for playing!" << endl;
  return 0;
}