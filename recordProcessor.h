/*!
 * @brief This file contains the class definition of `Container`, which holds all essential data structures
 * of our program. It also contains some registration records processing functions.
 */
#ifndef CS225_SP22_C1_RECORDPROCESSOR_H_
#define CS225_SP22_C1_RECORDPROCESSOR_H_

#include "registrationRecord.h"
#include "centralizedQueue.h"
#include "centralizedQueue.cpp"
#include "BTree.h"
#include "BTree.cpp"
#include "BPlusTree.h"
#include "BPlusTree.cpp"
#include "databaseSchema.h"
#include "utilities.h"
#include "config.h"

/*!
 * @brief This class contains crucial data structures.
 */
class Container {
public:
    std::vector<Queue<RegistrationRecord>> localQueues{};
    CentralizedQueue<RegistrationRecord> centralizedQueue{};
    std::vector<RegistrationRecord> waitingList{};
    std::vector<RegistrationRecord> pendingList{};  // Used to store withdrawn records.
    std::vector<RegistrationRecord> appointmentList{};
    std::vector<RegistrationRecord> treatedList{};
    std::vector<std::pair<RegistrationRecord*, time_t>>
        deadlineTracker{};  // Pairs of pointers-to-record and deadlines.
    std::vector<std::vector<int>> preferences;  // Appointment location preferences for each local queue.
    std::vector<std::vector<bool>> availabilities;  // Availability of each time slot.
    BPlusTree<int, DBRecord> primaryDB;
    BTree<std::string, DBRecord> secondaryDB;

    // Constructor and destructor.
    Container() = delete;  // No-args constructor explicitly deleted.
    Container(const Container& container) = delete;  // The container cannot be moved or copied.
    Container& operator=(const Container& container) = delete;
    Container(Container&& container) = delete;
    Container& operator=(Container&& container) = delete;
    explicit Container(int num_reg, int num_loc);
    virtual ~Container() = default;
};

void waitingListProcessor(Container& container);
void newRegistration(Container& container);
void addDeadline(int id, time_t deadline, Container& container);
void withdrawRecord(int id, Container& container);
void recoverRecord(int id, Container& container);
void appointmentProcessor(Container& container);
void treatmentProcessor(Container& container);
void addDBRecord(Container& container, RegistrationRecord& record, int regID);
void updateDBRecord(Container& container, RegistrationRecord& record, int medical_status);
void updateDBRecord(Container& container, RegistrationRecord& record, int medical_status, int treatment);
void removeDBRecord(Container& container, int id);
void removeDBRecord(Container& container, const std::string& name);
void printDBRecord(Container& container, int id);
void printDBRecord(Container& container, const std::string& name);

#endif //CS225_SP22_C1_RECORDPROCESSOR_H_
