/*!
 * @brief This file contains some color codes and global variables (some of them are constant expressions).
 */
#ifndef CS225_SP22_C1_CONFIG_H_
#define CS225_SP22_C1_CONFIG_H_

#define DEBUG 0

// Terminal color codes. Only tested in Linux and macOS.
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

constexpr inline int numReg{5};  // Number of local registries.
constexpr inline int numLoc{5};  // Number of appointment locations.
constexpr inline int numSlot{6};  // Number of available time slots for each day.
constexpr inline int forwardWindowSize{5};  // Number of registries to forward per 12 hours.
inline int halfDaysPassed{};  // Number of days passed.
inline time_t startingTime;  // System starting time.

#endif //CS225_SP22_C1_CONFIG_H_
