/*!
 * @brief This file contains function needed to change and monitor the system time
 * and call record processing functions to perform appropriate operations.
 */
#include "eventDriver.h"

/*!
 * @brief This function moves our time one day forward and calls record processing functions.
 * @param container is the crucial data structure.
 */
void move12Hours(Container& container) {
    halfDaysPassed++;
    eventTrigger(container);
}

/*!
 * @brief This function moves our time N days forward and calls record processing functions
 * once per 12 hours.
 * @param container is the crucial data structure.
 */
void moveNDays(int numberOfDays, Container& container) {
    for (int i = 0; i < numberOfDays; ++i) {
        move12Hours(container);
        move12Hours(container);
    }
}

/*!
 * @brief This function checks the current system time and performs corresponding operations.
 * @param container is the crucial data structure.
 */
void eventTrigger(Container& container) {
    waitingListProcessor(container);
    forwardRegistrationRecords(container);
    appointmentProcessor(container);
    if (0 == halfDaysPassed % 2) {
        treatmentProcessor(container);
    }
    if (0 == halfDaysPassed % 14) {
        std::cout << BOLDYELLOW << std::string(40, '-') << std::endl;
        std::cout << BOLDYELLOW << "New weekly report available!" << std::endl;
        std::cout << BOLDYELLOW << std::string(40, '-') << RESET << std::endl;
        int order{};
        std::cout << BLUE << "What order of reporting lists do you want? (1: by name, "
                             "2: by profession category, 3: by age group)" << RESET << std::endl;
        scanIntRange(order, 1, 3);
        generateWeeklyReports(order, container);
        showPrompt();
    }
    if (0 == halfDaysPassed % 60) {
        std::cout << BOLDYELLOW << std::string(40, '-') << std::endl;
        std::cout << BOLDYELLOW << "New monthly report available!" << std::endl;
        std::cout << BOLDYELLOW << std::string(40, '-') << std::endl;
        generateMonthlyReports(container);
        showPrompt();
    }
}

/*!
 * @brief This function reads the input @em csv files and creates new @em registrationRecord
 * objects and store them in random local queues or the waiting list basing on their risk status.
 * @param container is the crucial data structure.
 */
void loadRecords(Container& container) {
    for (int i = 0; i < numReg; ++i) {
        std::string path{"data/reg_" + std::to_string(i + 1) + ".csv"};
        std::ifstream file{path};  // Destructor closes the file automatically.
        if (!file.is_open()) {  // Check if file opened successfully.
            std::cerr << "Failed to open file " << path << "!" << std::endl;
            throw IOError();
        }
        Queue<RegistrationRecord> temp;
        for (const auto& row : CSVRange{file}) {
            int risk{std::stoi(std::string(row[8]))};
            RegistrationRecord record{row};
            addDBRecord(container, record, 0);
            if (0 == risk || 1 == risk) {
                temp.push(record);
            } else {
                container.waitingList.push_back(record);  // Add median/high risk patients to the waiting list.
            }
        }
        container.localQueues.emplace_back(temp);
    }
    std::cout << BOLDGREEN << "Registration files successfully parsed." << RESET << std::endl;
}

/*!
 * @brief This function loads preference information from a data file and writes them into
 * our crucial data structure.
 * @param container is the crucial data structure.
 */
void loadPreferences(Container& container) {
    std::string path{"data/location_preferences.csv"};
    std::ifstream file{path};
    if (!file.is_open()) {
        std::cerr << "Failed to open file " << path << "!" << std::endl;
        throw IOError();
    }
    for (const auto& row : CSVRange{file}) {
        int queueID{std::stoi(std::string(row[0]))};
        for (int i = 1; i <= numLoc; ++i) {
            container.preferences[queueID - 1][i - 1] = std::stoi(std::string(row[i]));
        }
    }
    std::cout << BOLDGREEN << "Appointment location preferences successfully parsed." << RESET << std::endl;
}

/*!
 * @brief This function checks whether the local queues and the waiting list are all empty.
 * @param container is the crucial data structure.
 * @return true if all of them are empty, false otherwise.
 */
