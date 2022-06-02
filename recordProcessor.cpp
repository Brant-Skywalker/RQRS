/*!
 * @brief This file contains constructor implementation for Container class and implementations of functions
 * which process registration records.
 */
#include "recordProcessor.h"

/*!
 * @brief This function creates two vectors with size specified by global variables and append them to the new
 * container object.
 * @param num_reg is number of registries.
 * @param num_loc is number of appointment locations.
 */
Container::Container(int num_reg, int num_loc) {
    preferences = std::vector<std::vector<int>>
        (num_reg, std::vector<int>(num_loc, 0));  // Appointment location preferences for each local queue.
    availabilities = std::vector<std::vector<bool>>
        (num_loc, std::vector<bool>(numSlot, true));  // Availability of each time slot.
}

/*!
 * @brief This function processes records in the waiting list. It decrements their `extension_` fields and
 * push them to a random local queue once their extension ends.
 * @param container is the crucial data structure container.
 */
void waitingListProcessor(Container& container) {
    if (container.waitingList.empty()) { return; }
    for (auto iter = container.waitingList.begin(); iter != container.waitingList.end();) {
        auto& record = *iter;
        record.updateExtension();
        if (0 == record.GetExtension()) {  // Extension ended, the record is put into a random local queue.
            int queueID{generateRandomRangedInt(0, numReg - 1)};
            container.localQueues[queueID].emplace(record);
            container.waitingList.erase(std::remove(
                container.waitingList.begin(), container.waitingList.end(), record), container.waitingList.end());
            updateDBRecord(container, record, 0);
        } else { ++iter; }  // Don't use range-based for loop when you erase something.
    }

}

/*!
 * @brief This function reads user input from the keyboard and generate a new registration record
 * and push it to the waiting list or a random local queue according to its risk status.
 * @param container is the crucial data structure container.
 */
void newRegistration(Container& container) {
    std::vector<std::string> recordInfo;
    int id;
    std::cout << BLUE << "Please enter your identification number (501-" << std::numeric_limits<int>::max() << "): "
              << RESET << std::endl;
    scanIntRange(id, 501, std::numeric_limits<int>::max());
    recordInfo.emplace_back(std::to_string(id));
    std::cout << BLUE << "Please enter your name: " << RESET << std::endl;
    std::string name;
    std::cin.clear();
    std::cin.ignore(256, '\n');
    getline(std::cin, name);
    recordInfo.emplace_back(std::move(name));
    std::cout << BLUE << "Please enter your address: " << RESET << std::endl;
    std::string address;
    std::cin.clear();
    getline(std::cin, address);
    recordInfo.emplace_back(std::move(address));
    std::cout << BLUE << "Please enter your phone number: " << RESET << std::endl;
    std::string phone;
    std::cin.clear();
    getline(std::cin, phone);
    recordInfo.emplace_back(std::move(phone));
    std::cout << BLUE << "Please enter your WeChat ID: " << RESET << std::endl;
    std::string WeChat;
    std::cin.clear();
    getline(std::cin, WeChat);
    recordInfo.emplace_back(std::move(WeChat));
    std::cout << BLUE << "Please enter your email: " << RESET << std::endl;
    std::string email;
    std::cin.clear();
    getline(std::cin, email);
    recordInfo.emplace_back(std::move(email));
    std::cout << BLUE << "Please enter your profession code (1-8): " << RESET << std::endl;
    int prof;
    scanIntRange(prof, 1, 8);
    recordInfo.emplace_back(std::to_string(prof));
    std::cout << BLUE << "Please enter your date of birth (yyyy-mm-dd): " << RESET
              << std::endl;
    time_t date;
    scanDate(date);
    recordInfo.emplace_back(time2str(date));
    std::cout << BLUE << "Please enter your risk status (0-3): " << RESET << std::endl;
    int risk;
    scanIntRange(risk, 0, 3);
    recordInfo.emplace_back(std::to_string(risk));
    int regID;
    std::cout << CYAN << "Please enter local registry ID (1-" << numReg << "): " << RESET << std::endl;
    scanIntRange(regID, 1, numReg);
    recordInfo.emplace_back(std::to_string(regID));
    RegistrationRecord record{recordInfo};
    addDBRecord(container, record, regID);
    if (0 == risk || 1 == risk) {
        container.localQueues[regID - 1].push(record);  // Passed to the overloaded constructor.
    } else {
        container.waitingList.emplace_back(recordInfo);  // Also passed to the overloaded constructor.
    }
    std::cout << BOLDGREEN << "New registration record successfully created!" << std::endl;
    std::string path{"data/reg_" + std::to_string(regID) + ".csv"};
    std::ofstream outfile{path, std::ios_base::app};  // Append to the end of text file.
    if (!outfile.is_open()) {
        std::cerr << "Failed to open or append to file " << path << "!" << std::endl;
        throw IOError();
    }
    for (const auto& info : recordInfo) {
        outfile << info << ',';  // The terminating ',' doesn't really matter.
    }
    outfile << '\n';
    std::cout << BOLDGREEN << "New registration record successfully saved!" << std::endl;
}

