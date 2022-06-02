/*!
 * @brief This file contains implementations of our helper functions.
 */
#include "utilities.h"

/*!
 * @brief This method gets the current time in reality.
 * @return string of current time.
 */
[[maybe_unused]] std::string_view getCurrentTime() {
    time_t curr_time;
    tm* curr_tm;
    char timestamp[100];

    time(&curr_time);
    curr_tm = localtime(&curr_time); // Get current time (time_t*).

    strftime(timestamp, 50, "%F %T", curr_tm);  // Return the formatted timestamp.

    return timestamp;
}

/*!
 * @brief This function returns a integer within the specified range [start, end].
 * @param start is start of the range.
 * @param end is end of the range (inclusive).
 * @return the generated random integer.
 */
int generateRandomRangedInt(int start, int end) {
    std::random_device random_device;  // Generate random numbers from hardware.
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<int> distribution(start, end);
    return distribution(generator);
}

/*!
 * @brief This function scans an integer from user input. If conversion fails, it uses the specified default value.
 * @param x is reference to the containing integer.
 * @param defaultVal is default value.
 */
[[maybe_unused]] void scanIntDefault(int& x, int defaultVal) {
    std::cin.clear();
    std::cin >> x;
    if (std::cin.fail()) {
        std::cout << CYAN << "Using default value (" << defaultVal << ")." << RESET << std::endl;
        x = defaultVal;
    }
}

/*!
 * @brief This function scans an integer from the user input. If conversion fails, it asks the user to retry.
 * @param x is reference to the containing integer.
 */
void scanInt(int& x) {
    std::cin.clear();
    std::cin >> x;
    while (std::cin.fail()) {
        std::cout << RED << "Invalid input! Please enter an integer: " << RESET;
        std::cin.clear();
        std::cin.ignore(256, '\n');
        std::cin >> x;
    }
}

/*!
 * @brief This function scans an integer from the user input in a specified range. If input is invalid,
 * it asks the user to try again.
 * @param x is reference to the containing integer.
 * @param start is the minimum acceptable integer.
 * @param end is the maximum acceptable integer.
 */
void scanIntRange(int& x, int start, int end) {
    scanInt(x);
    while (x < start || x > end) {
        std::cout << RED << "Invalid input! Please try again: " << RESET << std::endl;
        scanInt(x);
    }
}

/*!
 * @brief This function converts the input string to its corresponding time_t.
 * NOTE: This function assumes the input string is a valid date!
 * @param s is reference to the containing string.
 * @return a string of time.
 */
time_t str2time(const std::string& s) {
    std::istringstream is{s};
    int y;
    int m;
    int d;
    char delimiter;
    is >> y >> delimiter >> m >> delimiter >> d;
    tm t{};
    t.tm_year = y - 1900;
    t.tm_mon = m - 1;
    t.tm_mday = d;
    t.tm_isdst = -1;

    return mktime(&t);  // We attempt to create a `tm` struct.
}

/*!
 * @brief This function checks if the input string is a valid date. If it is,
 * the function converts it to time_t and stores it into `dest`.
 * @param s is constant reference to the string.
 * @param dest is reference to the containing `time_t` variable.
 * @return true if date is valid, and false otherwise.
 */
bool isValidDate(const std::string& s, time_t& dest) {
    std::istringstream is{s};
    int y;
    int m;
    int d;
    char delimiter;
    if (is >> y >> delimiter >> m >> delimiter >> d) {
        tm t{};
        t.tm_year = y - 1900;
        t.tm_mon = m - 1;
        t.tm_mday = d;
        t.tm_isdst = -1;

        time_t when = mktime(&t);  // We attempt to create a `tm` struct.
        const tm* normalized_time = localtime(&when);

        if (normalized_time->tm_year == y - 1900 &&
            normalized_time->tm_mon == m - 1 &&
            normalized_time->tm_mday == d) {
            dest = when;
            return true;
        }
        return false;
    }
    return false;
}

/*!
 * @brief This function scans a date from the user input and puts it to a `time_t` variable.
 * It asks the user to enter again if input is not a valid date.
 * @param x is reference to the containing `time_t` variable.
 */
void scanDate(time_t& x) {
    std::string s;
    std::cin >> s;
    while (!isValidDate(s, x)) {
        std::cout << RED << "Invalid date! Please retry: " << RESET << std::endl;
        std::cin.clear();
        std::cin.ignore(256, '\n');
        std::cin >> s;
    }
}

/*!
 * @brief This function gets the RQRS current time.
 * @return the RQRS current time.
 */