bool emptyQueueAndList(Container& container) {
    bool _empty{true};
    for (const auto& queue : container.localQueues) {
        _empty &= queue.empty();
    }
    _empty &= container.waitingList.empty();
    return _empty;
}

/*!
 * @brief This function extracts records from the local queues or the waiting list (when the random local queue
 * is empty) and push them onto the centralized queue.
 * @param container is the crucial data structure.
 */
void forwardRegistrationRecords(Container& container) {
    if (emptyQueueAndList(container)) {
        std::cout << BOLDYELLOW << "No records left in local queues or the waiting list. Please create a "
                  << "new registration." << RESET << std::endl;
        return;
    }
    for (int i = 0; i < forwardWindowSize; ++i) {
        int regID{generateRandomRangedInt(0, numReg - 1)};  // Randomly pick a local registry to place the record.
        if (!container.localQueues[regID].empty()) {  // If local queue is not empty, forward its first record.
            container.centralizedQueue.push(container.localQueues[regID].front());
            container.localQueues[regID].pop();  // Remove this record from the queue.
        } else if (!container.waitingList.empty()) {  // Forward the first element in the waiting list if local queue is empty.
            container.centralizedQueue.push(container.waitingList.front());
            std::cout << BOLDYELLOW << "Record (ID " << container.waitingList.front().GetId()
                      << ") has been forward to the centralized queue!" << std::endl;
            container.waitingList.erase(container.waitingList.begin());  // Remove the element from the waiting list.
        } else {
            if (emptyQueueAndList(container)) {
                std::cout << RESET << BOLDGREEN << i << " records have been forwarded to the centralized queue!"
                          << std::endl;
                std::cout << BOLDYELLOW << "No records left in local queues or the waiting list. Please create a "
                          << BOLDGREEN
                          << "new registration." << RESET << std::endl;
                break;
            }
            i--;  // Pick another record.
        }
    }
    std::cout << BOLDGREEN << forwardWindowSize << " records have been forwarded to the centralized queue!" << RESET
              << std::endl;
}

/*!
 * @brief This function searches for the specified record in RQRS and removes it from wherever it was and puts it in
 * the pending list.
 * @param id is id of record to be withdrawn.
 * @param container is the crucial data structure.
 */