/*!
 * @brief This function attaches deadlines to individual records.
 * @param id is id of the record to be assigned a deadline.
 * @param deadline is deadline of treatment (time_t).
 * @param container is the crucial data structure container.
 */
void addDeadline(int id, time_t deadline, Container& container) {
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
            auto& record{*queue_iter};  // Get the reference of the object.
            container.deadlineTracker.emplace_back(std::make_pair(&record,
                                                                  deadline));  // Save the pointer to the record.
            std::cout << BOLDGREEN << "Registration record (ID " << id
                      << ") found in a local queue has been successfully updated with a deadline!" << RESET
                      << std::endl;
            return;  // Done update.
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
            container.deadlineTracker.emplace_back(std::make_pair(&record,
                                                                  deadline));  // Save the pointer to the record.
            std::cout << BOLDGREEN << "Registration record (ID " << id
                      << ") found in the waiting list has been successfully updated with a deadline!" << RESET
                      << std::endl;
            return;  // Done update.
        }
    }
    // Finally, look at the centralized queue.
    if (!container.centralizedQueue.empty()) {
        auto centralized_iter =
            container.centralizedQueue.find_if(found, [&id](const auto& pair) {
                return pair.first.GetId() == id;  // `&` means "capture by reference."
            });
        if (found) {  // Found in centralized queue!
            auto& record{centralized_iter->second->key};  // Get a (non-const) reference of the object.
            container.deadlineTracker.emplace_back(std::make_pair(&record,
                                                                  deadline));  // Save the pointer to the record.
            std::cout << BOLDGREEN << "Registration record (ID " << id
                      << ") found in the centralized queue has been updated with a deadline!" << RESET
                      << std::endl;
            return;  // Done update.
        }
    }
    std::cout << BOLDRED << "Registration record (ID " << id
              << ") not found in local queue, centralized queue, or waiting list!" << RESET << std::endl;
}

/*!
 * @brief This function writes the appointment information to a data file.
 * @param success is a flag indicates whether output has been successfully operated.
 * @param record is reference to the assigned record.
 * @param locationID is ID of appointment location.
 * @param slotID is ID of appointment time slot.
 */
void writeAppointment(bool& success, const RegistrationRecord& record, int locationID, int slotID) {
    std::map<int, std::string> timeTable{
        {1, "8:00 - 9:00"},
        {2, "9:30 - 10:30"},
        {3, "11:00 - 12:00"},
        {4, "13:00 - 14:00"},
        {5, "14:30 - 15:30"},
        {6, "16:00 - 17:00"},
    };
    std::string path{"data/appointment.csv"};
    std::ofstream of{path, std::ios_base::app};  // Append mode.
    of << record.GetId() << ',' << record.GetName() << ',' << locationID << ',' << timeTable[slotID] << '\n';
    if (of.bad()) {
        success = false;
        throw IOError();
    }
    success = true;
}

/*!
 * @brief This method assigns appointment info to input records.
 * @param success indicates whether an appointment has been assigned.
 * @param record is constant reference of the processed record.
 * @param container is the crucial data structure container.
 */
void assignAppointment(bool& success, RegistrationRecord& record, Container& container) {
    // Search for an empty time slot in order of preference.
    for (auto locationID : container.preferences[record.GetLocalQueueId()
        - 1]) {  // `int` is cheap. No reference needed.
        for (int i = 1; i <= numSlot; ++i) {
            if (!container.availabilities[locationID - 1][i - 1]) { continue; }
            try {
                container.availabilities[locationID - 1][i - 1] = false;  // Slot occupied.
                writeAppointment(success, record, locationID, i);
            } catch (const IOError& error) {
                std::cerr << error.what() << std::endl;
            }
            record.SetTreatTime(getNextDay());
            record.SetTreatLocId(locationID);
            record.SetTreatSlotId(i);
            return;
        }
    }
    success = false;
    std::cout << BOLDRED << "No available time slots for all appointment locations!" << RESET << std::endl;
    std::cout << BOLDYELLOW << "The rest registration records will be processed later." << std::endl;
}

