/*!
 * @brief This file contains general purpose helper functions, including IO components.
 */
#ifndef CS225_SP22_C1_UTILITIES_H_
#define CS225_SP22_C1_UTILITIES_H_

#include <stdexcept>
#include <iostream>
#include <random>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <iterator>
#include <utility>
#include "config.h"

// General-purpose helper functions.
int generateRandomRangedInt(int start, int end);
[[maybe_unused]] std::string_view getCurrentTime();
void scanInt(int& x);
[[maybe_unused]] void scanIntDefault(int& x, int defaultVal);
void scanIntRange(int& x, int start, int end);
bool isValidDate(const std::string& s, time_t& dest);
void scanDate(time_t& x);
time_t getRQRSCurrTime();
time_t getNextDay();
time_t str2time(const std::string& s);
std::string time2str(time_t t);
std::string tm2str(const tm* tm);
void printBigText();
void printWelcomeMessage();
void showPrompt();

/*!
 * @brief This class is created for development purposes.
 */
class [[maybe_unused]] NotImplementedException : public std::logic_error {
public:
    NotImplementedException() : std::logic_error("Function not implemented!") {}
};

/*!
 * @brief This class defines a customized exception.
 */
class IOError : public std::exception {
public:
    IOError() noexcept = default;
    ~IOError() override = default;
    [[nodiscard]] const char* what() const noexcept override {
        return "Failed to read/write to file!";
    }
};

// IOs.
/*!
 * @brief The following three classes are heavily inspired by
 * <A HREF="https://stackoverflow.com/a/1120224">Martin York</A>. This class
 * parses a @em csv file and tokenize them.
 */
class CSVRow {
private:
    std::string line;  // Stores the current line.
    std::vector<int> commas{};  // Vector holding indices of commas.

public:
    std::string_view operator[](size_t index) const;  // C++17!
    [[nodiscard]] size_t size() const;  // Number of tokens of the current row.
    void readNextRow(std::istream& istream);
    friend std::istream& operator>>(std::istream& istream, CSVRow& data);  // Overloaded extraction operator.
};

/*!
 * @brief This class defines a custom iterator for our CSV parser.
 */
class CSVIterator {
public:
    // Defining
    using value_type = CSVRow;
    using differece_type = size_t;
    using pointer = CSVRow*;
    using reference = CSVRow&;
    using iterator_category = std::input_iterator_tag;  // For reading forward.

private:
    std::istream* istream_{nullptr};
    CSVRow row_;

public:
    CSVIterator() = default;
    explicit CSVIterator(std::istream& istream);

    CSVIterator& operator++();
    CSVIterator operator++(int);
    CSVRow const& operator*() const;
    CSVRow const* operator->() const;
    bool operator==(CSVIterator const& rhs);
    bool operator!=(CSVIterator const& rhs);
};

/*!
 * @brief This class provides necessary methods to enable <em>range-based for loop</em>
 * for our CSV parser.
 */
class CSVRange {
private:
    std::istream& istream_;

public:
    explicit CSVRange(std::istream& istream);
    [[nodiscard]] CSVIterator begin() const;
    static CSVIterator end();
};

#endif //CS225_SP22_C1_UTILITIES_H_