time_t getRQRSCurrTime() {
    struct tm* tm = localtime(&startingTime);
    tm->tm_mday += halfDaysPassed / 2;
    tm->tm_hour += halfDaysPassed % 2 * 12;
    tm->tm_isdst = -1;        // don't know if DST is in effect, please determine
    return mktime(tm);
}

/*!
 * @brief This function gets the next day of the RQRS current time.
 * @return the RQRS next day.
 */
time_t getNextDay() {
    struct tm* tm = localtime(&startingTime);
    tm->tm_mday += halfDaysPassed / 2 + 1;
    tm->tm_hour += halfDaysPassed % 2 * 12;
    tm->tm_isdst = -1;        // don't know if DST is in effect, please determine
    return mktime(tm);
}

/*!
 * @brief This function contains a `time_t` variable to a string.
 * @param t is time to be converted.
 * @return a string in format "yyyy-mm-dd".
 */
std::string time2str(const time_t t) {
    char s[20];
    strftime(s, 20, "%Y-%m-%d", localtime(&t));
    return std::string{s};
}

/*!
 * @brief This function contains a `tm` struct to a string.
 * @param tm is pointer to the `tm` struct.
 * @return a string in format "yyyy-mm-dd".
 */
std::string tm2str(const tm* tm) {
    char s[100];
    strftime(s, 50, "%Y-%m-%d", tm);  // Return the formatted timestamp.
    return std::string{s};
}

/*!
 * @brief This method returns the `i`th item in the current row.
 * @param index is index of the item we want.
 * @return (std::string_view) the item.
 */
std::string_view CSVRow::operator[](size_t index) const {
    auto substr = std::string_view(&line[commas[index] + 1], commas[index + 1] - (commas[index] + 1));
    return substr;  // Note: std::move of the variable of the trivially-copyable type 'std::string_view' has no effect.
}

/*!
 * @brief Tis method returns number of fields in a row.
 * @return number of fields.
 */
size_t CSVRow::size() const {
    return commas.size() - 1;
}

/*!
 * @brief This function scans the next row of the csv file.
 * @param istream is reference to the istream object.
 */
void CSVRow::readNextRow(std::istream& istream) {
    getline(istream, line);
    std::string::size_type pos{};
    commas.clear();  // Clear vector for the new row.
    commas.emplace_back(-1);  // For the first element.
    while ((pos = line.find(',', pos)) != std::string::npos) {
        commas.emplace_back(pos++);
    }
    pos = line.size();
    commas.emplace_back(pos);
}

/*!
 * @brief This overloaded extraction operator reads the current row of the csv file.
 * @param istream is reference to the istream object.
 * @param data is reference to a CSVRow object.
 * @return reference to the modified istream object.
 */
std::istream& operator>>(std::istream& istream, CSVRow& data) {
    data.readNextRow(istream);
    return istream;
}

/*!
 * @brief This constructor initializes a CSVIterator.
 * @param istream is reference to the input istream object.
 */
CSVIterator::CSVIterator(std::istream& istream)
    : istream_(istream.good() ? &istream : nullptr) {
    ++(*this);
}

/*!
 * @brief This overloaded pre-increment operator increments the CSVIterator
 * and returns an iterator to the next row.
 * @return reference to the incremented CSVIterator.
 */
CSVIterator& CSVIterator::operator++() {
    if (istream_) {
        if (!(*istream_ >> row_)) {
            istream_ = nullptr;
        }
    }
    return *this;
}

/*!
 * @brief This overloaded pre-increment operator returns the iterator to the current row and increments
 * the CSVIterator.
 * @return the original CSVIterator.
 */
CSVIterator CSVIterator::operator++(int) {
    CSVIterator temp{*this};
    operator++();
    return temp;
}

/*!
 * @brief This overloaded dereference operator returns the current row.
 * @return reference to the const CSVRow object.
 */
CSVRow const& CSVIterator::operator*() const {
    return row_;
}

/*!
 * @brief This overloaded member access operator returns the address of the current row.
 * @return pointer to the const CSVRow object.
 */
CSVRow const* CSVIterator::operator->() const {
    return &row_;
}

/*!
 * @brief This overloaded equality operator checks whether the iterator is equal to another.
 * @param rhs is reference to another iterator.
 * @return true if equal, false otherwise.
 */
bool CSVIterator::operator==(const CSVIterator& rhs) {
    return this == &rhs || (this->istream_ == nullptr && rhs.istream_ == nullptr);
}

/*!
 * @brief This overloaded inequality operator checks whether the iterator is not equal to another.
 * @param rhs is reference to another iterator.
 * @return true if not equal, false otherwise.
 */