/*!
 * @brief This function assigns appointment to records meeting their treatment deadlines or records with the highest
 * priority in the centralized queue. It examines the centralized queue and deadline tracker.
 * @param container is the crucial data structure container.
 */
void appointmentProcessor(Container& container) {
    // Always process the `min` element of the priority queue.
    if (container.centralizedQueue.empty()) {
        std::cout << BOLDYELLOW << "No appointment record found in RQRS! Please add new registries." << std::endl;
        return;  // Do nothing!
    }
    auto top_record{container.centralizedQueue.top()}; // It doesn't seem safe here to use move semantics.
    bool success{false};
    bool found{false};
    assignAppointment(success, top_record, container);
    if (!success) { return; }  // There is also no available slots for records meeting deadlines.
    std::cout << BOLDGREEN << "Registration record (ID " << top_record.GetId()
              << ") with the highest priority in the centralized queue has been assigned an appointment!"
              << RESET << std::endl;
    container.appointmentList.emplace_back(top_record);
    container.centralizedQueue.pop();
    // Update database.
    updateDBRecord(container, top_record, 2);
    // Check the deadline tracker.
    if (container.deadlineTracker.empty()) { return; }  // No need to process deadlines.
    time_t curr_time = getRQRSCurrTime();
    for (auto iter = container.deadlineTracker.begin(); iter != container.deadlineTracker.end();) {
        auto& record_pair = *iter;
        if (record_pair.second > curr_time) { continue; }  // Deadline not met!
        auto& record_ref{*record_pair.first};  // Reference to the record.
        // First search the local queues for the current record.
        for (auto& queue : container.localQueues) {
            if (queue.empty()) {
                continue;  // Skip current queue.
            }
            auto queue_iter = queue.find(found, record_ref);
            if (found) {  // Found in local queue!
                assignAppointment(success, record_ref, container);
                if (!success) { return; }  // No space for other appointments.
                int id{record_ref.GetId()};
                container.appointmentList.emplace_back(record_ref);
                updateDBRecord(container, record_ref, 2);
                queue.erase(queue_iter);
                container.deadlineTracker.erase(std::remove(container.deadlineTracker.begin(),
                                                            container.deadlineTracker.end(),
                                                            record_pair),
                                                container.deadlineTracker.end());  // Erase-remove idiom.
                std::cout << BOLDGREEN << "Registration record (ID " << id
                          << ") found in a local queue has reached its deadline and been assigned an appointment!"
                          << RESET << std::endl;
                break;  // No need to check other queues.
            }
        }
        if (found) { continue; }  // Current record done. No iterator increment.
        // Then search the centralized queue.
        auto cent_iter = container.centralizedQueue.find(found, record_ref);  // Search for the node.
        if (found) {
            int id{record_ref.GetId()};
            assignAppointment(success,
                              record_ref,
                              container);  // Assign appointment to the record.
            if (!success) { return; }  // No need to check other records.
            container.appointmentList.emplace_back(record_ref);
            auto temp{record_ref};  // Make a copy.
            temp.SetProfessionId(-1);  // Make `temp` the root.
            container.centralizedQueue.decreaseKey(cent_iter->second,
                                                   std::move(temp));  // Make the modified record the root.
            container.centralizedQueue.pop();  // Remove the record from the centralized queue.
            container.deadlineTracker.erase(std::remove(container.deadlineTracker.begin(),
                                                        container.deadlineTracker.end(),
                                                        record_pair),
                                            container.deadlineTracker.end());  // Erase-remove idiom.
            updateDBRecord(container, record_ref, 2);
            std::cout << BOLDGREEN << "Registration record (ID " << id
                      << ") found in the centralized queue has reached its deadline and been assigned an appointment!"
                      << RESET << std::endl;
            continue;  // Done for the current record. No iterator increment.
        }
        // Finally, search the waiting list.
        auto waiting_iter = std::find(container.waitingList.begin(), container.waitingList.end(), record_ref);
        if (waiting_iter != container.waitingList.end()) {  // Found in waiting list!
            assignAppointment(success, record_ref, container);
            if (!success) { return; }  // No need to check for other records.
            int id{record_ref.GetId()};
            container.appointmentList.emplace_back(record_ref);
            container.waitingList.erase(waiting_iter);
            container.deadlineTracker.erase(std::remove(container.deadlineTracker.begin(),
                                                        container.deadlineTracker.end(),
                                                        record_pair),
                                            container.deadlineTracker.end());  // Erase-remove idiom.
            updateDBRecord(container, record_ref, 2);
            std::cout << BOLDGREEN << "Registration record (ID " << id
                      << ") found in the waiting list has reached its deadline and been assigned an appointment!"
                      << RESET << std::endl;
            continue;  // No iterator increment.
        }  // Done for the current record.
        ++iter;  // Not found anywhere. Deadline is not modified.
    }
}

