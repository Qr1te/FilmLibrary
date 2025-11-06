#include "../includes/MovieManager.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <set>

MovieManager::MovieManager(const std::string& moviesFile,
                           const std::string& favoritesFile)
    : moviesFile(moviesFile), favoritesFile(favoritesFile) {
    try {
        loadMovies();
        loadFavorites();

        collectionManager = std::make_unique<CollectionManager>(&movies);
    } catch (const FileNotFoundException& e) {
        std::cout << "Warning: " << e.what() << "\n";
        std::cout << "Use option 8 to create sample movies file.\n";
        collectionManager = std::make_unique<CollectionManager>(&movies);
    }
}

MovieManager::~MovieManager() {
    if (collectionManager) {
        collectionManager->saveAll();
    }
}

void MovieManager::loadMovies() {
    movies.clear();
    std::ifstream file(moviesFile);

    if (!file.is_open()) {
        std::ofstream createFile(moviesFile);
        createFile.close();
        return;
    }

    std::string line;
    std::string fullLine;
    int lineNumber = 0;

    while (std::getline(file, line)) {
        lineNumber++;
        if (line.empty()) {
            if (!fullLine.empty()) {
                fullLine += " ";
            }
            continue;
        }
        
        if (fullLine.empty()) {
            fullLine = line;
        } else {
            fullLine += " " + line;
        }
        
        int pipeCount = std::count(fullLine.begin(), fullLine.end(), '|');
        if (pipeCount >= 10) {
            try {
                Movie movie = Movie::fromString(fullLine);
                if (movie.getId() != 0) {
                    if (movie.getPosterPath().empty() && movie.getId() > 0) {
                        std::string possiblePath1 = "posters/" + std::to_string(movie.getId()) + ".jpg";
                        std::string possiblePath2 = "posters/" + std::to_string(movie.getId()) + ".png";
                        movie.setPosterPath(possiblePath1);
                    }
                    movies.push_back(movie);
                    fullLine.clear();
                } else {
                    throw InvalidMovieDataException("Line " + std::to_string(lineNumber));
                }
            } catch (const std::exception& e) {
                fullLine.clear();
            }
        }
    }
    
    if (!fullLine.empty()) {
        try {
            Movie movie = Movie::fromString(fullLine);
            if (movie.getId() != 0) {
                if (movie.getPosterPath().empty() && movie.getId() > 0) {
                    std::string possiblePath1 = "posters/" + std::to_string(movie.getId()) + ".jpg";
                    std::string possiblePath2 = "posters/" + std::to_string(movie.getId()) + ".png";
                    movie.setPosterPath(possiblePath1);
                }
                movies.push_back(movie);
            }
        } catch (const std::exception& e) {
        }
    }
    
    if (collectionManager) {
        collectionManager->updateAllMoviesRef(&movies);
    }
}

void MovieManager::loadFavorites() {
    favoriteIds.clear();
    std::ifstream file(favoritesFile);

    if (!file.is_open()) {
        return;
    }

    int id;
    while (file >> id) {
        if (file.fail()) {
            throw InvalidMovieDataException("Invalid favorite ID in file");
        }
        favoriteIds.push_back(id);
    }
}

void MovieManager::saveFavorites() {
    std::ofstream file(favoritesFile);
    if (!file.is_open()) {
        throw FileNotFoundException(favoritesFile);
    }

    for (int id : favoriteIds) {
        file << id << "\n";
    }
}

void MovieManager::validateMovieId(int id) const {
    auto it = std::find_if(movies.begin(), movies.end(),
                           [id](const Movie& m) { return m.getId() == id; });
    if (it == movies.end()) {
        throw MovieNotFoundException(id);
    }
}

void MovieManager::searchByTitle(const std::string& title) const {
    if (title.empty()) {
        throw InvalidInputException("Title cannot be empty");
    }

    std::string searchTitle = title;
    std::transform(searchTitle.begin(), searchTitle.end(), searchTitle.begin(), ::tolower);

    bool found = false;
    for (const auto& movie : movies) {
        std::string movieTitle = movie.getTitle();
        std::transform(movieTitle.begin(), movieTitle.end(), movieTitle.begin(), ::tolower);

        if (movieTitle.find(searchTitle) != std::string::npos) {
            movie.print();
            found = true;
        }
    }

    if (!found) {
        throw MovieNotFoundException("title: " + title);
    }
}

