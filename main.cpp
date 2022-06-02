/*!
 * @brief This file contains the driver code of this application.
 */
#include "registrationRecord.h"
#include "config.h"
#include "eventDriver.h"

int main() {
    printWelcomeMessage();  // Welcome!
    startingTime = std::time(nullptr);

    // Instantiate our crucial data structures.
    Container container{numReg, numLoc};

    int choice;
    std::cout << std::endl;
    std::cout << BOLDCYAN << std::string(40, '-') << std::endl;
    std::cout << BOLDCYAN << "***\t1: " << RESET << CYAN << "Load registration records from file." << std::endl;
    std::cout << BOLDCYAN << "***\t0: " << RESET << CYAN << "Exit!" << std::endl;
    std::cout << std::string(40, '-') << std::endl;
    std::cout << GREEN << "Please enter your choice (0-1): " << RESET;
    scanIntRange(choice, 0, 1);
    if (0 == choice) {
        printBigText();
        return 0;
    }
    std::cout << YELLOW << "Loading local registries..." << RESET << std::endl;
    try {
        loadRecords(container);
    } catch (const IOError& error) {
        std::cerr << error.what() << std::endl;
    }
    std::cout << YELLOW << "Loading appointment preferences..." << RESET << std::endl;
    try {
        loadPreferences(container);
    } catch (const IOError& error) {
        std::cerr << error.what() << std::endl;
    }

    showPrompt();
    std::cout << GREEN << "Please enter your choice (0-13): " << RESET;
    while (true) {
        scanIntRange(choice, 0, 13);
        switch (choice) {
            case 1: {
                move12Hours(container);
                break;
            }
            case 2: {  // Using brackets here to avoid jumping bypassing initialization.
                int halfDays;
                std::cout << BLUE << "Please enter the length of time advance (in days, max=100):" << RESET
                          << std::endl;
                scanIntRange(halfDays, 1, 100);
                moveNDays(halfDays, container);
                break;
            }
            case 3: {
                try {
                    newRegistration(container);
                } catch (const IOError& error) {
                    std::cerr << error.what() << std::endl;
                }
                break;
            }
            case 4: {
                int id;
                std::cout << BLUE << "Please enter the ID of the record that you want to withdraw: " << RESET
                          << std::endl;
                scanIntRange(id, 1, std::numeric_limits<int>::max());
                withdrawRecord(id, container);
                break;
            }
            case 5: {
                int id;
                std::cout << BLUE << "Please enter the ID of the record that you want to recover: " << RESET
                          << std::endl;
                scanIntRange(id, 1, std::numeric_limits<int>::max());
                recoverRecord(id, container);
                break;
            }
            case 6: {
                int id;
                std::cout << BLUE << "Please enter the ID of the record that you want to update: " << RESET
                          << std::endl;
                scanIntRange(id, 1, std::numeric_limits<int>::max());
                int target_id;
                std::cout << BLUE << "Please enter the target profession category (between 1 and 8): " << RESET
                          << std::endl;
                scanIntRange(target_id, 1, 8);
                updateProfessionId(id, target_id, container);
                break;
            }
            case 7: {
                int id;
                std::cout << BLUE << "Please enter the ID of the record that you want to update: " << RESET
                          << std::endl;
                scanIntRange(id, 1, std::numeric_limits<int>::max());
                int target_id;
                std::cout << BLUE << "Please enter the target risk status (between 0 and 3): " << RESET
                          << std::endl;
                scanIntRange(target_id, 0, 3);
                updateRiskStatus(id, target_id, container);
                break;
            }
            case 8: {
                int id;
                time_t deadline;
                std::cout << BLUE << "Please enter the ID of the record that you want to update: " << RESET
                          << std::endl;
                scanIntRange(id, 1, std::numeric_limits<int>::max());
                std::cout << MAGENTA << "Please enter the treatment deadline (yyyy-mm-dd): " << std::endl;
                scanDate(deadline);
                addDeadline(id, deadline, container);
                break;
            }
            case 0:
                goto EXIT;
            case 10: {
                int id;
                std::cout << BLUE << "Please enter the ID of the database record that you wish to retrieve: " << RESET
                          << std::endl;
                scanIntRange(id, 1, std::numeric_limits<int>::max());
                printDBRecord(container, id);
                break;
            }
            case 11: {
                std::string name;
                std::cout << BLUE << "Please enter the username of the database record that you wish to retrieve: "
                          << RESET
                          << std::endl;
                std::cin.clear();
                std::cin.ignore(256, '\n');
                getline(std::cin, name);
                printDBRecord(container, name);
                break;
            }
            case 12: {
                int id;
                std::cout << BOLDYELLOW << "WARNING: THIS OPERATION IS IRREVERSIBLE!" << RESET << std::endl;
                std::cout << BLUE << "Please enter the ID of the database record that you wish to remove: " << RESET
                          << std::endl;
                scanIntRange(id, 1, std::numeric_limits<int>::max());
                removeDBRecord(container, id);
                break;
            }
            case 13: {
                std::string name;
                std::cout << BOLDYELLOW << "WARNING: THIS OPERATION IS IRREVERSIBLE!" << RESET << std::endl;
                std::cout << BLUE << "Please enter the username of the database record that you wish to remove: "
                          << RESET
                          << std::endl;
                std::cin.clear();
                std::cin.ignore(256, '\n');
                getline(std::cin, name);
                removeDBRecord(container, name);
                break;
            }
            case 9:
            default:
                showPrompt();
        }
        std::cout << GREEN << "Please enter your choice (0-13, enter 9 to re-display the prompt): " << RESET;
    }

    EXIT:
    printBigText();
    return 0;
}