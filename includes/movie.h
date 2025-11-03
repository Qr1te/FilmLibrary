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


    int getId() const { return id; }
    const std::string& getTitle() const { return title; }
    double getRating() const { return rating; }
    int getYear() const { return year; }
    const std::vector<std::string>& getGenres() const { return genres; }
    std::string getGenreString() const;  // Возвращает жанры через запятую
    const std::string& getDirector() const { return director; }
    const std::string& getDescription() const { return description; }
    const std::string& getPosterPath() const { return posterPath; }
    const std::string& getCountry() const { return country; }
    const std::string& getActors() const { return actors; }
    int getDuration() const { return duration; }

    // Сеттеры
    void setId(int id) { this->id = id; }
    void setTitle(const std::string& title) { this->title = title; }
    void setRating(double rating) { this->rating = rating; }
    void setYear(int year) { this->year = year; }
    void setGenres(const std::vector<std::string>& genres) { this->genres = genres; }
    void addGenre(const std::string& genre);
    void setDirector(const std::string& director) { this->director = director; }
    void setDescription(const std::string& description) { this->description = description; }
    void setPosterPath(const std::string& path) { this->posterPath = path; }
    void setCountry(const std::string& country) { this->country = country; }
    void setActors(const std::string& actors) { this->actors = actors; }
    void setDuration(int duration) { this->duration = duration; }

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


#endif//BETA2_MOVIE_H