void withdrawRecord(int id, Container& container) {
    // Search the pending list and make sure the record has not been withdrawn before.
    if (!container.pendingList.empty()) {
        auto pending_iter = std::find_if(container.pendingList.begin(), container.pendingList.end(),
                                         [&id](const RegistrationRecord& record) {
                                             return record.GetId() == id;
                                         });
        if (pending_iter != container.pendingList.end()) {  // Record found in withdrawal list.
            std::cout << BOLDRED << "Registration record (ID " << id << ") has already been withdrawn!" << RESET
                      << std::endl;
            return;
        }
    }
    bool found{false};
    // First find in the local queues.
    for (auto& queue : container.localQueues) {
        if (queue.empty()) {
            continue;  // Skip current queue.
        }
        auto queue_iter =
            queue.find_if(found, [&id](const RegistrationRecord& record) {
                return record.GetId() == id;  // `&` means "capture by reference."
            });
        if (found) {  // Found in local queue!
            auto& record{*queue_iter};  // Get a reference of the object.
            container.pendingList.push_back(std::move(record));  // It calls the move constructor.
            updateDBRecord(container, record, 3);
            queue.erase(queue_iter);  // Remove the record from the local queue.
            std::cout << BOLDGREEN << "Registration record (ID " << id
                      << ") found in a local queue has been successfully withdrawn!" << RESET
                      << std::endl;
            return;  // Done withdrawal.
        }
    }
    // Then look at the waiting list.
    if (!container.waitingList.empty()) {
        auto waiting_iter =
            std::find_if(container.waitingList.begin(),
                         container.waitingList.end(),
                         [&id](const RegistrationRecord& record) {
                             return record.GetId() == id;  // `&` means "capture by reference."
                         });
        if (waiting_iter != container.waitingList.end()) {  // Found in waiting list!
            auto& record{*waiting_iter};  // Get the reference of the object.
            container.pendingList.push_back(std::move(record));  // It calls the move constructor.
            container.waitingList.erase(waiting_iter);  // Remove the record from the waiting list.
            updateDBRecord(container, record, 3);
            std::cout << BOLDGREEN << "Registration record (ID " << id
                      << ") found in the waiting list has been successfully withdrawn!" << RESET
                      << std::endl;
            return;  // Done withdrawal.
        }
    }
    // Then look at the appointment list.
    if (!container.appointmentList.empty()) {
        auto app_iter =
            std::find_if(container.appointmentList.begin(),
                         container.appointmentList.end(),
                         [&id](const auto& record) {
                             return record.GetId() == id;  // `&` means "capture by reference."
                         });
        if (app_iter != container.appointmentList.end()) {  // Found in waiting list!
            auto& record{*app_iter};  // Get the reference of the object.
            container.pendingList.push_back(std::move(record));  // It calls the move constructor.
            updateDBRecord(container, record, 3);
            container.appointmentList.erase(app_iter);  // Remove the record from the waiting list.
            std::cout << BOLDGREEN << "Registration record (ID " << id
                      << ") found in the appointment list has been successfully withdrawn!" << RESET
                      << std::endl;
            return;  // Done withdrawal.
        }
    }
    // Finally, look at the centralized queue.
    if (!container.centralizedQueue.empty()) {
        auto centralized_iter =
            container.centralizedQueue.find_if(found, [&id](const auto& pair) {
                return pair.first.GetId() == id;  // `&` means "capture by reference."
            });
        if (found) {  // Found in the centralized queue!
            auto& record{centralized_iter->first};  // Get a reference of the object.
            auto temp{record};  // Make a copy.
            container.pendingList.push_back(record);  // It calls the copy constructor.
            updateDBRecord(container, temp, 3);
            temp.SetProfessionId(-1);  // This dummy object can always be the root.
            container.centralizedQueue.decreaseKey(centralized_iter->second,
                                                   std::move(temp));  // Make the modified record the root.
            container.centralizedQueue.pop();  // Remove the record from the centralized queue.
            std::cout << BOLDGREEN << "Registration record (ID " << id
                      << ") found in the centralized queue has been successfully withdrawn!" << RESET
                      << std::endl;
            return;  // Done withdrawal.
        }
    }
    std::cout << BOLDRED << "Registration record (ID " << id << ") not found in RQRS or has been cured!" << RESET
              << std::endl;
}

/*!
 * @brief This function recovers the queueing state for a specified record by pushing it onto a random local queue.
 * @param id is id of the record to be recovered.
 * @param container is the crucial data structure.
 */
void recoverRecord(int id, Container& container) {
    auto pending_iter = std::find_if(container.pendingList.begin(), container.pendingList.end(),
                                     [&id](const RegistrationRecord& record) {
                                         return record.GetId() == id;
                                     });
    if (pending_iter == container.pendingList.end()) {  // Record not found in withdrawal list.
        std::cout << BOLDRED << "No withdrawal recorded for record (ID " << id << ")!" << RESET << std::endl;
        return;
    }
    auto& record{*pending_iter};
    if (0 == record.GetRiskStatus() || 1 == record.GetRiskStatus()) {
        record.applyPenalty();  // Add additional two weeks waiting time for risk status 0/1.
    }
    container.waitingList.push_back(std::move(record));  // Move constructor called here.
    updateDBRecord(container, record, 0);
    container.pendingList.erase(pending_iter);  // Remove record from pending list.
    std::cout << BOLDGREEN << "Registration record (ID " << id
              << ") has been recovered!" << RESET
              << std::endl;
    std::cout << BOLDYELLOW << "A 14-day penalty has been applied." << RESET
              << std::endl;
}

/*!
 * @brief This function updates the profession id for a specified record and updates its container @em if
 * the information update increases the priority of this record.
 * @param id is record to be updated.
 * @param targetID is the profession id desired.
 * @param container is the crucial data structure.
 */