/*!
 * @brief This function process records with assigned appointments. It examines the appointment list.
 * @param container is the crucial data structure container.
 */
void treatmentProcessor(Container& container) {
    time_t curr_time = getRQRSCurrTime();
    for (auto iter = container.appointmentList.begin(); iter != container.appointmentList.end();) {
        auto& record_ref{*iter};  // Reference to the record.
        if (record_ref.GetTreatTime() > curr_time) {
            ++iter;  // No records removed here. Increment the iterator.
            continue;
        }  // Appointment time not met.
        int id{record_ref.GetId()};
        record_ref.SetTreated(true);  // Update status.
        record_ref.SetFinalWaitingTime(getRQRSCurrTime());
        container.availabilities[record_ref.GetTreatLocId() - 1][record_ref.GetTreatSlotId() - 1]
            = true;  // Free the slot.
        container.treatedList.emplace_back(record_ref);
        container.appointmentList.erase(std::remove(container.appointmentList.begin(),
                                                    container.appointmentList.end(),
                                                    record_ref),
                                        container.appointmentList.end());  // Erase-remove idiom.
        // Generate a random treatment! Priority? It's a waste of time to create a new set of rule! :)
        updateDBRecord(container, record_ref, 4, generateRandomRangedInt(0, 2));
        std::cout << BOLDGREEN << "Record (ID " << id << ") has been treated!" << std::endl;
        // The current record has been removed. No need to increment the iterator.
    }
}

void addDBRecord(Container& container, RegistrationRecord& record, int regID) {
    int id = record.GetId();
    std::string name = record.GetName();
    DBRecord db_record{record, regID};
    container.primaryDB.insert(id, db_record);
    container.secondaryDB.insert(name, db_record);
}

void updateDBRecord(Container& container, RegistrationRecord& record, int medical_status) {
    int id = record.GetId();
    if (!container.primaryDB.contains(id)) { return; }
    const std::string& name = record.GetName();
    auto temp = *container.primaryDB.search(id);
    temp.SetMedicalStatus(medical_status);
    temp.SetRecord(record);
    container.primaryDB.remove(id);
    container.primaryDB.insert(id, temp);
    container.secondaryDB.remove(name);
    container.secondaryDB.insert(name, temp);
}

void updateDBRecord(Container& container, RegistrationRecord& record, int medical_status, int treatment) {
    int id = record.GetId();
    const std::string& name = record.GetName();
    if (!container.primaryDB.contains(id)) { return; }
    auto temp = *container.primaryDB.search(id);
    temp.SetMedicalStatus(medical_status);
    temp.SetRecord(record);
    temp.SetTreatment(treatment);
    container.primaryDB.remove(id);
    container.primaryDB.insert(id, temp);
    container.secondaryDB.remove(name);
    container.secondaryDB.insert(name, temp);
}

void removeDBRecord(Container& container, int id) {
    if (!container.primaryDB.contains(id)) {
        std::cout << BOLDRED << "Database record (ID: " << id << ") does not exist!" << std::endl;
        std::cout << std::endl;
        return;
    }
    std::string name = container.primaryDB.search(id)->GetRecord().GetName();
    container.primaryDB.remove(id);
    container.secondaryDB.remove(name);
    std::cout << BOLDGREEN << "Database record (ID: " << id << ", Name: " << name << ") has been successfully removed!"
              << std::endl;
    std::cout << std::endl;
}