void MovieManager::searchByGenre(const std::string& genre) const {
    if (genre.empty()) {
        throw InvalidInputException("Genre cannot be empty");
    }

    std::string searchGenre = genre;
    std::transform(searchGenre.begin(), searchGenre.end(), searchGenre.begin(), ::tolower);

    bool found = false;
    for (const auto& movie : movies) {
        if (movie.hasGenre(searchGenre)) {
            movie.print();
            found = true;
        }
    }

    if (!found) {
        throw MovieNotFoundException("genre: " + genre);
    }
}

void MovieManager::addToFavorites(int movieId) {
    validateMovieId(movieId);

    auto it = std::find(favoriteIds.begin(), favoriteIds.end(), movieId);
    if (it != favoriteIds.end()) {
        throw DuplicateFavoriteException(movieId);
    }

    favoriteIds.push_back(movieId);
    saveFavorites();
    std::cout << "Movie added to favorites!\n";
}

void MovieManager::removeFromFavorites(int movieId) {
    auto it = std::find(favoriteIds.begin(), favoriteIds.end(), movieId);
    if (it == favoriteIds.end()) {
        throw MovieNotFoundException(movieId);
    }

    favoriteIds.erase(it);
    saveFavorites();
    std::cout << "Movie removed from favorites!\n";
}

void MovieManager::showFavorites() const {
    if (favoriteIds.empty()) {
        std::cout << "No favorite movies yet.\n";
        return;
    }

    std::cout << "=== FAVORITE MOVIES ===\n";
    for (int id : favoriteIds) {
        auto it = std::find_if(movies.begin(), movies.end(),
                               [id](const Movie& m) { return m.getId() == id; });
        if (it != movies.end()) {
            it->print();
        }
    }
}

void MovieManager::showAllMovies() const {
    if (movies.empty()) {
        throw MovieException("No movies available. Create sample movies file first.");
    }

    std::cout << "=== ALL MOVIES ===\n";
    for (const auto& movie : movies) {
        movie.print();
    }
}

void MovieManager::sortByRating() {
    if (movies.empty()) {
        throw MovieException("No movies to sort");
    }

    std::sort(movies.begin(), movies.end(),
              [](const Movie& a, const Movie& b) { return a.getRating() > b.getRating(); });
    std::cout << "Movies sorted by rating!\n";
}

void MovieManager::showTopRated(int count) const {
    if (movies.empty()) {
        throw MovieException("No movies available");
    }

    if (count <= 0) {
        throw InvalidInputException("Count must be positive");
    }

    std::vector<Movie> sortedMovies = movies;
    std::sort(sortedMovies.begin(), sortedMovies.end(),
              [](const Movie& a, const Movie& b) { return a.getRating() > b.getRating(); });

    std::cout << "=== TOP " << count << " RATED MOVIES ===\n";
    for (int i = 0; i < std::min(count, (int)sortedMovies.size()); i++) {
        sortedMovies[i].print();
    }
}

void MovieManager::getMovieDetails(int id) const {
    validateMovieId(id);

    auto it = std::find_if(movies.begin(), movies.end(),
                           [id](const Movie& m) { return m.getId() == id; });
    if (it != movies.end()) {
        std::cout << "=== MOVIE DETAILS ===\n";
        it->print();
    }
}

void MovieManager::reloadMovies() {
    loadMovies();
}