void updateProfessionId(int id, int targetID, Container& container) {
    bool found{false};
    // First find in the local queues.
    for (auto& queue : container.localQueues) {
        auto queue_iter =
            queue.find_if(found, [&id](const RegistrationRecord& record) {
                return record.GetId() == id;  // `&` means "capture by reference."
            });
        if (found) {  // Found in local queue!
            auto& record{*queue_iter};  // Get the reference of the object.
            if (record.GetProfessionId() <= targetID) {
                std::cout << BOLDYELLOW << "Attributes not updated since the request does not increase the priority."
                          << std::endl;
                return;  // No update applied.
            }
            record.SetProfessionId(targetID);
            std::cout << BOLDGREEN << "Registration record (ID " << id
                      << ") found in a local queue has been successfully updated with a new profession category!"
                      << RESET << std::endl;
            return;  // Done update.
        }
    }
    // Then look at the centralized queue.
    auto centralized_iter =
        container.centralizedQueue.find_if(found, [&id](const auto& keyPair) {
            return keyPair.first.GetId() == id;  // `&` means "capture by reference."
        });
    if (found) {  // Found in the centralized queue!
        auto record{centralized_iter->first};  // Get the reference of the object.
        if (record.GetProfessionId() <= targetID) {
            std::cout << BOLDYELLOW << "Attribute not updated since the request does not increase the priority."
                      << std::endl;
            return;  // No update applied.
        }
        auto temp{record};  // Make a copy to update the property.
        temp.SetProfessionId(targetID);   // Update the attribute.
        container.centralizedQueue.decreaseKey(centralized_iter->second,
                                               std::move(temp));  // Maintain the heap property.
        std::cout << BOLDGREEN << "Registration record (ID " << id
                  << ") found in the centralized queue has been successfully updated with a new profession category!"
                  << RESET << std::endl;
        return;  // Done update.
    }
    // Then look at the waiting list.
    auto waiting_iter =
        std::find_if(container.waitingList.begin(),
                     container.waitingList.end(),
                     [&id](const RegistrationRecord& record) {
                         return record.GetId() == id;  // `&` means "capture by reference."
                     });
    if (waiting_iter != container.waitingList.end()) {  // Found in waiting list!
        auto record{*waiting_iter};  // Get the reference of the object.
        if (record.GetProfessionId() <= targetID) {
            std::cout << BOLDYELLOW << "Attributes not updated since the request does not increase the priority."
                      << std::endl;
            return;  // No update applied.
        }
        record.SetProfessionId(targetID);
        std::cout << BOLDGREEN << "Registration record (ID " << id
                  << ") found in the waiting list has been successfully updated with a new profession category!"
                  << RESET << std::endl;
        return;  // Done update.
    }
    std::cout << BOLDRED << "Registration record (ID " << id << ") not found in RQRS!"
              << RESET << std::endl;
}

/*!
 * @brief This function updates the risk status for a specified record and updates its container @em if
 * the information update increases the priority of this record.
 * @param id is record to be updated.
 * @param targetID is the profession id desired.
 * @param container is the crucial data structure.
 */
void updateRiskStatus(int id, int targetID, Container& container) {
    // First look at the waiting list.
    auto waiting_iter =
        std::find_if(container.waitingList.begin(),
                     container.waitingList.end(),
                     [&id](const RegistrationRecord& record) {
                         return record.GetId() == id;  // `&` means "capture by reference."
                     });
    if (waiting_iter != container.waitingList.end()) {  // Found in waiting list!
        auto& record{*waiting_iter};  // Get the reference of the object.
        if (record.GetRiskStatus() <= targetID) {
            std::cout << BOLDYELLOW << "Attributes not updated since the request does not increase the priority."
                      << std::endl;
            return;  // No update applied.
        }
        record.SetRiskStatus(targetID);
        if (targetID == 3) {
            record.SetExtension(60);
        } else {
            record.SetExtension(0);
            container.localQueues[generateRandomRangedInt(0, numReg - 1)].emplace(std::move(record));
            updateDBRecord(container, record, 0);
            container.waitingList.erase(waiting_iter);  // Remove the current record from the waiting list.
        }
        std::cout << BOLDGREEN << "Registration record (ID " << id
                  << ") found in the waiting list has been successfully updated with a new risk status!"
                  << RESET << std::endl;
        return;  // Done update.
    }
    std::cout << BOLDYELLOW << "Registration record (ID " << id
              << ") does not exist in RQRS or operation has no effects!"
              << RESET << std::endl;
}