void removeDBRecord(Container& container, const std::string& name) {
    if (!container.secondaryDB.contains(name)) {
        std::cout << BOLDRED << "Database record (Name: " << name << ") does not exist!" << std::endl;
        std::cout << std::endl;
        return;
    }
    int id = container.secondaryDB.search(name)->GetRecord().GetId();
    container.primaryDB.remove(id);
    container.secondaryDB.remove(name);
    std::cout << BOLDGREEN << "Database record (ID: " << id << ", Name: " << name << ") has been successfully removed!"
              << std::endl;
    std::cout << std::endl;
}

void printDBRecord(Container& container, int id) {
    std::unordered_map<int, std::string> med_map{
        {0, "Registered"},
        {1, "Queueing"},
        {2, "Appointment Assigned"},
        {3, "Withdrawn"},
        {4, "Treated"}
    };

    std::unordered_map<int, std::string> reg_map{
        {0, "Carle Foundation Hospital"},
        {1, "OSF Heart of Mary Medical Center"},
        {2, "Springfield Memorial Hospital"},
        {3, "McKinley Health Center"},
        {4, "Hangzhou No.7 People's Hospital"}
    };

    std::unordered_map<int, std::string> treat_map{
        {-1, "Untreated"},
        {1, "Targeted Therapies"},
        {2, "Chemotherapy"},
        {3, "Surgery"}
    };

    if (!container.primaryDB.contains(id)) {
        std::cout << BOLDRED << "Database record (ID " << id << ") does not exist!" << std::endl;
        return;
    }
    auto db_record = *container.primaryDB.search(id);
    auto record = db_record.GetRecord();
    std::cout << std::endl;
    std::cout << BOLDYELLOW << "***  START of Database Query  ***" << RESET << std::endl;
    std::cout << BOLDCYAN << "PERSON: " << RESET;
    std::cout << BOLDMAGENTA << record << RESET << std::endl;
    std::cout << BOLDCYAN << "MEDICAL STATUS: " << med_map[db_record.GetMedicalStatus()] << RESET << std::endl;
    std::cout << BOLDCYAN << "REGISTRATION AT: " << reg_map[db_record.GetRegistration()] << RESET << std::endl;
    std::cout << BOLDCYAN << "TREATMENT: " << treat_map[db_record.GetTreatment()] << RESET << std::endl;
    std::cout << BOLDYELLOW << "***   END of Database Query   ***" << RESET << std::endl;
    std::cout << std::endl;
}

void printDBRecord(Container& container, const std::string& name) {
    std::unordered_map<int, std::string> med_map{
        {0, "Registered"},
        {1, "Queueing"},
        {2, "Appointment Assigned"},
        {3, "Withdrawn"},
        {4, "Treated"}
    };

    std::unordered_map<int, std::string> reg_map{
        {0, "Carle Foundation Hospital"},
        {1, "OSF Heart of Mary Medical Center"},
        {2, "Springfield Memorial Hospital"},
        {3, "McKinley Health Center"},
        {4, "Hangzhou No.7 People's Hospital"}
    };

    std::unordered_map<int, std::string> treat_map{
        {-1, "Untreated"},
        {1, "Targeted Therapies"},
        {2, "Chemotherapy"},
        {3, "Surgery"}
    };
    if (!container.secondaryDB.contains(name)) {
        std::cout << BOLDRED << "Database record (Name:  " << name << ") does not exist!" << RESET << std::endl;
        return;
    }
    auto db_record = *container.secondaryDB.search(name);
    auto record = db_record.GetRecord();
    std::cout << std::endl;
    std::cout << BOLDYELLOW << "***  START of Database Query  ***" << RESET << std::endl;
    std::cout << BOLDCYAN << "PERSON: " << RESET << std::endl;
    std::cout << BOLDMAGENTA << record << RESET << std::endl;
    std::cout << BOLDCYAN << "MEDICAL STATUS: " << med_map[db_record.GetMedicalStatus()] << RESET << std::endl;
    std::cout << BOLDCYAN << "REGISTRATION AT: " << reg_map[db_record.GetRegistration()] << RESET << std::endl;
    std::cout << BOLDCYAN << "TREATMENT: " << treat_map[db_record.GetTreatment()] << RESET << std::endl;
    std::cout << BOLDYELLOW << "***   END of Database Query   ***" << RESET << std::endl;
    std::cout << std::endl;
}