bool CSVIterator::operator!=(const CSVIterator& rhs) {
    return !(*this == rhs);
}

/*!
 * @brief This constructor initializes a CSVRange object.
 * @param istream is reference to the input istream object.
 */
CSVRange::CSVRange(std::istream& istream) : istream_(istream) {}

/*!
 * @brief This method gets the begin iterator of a CSVRange.
 * @return an iterator.
 */
CSVIterator CSVRange::begin() const {
    return CSVIterator{istream_};
}

/*!
 * @brief This method gets the end iterator of a CSVRange.
 * @return an iterator instantiated by default.
 */
CSVIterator CSVRange::end() {
    return CSVIterator{};
}

/*!
 * @brief This function prints the colorful RQRS big text to the console!
 */
void printBigText() {
    std::cout << std::endl;
    std::cout << BOLDGREEN
              << BOLDGREEN << R"(          _____                  )" << BOLDYELLOW << R"( _______                   )"
              << BOLDRED << R"(_____                    )" << BOLDBLUE << R"(_____          )" << "\n"
              << BOLDGREEN << R"(         /\    \               )" << BOLDYELLOW << R"(  /::\    \               )"
              << BOLDRED << R"(  /\    \                )" << BOLDBLUE << R"(  /\    \         )" << "\n"
              << BOLDGREEN << R"(        /::\    \             )" << BOLDYELLOW << R"(  /::::\    \               )"
              << BOLDRED << R"(/::\    \                )" << BOLDBLUE << R"(/::\    \        )" << "\n"
              << BOLDGREEN << R"(       /::::\    \            )" << BOLDYELLOW << R"( /::::::\    \)" << BOLDRED
              << R"(             /::::\    \              )" << BOLDBLUE << R"(/::::\    \       )" << "\n"
              << BOLDGREEN << R"(      /::::::\    \         )" << BOLDYELLOW << R"(  /::::::::\    \        )"
              << BOLDRED
              << R"(   /::::::\    \            )" << BOLDBLUE << R"(/::::::\    \     )" << "\n"
              << BOLDGREEN << R"(     /:::/\:::\    \         )" << BOLDYELLOW << R"(/:::/~~\:::\    \        )"
              << BOLDRED << R"( /:::/\:::\    \          )" << BOLDBLUE << R"(/:::/\:::\    \    )" << "\n"
              << BOLDGREEN << R"(    /:::/__\:::\    \       )" << BOLDYELLOW << R"(/:::/    \:::\    \       )"
              << BOLDRED << R"(/:::/__\:::\    \        )" << BOLDBLUE << R"(/:::/__\:::\    \    )" << "\n"
              << BOLDGREEN << R"(   /::::\   \:::\    \     )" << BOLDYELLOW << R"(/:::/    / \:::\    \  )" << BOLDRED
              << R"(   /::::\   \:::\    \       )" << BOLDBLUE << R"(\:::\   \:::\    \   )" << "\n"
              << BOLDGREEN << R"(  /::::::\   \:::\    \   )" << BOLDYELLOW << R"(/:::/____/   \:::\____\   )"
              << BOLDRED << R"(/::::::\   \:::\    \   )" << BOLDBLUE << R"( ___\:::\   \:::\    \  )" << "\n"
              << BOLDGREEN << R"( /:::/\:::\   \:::\____\ )" << BOLDYELLOW << R"(|:::|    |     |:::|    | )" << BOLDRED
              << R"(/:::/\:::\   \:::\____\  )" << BOLDBLUE << R"(/\   \:::\   \:::\    \)" << "\n"
              << BOLDGREEN << R"(/:::/  \:::\   \:::|    |)" << BOLDYELLOW << R"(|:::|____|     |:::|____|)" << BOLDRED
              << R"(/:::/  \:::\   \:::|    |)" << BOLDBLUE << R"(/::\   \:::\   \:::\____\)" << "\n"
              << BOLDGREEN << R"(\::/   |::::\  /:::|____|)" << BOLDYELLOW << R"( \:::\   _\___/:::/    /)" << BOLDRED
              << R"( \::/   |::::\  /:::|____|)" << BOLDBLUE << R"(\:::\   \:::\   \::/    /)" << "\n"
              << BOLDGREEN << R"( \/____|:::::\/:::/    /)" << BOLDYELLOW << R"(   \:::\ |::| /:::/    /)" << BOLDRED
              << R"(   \/____|:::::\/:::/    /)" << BOLDBLUE << R"(  \:::\   \:::\   \/____/)" << "\n"
              << BOLDGREEN << R"(       |:::::::::/    /)" << BOLDYELLOW << R"(     \:::\|::|/:::/    /)" << BOLDRED
              << R"(          |:::::::::/    /    )" << BOLDBLUE << R"(\:::\   \:::\    \     )" << "\n"
              << BOLDGREEN << R"(       |::|\::::/    /)" << BOLDYELLOW << R"(       \::::::::::/    /)" << BOLDRED
              << R"(           |::|\::::/    /)" << BOLDBLUE << R"(      \:::\   \:::\____\    )" << "\n"
              << BOLDGREEN << R"(       |::| \::/____/)" << BOLDYELLOW << R"(         \::::::::/    /)" << BOLDRED
              << R"(            |::| \::/____/)" << BOLDBLUE << R"(        \:::\  /:::/    /)" << "\n"
              << BOLDGREEN << R"(       |::|  ~|)" << BOLDYELLOW << R"(                \::::::/    /)" << BOLDRED
              << R"(             |::|  ~|)" << BOLDBLUE << R"(               \:::\/:::/    /)" << "\n"
              << BOLDGREEN << R"(       |::|   |)" << BOLDYELLOW << R"(                 \::::/____/)" << BOLDRED
              << R"(              |::|   |)" << BOLDBLUE << R"(                \::::::/    /)" << "\n"
              << BOLDGREEN << R"(       \::|   |)" << BOLDYELLOW << R"(                  |::|    |)" << BOLDRED
              << R"(               \::|   |)" << BOLDBLUE << R"(                 \::::/    /)" << "\n"
              << BOLDGREEN << R"(        \:|   |)" << BOLDYELLOW << R"(                  |::|____|)" << BOLDRED
              << R"(                \:|   |)" << BOLDBLUE << R"(                  \::/    /)" << "\n"
              << BOLDGREEN << R"(         \|___|)" << BOLDYELLOW << R"(                   ~~                       )"
              << BOLDRED << R"(\|___|)" << BOLDBLUE << R"(                   \/____/)" << "\n"
              << RESET << std::endl;
    std::cout << std::endl;
}