/*!
 * @brief This function produces the weekly reports, including people treated, people with appointments,
 * and people waiting for appointments.
 * @param order 1 -> order by name, 2 -> order by profession category, 3 -> order by age group.
 * @param container is the crucial data structure.
 * @sideeffects It prints the report to the console.
 */
void generateWeeklyReports(int order, Container& container) {
    std::ofstream file{"data/report.txt", std::ios_base::app};
    std::map<int, std::function<bool(const RegistrationRecord*, const RegistrationRecord*)>> comp_map{
        {1, [](const auto lhs, const auto rhs) { return lhs->GetName() < rhs->GetName(); }},
        {2, [](const auto lhs, const auto rhs) { return lhs->GetProfessionId() < rhs->GetProfessionId(); }},
        {3, [](const auto lhs, const auto rhs) { return lhs->GetAgeId() < rhs->GetAgeId(); }}
    };  // Map of comparators.
    std::cout << std::endl;
    file << "\n";
    std::cout << BOLDBLUE << std::string(62, '-') << "  *** People Treated ***  "
              << std::string(62, '-') << RESET << std::endl;
    file << std::string(62, '-') << "  *** People Treated ***  "
         << std::string(62, '-') << std::endl;
    std::vector<const RegistrationRecord*> treat_temp;
    for (const auto& record : container.treatedList) {
        treat_temp.push_back(&record);  // Get pointers to treated records.
    }
    std::sort(treat_temp.begin(), treat_temp.end(), comp_map[order]);
    for (const auto* const record_ptr : treat_temp) {
        std::cout << BOLDMAGENTA << *record_ptr << RESET << std::endl;  // Output treated records.
        file << *record_ptr << std::endl;
    }
    std::cout << BOLDBLUE << std::string(57, '-') << "  *** People with Appointments ***  "
              << std::string(57, '-') << RESET << std::endl;
    file << std::string(57, '-') << "  *** People with Appointments ***  "
         << std::string(57, '-') << std::endl;
    std::vector<const RegistrationRecord*> appoint_temp;
    for (const auto& record_ref : container.appointmentList) {
        appoint_temp.push_back(&record_ref);  // Get pointers to records with appointments.
    }
    std::sort(appoint_temp.begin(), appoint_temp.end(), comp_map[order]);
    for (const auto* const record_ptr : appoint_temp) {
        std::cout << BOLDMAGENTA << *record_ptr << RESET << std::endl;  // Output records with appointments.
        file << *record_ptr << std::endl;
    }
    std::cout << BOLDBLUE << std::string(61, '-') << "  *** People Queueing ***  "
              << std::string(62, '-') << RESET << std::endl;
    file << std::string(61, '-') << "  *** People Queueing ***  "
         << std::string(62, '-') << std::endl;
    std::vector<const RegistrationRecord*> queue_rec{};
    for (const auto& queue : container.localQueues) {
        auto queue_temp = queue.get_ptrs();
        queue_rec.insert(queue_rec.end(), queue_temp.begin(), queue_temp.end());
    }
    auto cent_temp = container.centralizedQueue.get_ptrs();
    queue_rec.insert(queue_rec.end(), cent_temp.begin(), cent_temp.end());
    std::sort(queue_rec.begin(), queue_rec.end(), comp_map[order]);
    for (const auto* const record_ptr : queue_rec) {
        std::cout << BOLDMAGENTA << *record_ptr << RESET << std::endl;
        file << *record_ptr << std::endl;
    }
    std::cout << BOLDBLUE << std::string(120, '-') << RESET << std::endl;
    file << std::string(120, '-') << std::endl;
    std::cout << std::endl;
}

/*!
 * @brief This function produces the monthly reports, including people treated, people with appointments,
 * and people waiting for appointments.
 * @param container is the crucial data structure.
 * @sideeffects It prints the report to the console.
 */
