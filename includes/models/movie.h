#ifndef BETA2_MOVIE_H
#define BETA2_MOVIE_H

#include <string>
#include <string_view>
#include <vector>
#include <ostream>
#include <istream>
#include <compare>

class Movie {
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
    Movie(int id, const std::string& title, double rating, int year,
          const std::vector<std::string>& genres, const std::string& director,
          const std::string& description, const std::string& posterPath,
          const std::string& country, const std::string& actors, int duration);

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
    void setPosterPath(std::string_view newPath);

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
