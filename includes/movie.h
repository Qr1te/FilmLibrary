#ifndef BETA2_MOVIE_H
#define BETA2_MOVIE_H


#include <string>
#include <vector>
#include <ostream>
#include <istream>

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
    Movie(int id = 0, const std::string& title = "", double rating = 0.0, int year = 0,
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

    void setId(int id);
    void setTitle(const std::string& title);
    void setRating(double rating);
    void setYear(int year);
    void setGenres(const std::vector<std::string>& genres);
    void addGenre(const std::string& genre);
    void setDirector(const std::string& director);
    void setDescription(const std::string& description);
    void setPosterPath(const std::string& path);
    void setCountry(const std::string& country);
    void setActors(const std::string& actors);
    void setDuration(int duration);

    void print() const;
    std::string toString() const;
    static Movie fromString(const std::string& data);
    bool hasGenre(const std::string& genre) const;

    bool operator==(const Movie& other) const;
    bool operator!=(const Movie& other) const;
    bool operator<(const Movie& other) const;
    bool operator>(const Movie& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Movie& movie);
    friend std::istream& operator>>(std::istream& is, Movie& movie);
};


#endif