void generateMonthlyReports(Container& container) {
    std::ofstream file{"data/report.txt", std::ios_base::app};
    size_t num_wait{};
    long waiting_time{};
    std::vector<const RegistrationRecord*> queue_ptr{};
    std::vector<size_t> queue_count{};
    for (const auto& queue : container.localQueues) {
        auto queue_temp = queue.get_ptrs();
        queue_ptr.insert(queue_ptr.end(), queue_temp.begin(), queue_temp.end());
        num_wait += queue.size();
        queue_count.push_back(queue.size());
    }
    for (const auto* const record_ptr : queue_ptr) {
        waiting_time += record_ptr->GetWaitingTime();
    }
    num_wait += container.centralizedQueue.size();
    num_wait += container.waitingList.size();
    num_wait += container.appointmentList.size();
    unsigned long num_appoint{container.appointmentList.size() + container.treatedList.size()};
    unsigned long num_reg{num_wait + container.treatedList.size()};
    auto cent_temp = container.centralizedQueue.get_ptrs();
    for (const auto* const record_ptr : cent_temp) {
        waiting_time += record_ptr->GetWaitingTime();
    }
    for (const auto& record : container.waitingList) {
        waiting_time += record.GetWaitingTime();
    }
    for (const auto& record : container.appointmentList) {
        waiting_time += record.GetWaitingTime();
    }
    for (const auto& record : container.treatedList) {
        waiting_time += record.GetWaitingTime();
    }
    double average_waiting_time = (double) waiting_time / (double) num_reg;

    std::cout << std::endl;
    std::cout << BOLDBLUE << std::setw(50) << std::left << "Number of people registered: " << CYAN << std::left
              << num_reg << std::endl;
    for (size_t i = 0; i < queue_count.size(); ++i) {
        std::cout << BOLDBLUE << "Number of people waiting in Local Queue #" << i + 1
                  << std::setw(8) << std::left << ": " << CYAN << std::left << queue_count[i] << std::endl;
    }
    std::cout << BOLDBLUE << std::setw(50) << std::left << "Number of people waiting in Centralized Queue: " <<
              CYAN << std::left << container.centralizedQueue.size() << std::endl;
    std::cout << BOLDBLUE << std::setw(50) << std::left << "Number of people with an extension: " <<
              CYAN << std::left << container.waitingList.size() << std::endl;
    std::cout << BOLDBLUE << std::setw(50) << std::left
              << "Number of people waiting for treatment: " <<
              CYAN << std::left << container.appointmentList.size() << std::endl;
    std::cout << BOLDBLUE << std::setw(50) << std::left << "Number of people waiting in total: " << CYAN << std::left
              << num_wait << "\n"
              << BOLDBLUE << std::setw(50) << std::left << "Number of appointments made (cumulative): " << CYAN
              << std::left
              << num_appoint << "\n"
              << BOLDBLUE << std::setw(50) << std::left << "Average waiting time: " << CYAN << std::left
              << average_waiting_time << " days\n"
              << BOLDBLUE << std::setw(50) << std::left << "Number of withdrawals: " << CYAN << std::left
              << container.pendingList.size() << "\n"
              << RESET << std::endl;
    file << "\n" << std::setw(50) << std::left << "Number of people registered: " << std::left
         << num_reg << "\n";
    for (size_t i = 0; i < queue_count.size(); ++i) {
        file << BOLDBLUE << "Number of people waiting in Local Queue #" << i + 1
             << std::setw(8) << std::left << ": " << CYAN << std::left << queue_count[i] << "\n";
    }
    file << BOLDBLUE << std::setw(50) << std::left << "Number of people waiting in Centralized Queue: " <<
         CYAN << std::left << container.centralizedQueue.size() << "\n";
    file << BOLDBLUE << std::setw(50) << std::left << "Number of people with an extension: " <<
         CYAN << std::left << container.waitingList.size() << "\n";
    file << BOLDBLUE << std::setw(50) << std::left
         << "Number of people waiting for treatment: " <<
         CYAN << std::left << container.appointmentList.size() << "\n";
    file << std::setw(50) << std::left << "Number of people waiting in total: " << std::left
         << num_wait << "\n"
         << std::setw(50) << std::left << "Number of appointments made (cumulative): "
         << std::left << num_appoint << "\n"
         << std::setw(50) << std::left << "Average waiting time: " << std::left
         << average_waiting_time << " days\n"
         << std::setw(50) << std::left << "Number of withdrawals: " << std::left
         << container.pendingList.size() << "\n";
}