void MovieManager::createSampleMoviesFile() {
    std::ofstream file(moviesFile);
    if (!file.is_open()) {
        throw FileNotFoundException(moviesFile);
    }

    file << "1|The Shawshank Redemption|9.2|1994|Drama|Frank Darabont|Two imprisoned men bond over a number of years, finding solace and eventual redemption through acts of common decency.|posters/The Shawshank Redemption.png|USA|Tim Robbins, Morgan Freeman|142\n";
    file << "2|The Godfather|9.2|1972|Crime;Drama|Francis Ford Coppola|The aging patriarch of an organized crime dynasty transfers control to his reluctant son.|posters/The Godfather.png|USA|Marlon Brando, Al Pacino|175\n";
    file << "3|The Dark Knight|9.0|2008|Action;Crime;Drama|Christopher Nolan|Batman faces the Joker, a criminal mastermind who seeks to undermine society.|posters/The Dark Knight.png|USA|Christian Bale, Heath Ledger|152\n";
    file << "4|Forrest Gump|8.8|1994|Drama;Romance|Robert Zemeckis|The story of a man with low IQ who accomplished great things in his life.|posters/Forrest Gump.png|USA|Tom Hanks, Robin Wright|142\n";
    file << "6|Inception|8.7|2010|Sci-Fi;Action;Thriller|Christopher Nolan|A thief who steals corporate secrets through dream-sharing technology is given a chance to have his criminal history erased.|posters/Inception.png|USA;UK|Leonardo DiCaprio, Marion Cotillard|148\n";
    file << "7|The Matrix|8.7|1999|Sci-Fi;Action|The Wachowskis|A computer hacker learns about the true nature of reality and his role in the war against its controllers.|posters/The Matrix.png|USA|Keanu Reeves, Laurence Fishburne|136\n";
    file << "8|Goodfellas|8.7|1990|Crime;Drama|Martin Scorsese|The story of Henry Hill and his life in the mob.|posters/Goodfellas.png|USA|Robert De Niro, Ray Liotta|146\n";
    file << "9|The Lord of the Rings: The Fellowship of the Ring|8.8|2001|Fantasy;Adventure|Peter Jackson|A meek Hobbit begins a quest to destroy the One Ring.|posters/The Lord of the Rings The Fellowship of the Ring.png|New Zealand|Elijah Wood, Ian McKellen|178\n";
    file << "10|Fight Club|8.8|1999|Drama;Thriller|David Fincher|An insomniac office worker forms an underground fight club as a form of therapy.|posters/Fight Club.png|USA|Brad Pitt, Edward Norton|139\n";
    file << "11|Interstellar|8.6|2014|Sci-Fi;Drama|Christopher Nolan|A team of explorers travel through a wormhole in space in an attempt to ensure humanity's survival.|posters/Interstellar.png|USA;UK|Matthew McConaughey, Anne Hathaway|169\n";
    file << "12|The Lord of the Rings: The Return of the King|8.7|2003|Fantasy;Adventure|Peter Jackson|Gandalf and Aragorn lead the World of Men against Sauron's army.|posters/The Lord of the Rings The Return of the King.png|New Zealand|Elijah Wood, Viggo Mortensen|201\n";
    file << "13|The Godfather Part II|9.0|1974|Crime;Drama|Francis Ford Coppola|The early life and career of Vito Corleone in 1920s New York is portrayed.|posters/The Godfather.png|USA|Al Pacino, Robert De Niro|200\n";
    file << "14|Schindler's List|8.9|1993|Biography;Drama|Steven Spielberg|In German-occupied Poland during World War II, industrialist Oskar Schindler becomes concerned for his Jewish workforce.|posters/Schindler's List.png|USA|Liam Neeson, Ralph Fiennes|195\n";
    file << "15|The Prestige|8.5|2006|Drama;Mystery|Christopher Nolan|Two stage magicians engage in competitive one-upmanship in an attempt to create the ultimate illusion.|posters/The Prestige.png|USA;UK|Hugh Jackman, Christian Bale|130\n";
    file << "16|Gladiator|8.5|2000|Action;Drama|Ridley Scott|A former Roman General sets out to exact vengeance against the corrupt emperor who murdered his family.|posters/Gladiator.png|USA;UK|Russell Crowe, Joaquin Phoenix|155\n";
    file << "17|The Green Mile|8.6|1999|Crime;Drama|Frank Darabont|The lives of guards on Death Row are affected by one of their charges: a black man accused of child murder.|posters/The Green Mile.png|USA|Tom Hanks, Michael Clarke Duncan|189\n";
    file << "18|The Pianist|8.5|2002|Biography;Drama|Roman Polanski|A Polish Jewish musician struggles to survive the destruction of the Warsaw ghetto.|posters/The Pianist.png|France;Poland|Adrien Brody, Thomas Kretschmann|150\n";
    file << "19|Whiplash|8.5|2014|Drama;Music|Damien Chazelle|A promising young drummer enrolls at a cut-throat music conservatory.|posters/Whiplash.png|USA|Miles Teller, J.K. Simmons|107\n";
    file << "20|Shutter Island|8.2|2010|Mystery;Thriller|Martin Scorsese|In 1954, a U.S. Marshal investigates the disappearance of a murderess who escaped from a hospital.|posters/Shutter Island.png|USA|Leonardo DiCaprio, Mark Ruffalo|138\n";
    file << "21|Django Unchained|8.4|2012|Drama;Western|Quentin Tarantino|With the help of a German bounty hunter, a freed slave sets out to rescue his wife.|posters/Django Unchained.png|USA|Jamie Foxx, Christoph Waltz|165\n";
    file << "22|Titanic|7.9|1997|Drama;Romance|James Cameron|A seventeen-year-old aristocrat falls in love with a kind but poor artist aboard the luxurious ship.|posters/Titanic.png|USA|Leonardo DiCaprio, Kate Winslet|194\n";
    file << "23|Avatar|7.8|2009|Action;Adventure|James Cameron|A paraplegic Marine dispatched to the moon Pandora on a unique mission becomes torn between following his orders.|posters/Avatar.png|USA|Sam Worthington, Zoe Saldana|162\n";
    file << "24|Catch Me If You Can|8.1|2002|Biography;Crime|Steven Spielberg|A seasoned FBI agent pursues Frank Abagnale Jr. who, before his 19th birthday, successfully forged millions of dollars' worth of checks.|posters/Catch Me If You Can.png|USA|Leonardo DiCaprio, Tom Hanks|141\n";
    file << "25|Snatch|8.2|2000|Comedy;Crime|Guy Ritchie|Unscrupulous boxing promoters, violent bookmakers, a Russian gangster, and a dog.|posters/Snatch.png|UK|Jason Statham, Brad Pitt|104\n";
    file << "27|The Fifth Element|7.6|1997|Action;Sci-Fi|Luc Besson|In the 23rd century, a taxi driver unwittingly becomes the central figure in the search for a legendary cosmic weapon.|posters/The Fifth Element.png|France|Bruce Willis, Milla Jovovich|126\n";
    file << "28|Iron Man|7.9|2008|Action;Adventure|Jon Favreau|After being held captive in an Afghan cave, billionaire engineer Tony Stark creates a unique weaponized suit of armor.|posters/Iron Man.png|USA|Robert Downey Jr., Gwyneth Paltrow|126\n";
    file << "29|Avengers Endgame|8.4|2019|Action;Adventure|Anthony Russo|After the devastating events of Infinity War, the universe is in ruins.|posters/Avengers Endgame.png|USA|Robert Downey Jr., Chris Evans|181\n";
    file << "30|Spider-Man|7.4|2002|Action;Adventure|Sam Raimi|When bitten by a genetically modified spider, a nerdy teen gains superhuman abilities.|posters/Spider-Man.png|USA|Tobey Maguire, Kirsten Dunst|121\n";
    file << "31|Terminator 2: Judgment Day|8.5|1991|Action;Sci-Fi|James Cameron|A cyborg, identical to the one who failed to kill Sarah Connor, must now protect her ten-year-old son.|posters/Terminator 2 Judgment Day.png|USA|Arnold Schwarzenegger, Linda Hamilton|137\n";
    file << "32|No Country for Old Men|8.2|2007|Crime;Thriller|Ethan Coen|Violence and mayhem ensue after a hunter stumbles upon a drug deal gone wrong.|posters/No Country for Old Men.png|USA|Tommy Lee Jones, Javier Bardem|122\n";
    file << "33|American History X|8.5|1998|Crime;Drama|Tony Kaye|A former neo-nazi skinhead tries to prevent his younger brother from going down the same wrong path.|posters/American History X.png|USA|Edward Norton, Edward Furlong|119\n";
    file << "34|Requiem for a Dream|8.3|2000|Drama|Darren Aronofsky|The drug-induced utopias of four Coney Island people are shattered when their addictions run deep.|posters/Requiem for a Dream.png|USA|Ellen Burstyn, Jared Leto|102\n";
    file << "35|A Beautiful Mind|8.2|2001|Biography;Drama|Ron Howard|After John Nash, a brilliant but asocial mathematician, accepts secret work in cryptography, his life takes a turn.|posters/A Beautiful Mind.png|USA|Russell Crowe, Jennifer Connelly|135\n";
    file << "36|The King's Speech|8.0|2010|Biography;Drama|Tom Hooper|The story of King George VI of the United Kingdom and his struggle to overcome his stammer.|posters/The King's Speech.png|UK|Colin Firth, Geoffrey Rush|118\n";
    file << "37|Slumdog Millionaire|8.0|2008|Drama|Danny Boyle|A Mumbai teenager who grew up in the slums becomes a contestant on the Indian version of Who Wants to Be a Millionaire?|posters/Slumdog Millionaire.png|UK;India|Dev Patel, Freida Pinto|120\n";
    file << "38|12 Years a Slave|8.1|2013|Biography;Drama|Steve McQueen|In the antebellum United States, Solomon Northup, a free black man from upstate New York, is abducted and sold into slavery.|posters/12 Years a Slave.png|USA|Chiwetel Ejiofor, Michael Fassbender|134\n";
    file << "39|Hacksaw Ridge|8.1|2016|Biography;Drama|Mel Gibson|World War II American Army Medic Desmond T. Doss refuses to kill people.|posters/Hacksaw Ridge.png|USA;Australia|Andrew Garfield, Teresa Palmer|139\n";
    file << "40|Million Dollar Baby|8.1|2004|Drama|Clint Eastwood|A determined woman works with a hardened boxing trainer to become a professional.|posters/Million Dollar Baby.png|USA|Clint Eastwood, Hilary Swank|132\n";
    file << "41|Green Book|8.2|2018|Biography;Comedy|Peter Farrelly|A working-class Italian-American bouncer becomes the driver of an African-American classical pianist.|posters/Green Book.png|USA|Viggo Mortensen, Mahershala Ali|130\n";
    file << "42|The Truman Show|8.1|1998|Comedy;Drama|Peter Weir|An insurance salesman discovers his whole life is actually a reality TV show.|posters/The Truman Show.png|USA|Jim Carrey, Laura Linney|103\n";
    file << "43|La La Land|8.0|2016|Comedy;Drama|Damien Chazelle|While navigating their careers in Los Angeles, a pianist and an actress fall in love.|posters/La La Land.png|USA|Ryan Gosling, Emma Stone|128\n";
    file << "44|The Social Network|7.7|2010|Biography;Drama|David Fincher|Harvard student Mark Zuckerberg creates the social networking site that would become known as Facebook.|posters/The Social Network.png|USA|Jesse Eisenberg, Andrew Garfield|120\n";
    file << "45|Oppenheimer|8.3|2023|Biography;Drama|Christopher Nolan|The story of American scientist J. Robert Oppenheimer and his role in the development of the atomic bomb.|posters/Oppenheimer.png|USA;UK|Cillian Murphy, Emily Blunt|180\n";
    file << "46|Blade Runner 2049|8.0|2017|Sci-Fi;Thriller|Denis Villeneuve|Young Blade Runner K's discovery of a long-buried secret leads him to track down former Blade Runner Rick Deckard.|posters/Blade Runner 2049.png|USA;UK|Ryan Gosling, Harrison Ford|164\n";
    file << "47|Dune Part Two|8.4|2024|Action;Adventure|Denis Villeneuve|Paul Atreides unites with Chani and the Fremen while seeking revenge against the conspirators who destroyed his family.|posters/Dune Part Two.png|USA|Timothée Chalamet, Zendaya|166\n";
    file << "48|Good Will Hunting|8.3|1997|Drama|Gus Van Sant|Will Hunting, a janitor at MIT, has a gift for mathematics.|posters/Good Will Hunting.png|USA|Matt Damon, Robin Williams|126\n";
    file << "49|Star Wars Episode V - The Empire Strikes Back|8.7|1980|Action;Adventure|Irvin Kershner|After the Rebels are brutally overpowered by the Empire on the ice planet Hoth, Luke Skywalker begins Jedi training.|posters/Star Wars Episode V - The Empire Strikes Back.png|USA|Mark Hamill, Harrison Ford|124\n";
    file << "50|Star Wars Episode I - The Phantom Menace|6.5|1999|Action;Adventure|George Lucas|Two Jedi escape a hostile blockade to find allies and come across a young boy who may bring balance to the Force.|posters/Star Wars Episode I - The Phantom Menace.png|USA|Liam Neeson, Ewan McGregor|136\n";
    file << "51|Guardians of the Galaxy|8.0|2014|Action;Adventure|James Gunn|A group of intergalactic criminals are forced to work together to stop a fanatical warrior.|posters/Guardians of the Galaxy.png|USA|Chris Pratt, Zoe Saldana|121\n";
    file << "52|Harry Potter and the Sorcerer's Stone|7.6|2001|Adventure;Family|Chris Columbus|An orphaned boy enrolls in a school of wizardry.|posters/Harry Potter and the Sorcerer's Stone.png|UK|Daniel Radcliffe, Rupert Grint|152\n";
    file << "53|Harry Potter and the Chamber of Secrets|7.4|2002|Adventure;Family|Chris Columbus|An ancient chamber is opened at Hogwarts, releasing a monster.|posters/Harry Potter and the Chamber of Secrets.png|UK|Daniel Radcliffe, Rupert Grint|161\n";
    file << "54|Home Alone|7.6|1990|Comedy;Family|Chris Columbus|An eight-year-old troublemaker must protect his house from a pair of burglars.|posters/Home Alone.png|USA|Macaulay Culkin, Joe Pesci|103\n";
    file << "55|Hachi: A Dog's Tale|8.1|2009|Drama;Family|Lasse Hallström|A college professor bonds with an abandoned dog he takes into his home.|posters/Hachi A Dog's Tale.png|USA;UK|Richard Gere, Joan Allen|93\n";
    file << "56|Wonder|7.9|2017|Drama;Family|Stephen Chbosky|Based on the New York Times bestseller, this tells the inspiring story of August Pullman.|posters/Wonder.png|USA|Jacob Tremblay, Julia Roberts|113\n";
    file << "57|The Blind Side|7.6|2009|Biography;Drama|John Lee Hancock|The story of Michael Oher, a homeless and traumatized boy who became an All-American football player.|posters/The Blind Side.png|USA|Sandra Bullock, Tim McGraw|129\n";
    file << "58|Meet Joe Black|7.2|1998|Drama;Fantasy|Martin Brest|Death, who takes the form of a young man, asks a media mogul to act as a guide.|posters/Meet Joe Black.png|USA|Brad Pitt, Anthony Hopkins|178\n";
    file << "59|The Gentlemen|7.8|2019|Action;Comedy|Guy Ritchie|A drug lord's attempt to sell his marijuana empire sets off a chain of blackmail and betrayal.|posters/The Gentlemen.png|UK;USA|Matthew McConaughey, Charlie Hunnam|113\n";
    file << "60|Leon|8.5|1994|Action;Crime;Drama|Luc Besson|12-year-old Mathilda is reluctantly taken in by Léon, a professional assassin.|posters/Leon.png|France|Jean Reno, Natalie Portman|110\n";
    file << "61|Knockin' on Heaven's Door|7.9|1997|Action;Comedy|Thomas Jahn|Two terminally ill men escape from a cancer ward and take a joyride through Germany.|posters/Knockin' on Heaven's Door.png|Germany|Til Schweiger, Jan Josef Liefers|87\n";
    file << "62|Pirates of the Caribbean: The Curse of the Black Pearl|8.0|2003|Action;Adventure|Gore Verbinski|Blacksmith Will Turner teams up with eccentric pirate Captain Jack Sparrow.|posters/Pirates of the Caribbean The Curse of the Black Pearl.png|USA|Johnny Depp, Orlando Bloom|143\n";
    file << "63|F1|8.0|2023|Documentary|Paul Greengrass|A documentary following the 2022 Formula 1 season.|posters/F1.png|UK|Lewis Hamilton, Max Verstappen|120\n";


    std::cout << "Sample movies file created with 61 movies!\n";
    loadMovies();
}

