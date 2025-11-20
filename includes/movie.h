#ifndef BETA2_MOVIE_H
#define BETA2_MOVIE_H


#include <string>
#include <string_view>
#include <vector>
#include <ostream>
#include <istream>
#include <compare>

class MovieParser {
public:
    static int parseId(const std::vector<std::string>& tokens);
    static double parseRating(const std::vector<std::string>& tokens);
    static int parseYear(const std::vector<std::string>& tokens);
    static std::vector<std::string> parseGenres(const std::vector<std::string>& tokens);
    static int parseDuration(const std::vector<std::string>& tokens);
};

class Movie {
public:
    struct Data {
        int id = 0;
        std::string title;
        double rating = 0.0;
        int year = 0;
        std::vector<std::string> genres;
        std::string director;
        std::string description;
        std::string posterPath;
        std::string country;
        std::string actors;
        int duration = 0;
    };

private:
    int id;
    std::string title;
    double rating;
    int year;
    std::vector<std::string> genres;
    std::string director;
    std::string description;
    std::string posterPath;
    std::string country;
    std::string actors;
    int duration;

public:
    explicit Movie(const Data& data);
    
    explicit Movie(int id = 0, const std::string& title = "", double rating = 0.0, int year = 0,
          const std::vector<std::string>& genres = {}, const std::string& director = "",
          const std::string& description = "", const std::string& posterPath = "",
          const std::string& country = "", const std::string& actors = "", int duration = 0);


    Movie(int id, const std::string& title, double rating, int year,
          const std::string& genre, const std::string& director,
          const std::string& description);


    int getId() const;
    const std::string& getTitle() const;
    double getRating() const;
    int getYear() const;
    const std::vector<std::string>& getGenres() const;
    std::string getGenreString() const;
    const std::string& getDirector() const;
    const std::string& getDescription() const;
    const std::string& getPosterPath() const;
    const std::string& getCountry() const;
    const std::string& getActors() const;
    int getDuration() const;

    void setId(int newId);
    void setTitle(std::string_view newTitle);
    void setRating(double newRating);
    void setYear(int newYear);
    void setGenres(const std::vector<std::string>& newGenres);
    void addGenre(std::string_view genre);
    void setDirector(std::string_view newDirector);
    void setDescription(std::string_view newDescription);
    void setPosterPath(std::string_view newPath);
    void setCountry(std::string_view newCountry);
    void setActors(std::string_view newActors);
    void setDuration(int newDuration);

    void print() const;
    std::string toString() const;
    static Movie fromString(const std::string& data);
    bool hasGenre(std::string_view genre) const;

private:
    bool operator==(const Movie& other) const;
    std::partial_ordering operator<=>(const Movie& other) const;
    
    friend std::ostream& operator<<(std::ostream& os, const Movie& movie) {
        os << movie.toString();
        return os;
    }
    
    friend std::istream& operator>>(std::istream& is, Movie& movie) {
        if (std::string line; std::getline(is, line)) {
            movie = Movie::fromString(line);
        }
        return is;
    }
};


#endif