/*!
 * @brief This ugly function prints a beautiful (at least I think so) and colorful (well, literally yes)
 * welcome message to the console.
 */
void printWelcomeMessage() {
    printBigText();
    std::cout << BOLDMAGENTA << "WELCOME to our " << BOLDGREEN << "Registration " << BOLDYELLOW << "Queueing "
              << BOLDRED << "Reporting " << BOLDBLUE << "System" << BOLDMAGENTA << "!" << RESET << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
}

/*!
 * @brief This function shows the prompt of our program.
 */
void showPrompt() {
    std::cout << std::endl;
    std::cout << BOLDCYAN << std::string(40, '-') << RESET << std::endl;
    std::cout << BOLDCYAN << "***\t1: " << RESET << CYAN << "Move 12 hours forward." << std::endl;
    std::cout << BOLDCYAN << "***\t2: " << RESET << CYAN << "Move a few days forward."
              << std::endl;
    std::cout << BOLDCYAN << "***\t3: " << RESET << CYAN << "Create new registrations." << std::endl;
    std::cout << BOLDCYAN << "***\t4: " << RESET << CYAN << "Withdraw a registration." << std::endl;
    std::cout << BOLDCYAN << "***\t5: " << RESET << CYAN << "Recover a registration." << std::endl;
    std::cout << BOLDCYAN << "***\t6: " << RESET << CYAN << "Update the profession category for a record." << std::endl;
    std::cout << BOLDCYAN << "***\t7: " << RESET << CYAN << "Update the risk status for a record." << std::endl;
    std::cout << BOLDCYAN << "***\t8: " << RESET << CYAN << "Add a treatment deadline for a record." << std::endl;
    std::cout << BOLDCYAN << "***\t9: " << RESET << CYAN << "Print this prompt again." << std::endl;
    std::cout << BOLDCYAN << "***\t10: " << RESET << CYAN << "Retrieve a Database record by ID." << std::endl;
    std::cout << BOLDCYAN << "***\t11: " << RESET << CYAN << "Retrieve a Database record by NAME." << std::endl;
    std::cout << BOLDCYAN << "***\t12: " << RESET << CYAN
              << "Remove a Database record by ID." << std::endl;
    std::cout << BOLDCYAN << "***\t13: " << RESET << CYAN
              << "Remove a Database record by NAME." << std::endl;
    std::cout << BOLDCYAN << "***\t0: " << RESET << CYAN << "Exit!" << std::endl;
    std::cout << BOLDCYAN << std::string(40, '-') << RESET << std::endl;
}