std::vector<Movie> MovieManager::getFavoriteMovies() const {
    std::vector<Movie> favorites;
    for (int id : favoriteIds) {
        auto it = std::find_if(movies.begin(), movies.end(), [id](const Movie& m) { return m.getId() == id; });
        if (it != movies.end()) {
            favorites.push_back(*it);
        }
    }
    return favorites;
}

bool MovieManager::isFavorite(int id) const {
    return std::find(favoriteIds.begin(), favoriteIds.end(), id) != favoriteIds.end();
}

std::vector<Movie> MovieManager::searchByTitleResults(const std::string& title) const {
    if (title.empty()) {
        throw InvalidInputException("Title cannot be empty");
    }
    std::string searchTitle = title;
    std::transform(searchTitle.begin(), searchTitle.end(), searchTitle.begin(), ::tolower);
    std::vector<Movie> results;
    for (const auto& movie : movies) {
        std::string movieTitle = movie.getTitle();
        std::transform(movieTitle.begin(), movieTitle.end(), movieTitle.begin(), ::tolower);
        if (movieTitle.find(searchTitle) != std::string::npos) {
            results.push_back(movie);
        }
    }
    return results;
}

std::vector<Movie> MovieManager::searchByGenreResults(const std::string& genre) const {
    if (genre.empty()) {
        throw InvalidInputException("Genre cannot be empty");
    }
    std::string searchGenre = genre;
    std::transform(searchGenre.begin(), searchGenre.end(), searchGenre.begin(), ::tolower);
    std::vector<Movie> results;
    for (const auto& movie : movies) {
        if (movie.hasGenre(searchGenre)) {
            results.push_back(movie);
        }
    }
    return results;
}

std::vector<Movie> MovieManager::topRatedResults(int count) const {
    if (movies.empty()) {
        throw MovieException("No movies available");
    }
    if (count <= 0) {
        throw InvalidInputException("Count must be positive");
    }
    std::vector<Movie> sortedMovies = movies;
    std::sort(sortedMovies.begin(), sortedMovies.end(),
              [](const Movie& a, const Movie& b) { return a.getRating() > b.getRating(); });
    if (count < static_cast<int>(sortedMovies.size())) {
        sortedMovies = std::vector<Movie>(sortedMovies.begin(), sortedMovies.begin() + count);
    }
    return sortedMovies;
}

const Movie* MovieManager::findMovieById(int id) const {
    auto it = std::find_if(movies.begin(), movies.end(), [id](const Movie& m) { return m.getId() == id; });
    if (it == movies.end()) {
        return nullptr;
    }
    return &(*it);
}

std::set<std::string> MovieManager::getAllGenres() const {
    std::set<std::string> genres;
    for (const auto& movie : movies) {
        const auto& movieGenres = movie.getGenres();
        for (const auto& genre : movieGenres) {
            genres.insert(genre);
        }
    }
    return genres;
}

MovieCollection* MovieManager::createCollection(const std::string& name) {
    if (!collectionManager) {
        throw MovieException("Collection manager not initialized");
    }
    return collectionManager->createCollection(name);
}

std::vector<std::string> MovieManager::getAllCollectionNames() const {
    if (!collectionManager) {
        return {};
    }
    return collectionManager->getAllCollectionNames();
}

const std::vector<Movie>& MovieManager::getAllMovies() const {
    return movies;
}

CollectionManager* MovieManager::getCollectionManager() {
    return collectionManager.get();
}

const CollectionManager* MovieManager::getCollectionManager() const {
    return collectionManager.get();
}

size_t MovieManager::getMoviesCount() const {
    return movies.size();
}

size_t MovieManager::getFavoritesCount() const {
    return favoriteIds.size();
}

void MovieManager::saveMovies() {
    std::ofstream file(moviesFile, std::ios::out);
    if (!file.is_open()) {
        throw FileNotFoundException(moviesFile);
    }
    
    for (const auto& movie : movies) {
        std::string line = movie.toString();
        file << line << "\n";
    }
    
    file.close();
}

void MovieManager::removeMovie(int movieId) {
    auto it = std::find_if(movies.begin(), movies.end(),
                          [movieId](const Movie& m) { return m.getId() == movieId; });
    
    if (it == movies.end()) {
        throw MovieNotFoundException(movieId);
    }
    
    auto favIt = std::find(favoriteIds.begin(), favoriteIds.end(), movieId);
    if (favIt != favoriteIds.end()) {
        favoriteIds.erase(favIt);
        saveFavorites();
    }
    
    if (collectionManager) {
        auto allCollections = getAllCollectionNames();
        for (const auto& name : allCollections) {
            MovieCollection* collection = collectionManager->getCollection(name);
            if (collection && collection->containsMovie(movieId)) {
                collection->removeMovie(movieId);
            }
        }
    }
    
    movies.erase(it);
    
    saveMovies();
    
    if (collectionManager) {
        collectionManager->updateAllMoviesRef(&movies);
    }
}

void MovieManager::addMovieToFile(const Movie& movie) {
    int newId = movie.getId();
    if (newId == 0) {
        newId = 1;
        for (const auto& m : movies) {
            if (m.getId() >= newId) {
                newId = m.getId() + 1;
            }
        }
    }
    
    Movie movieWithId = movie;
    movieWithId.setId(newId);
    
    movies.push_back(movieWithId);
    
    saveMovies();
    
    if (collectionManager) {
        collectionManager->updateAllMoviesRef(&movies);
    }
}