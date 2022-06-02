/*!
 * @brief This file contains the implementation of class `registrationRecord`.
 */
#include "registrationRecord.h"

/*!
 * @brief This method converts a profession category (int) to Roman numerals.
 * @param record is reference to the record object.
 * @return a string of Roman numerals.
 */
std::string RegistrationRecord::displayProfessionCategory(const RegistrationRecord& record) {
    std::map<int, std::string> intToRoman{
        {1, "I"},
        {2, "II"},
        {3, "III"},
        {4, "IV"},
        {5, "V"},
        {6, "VI"},
        {7, "VII"},
        {8, "VIII"},
        {-1, "ROOT"}
    };
    return intToRoman[record.profession_id_];
}

/*!
 * @brief This method converts age ids to strings.
 * @param record is reference to the record object.
 * @return a string of age category.
 */
std::string RegistrationRecord::displayAgeCategory(const RegistrationRecord& record) {
    std::map<int, std::string> mappingTable{
        {1, "Children"},
        {2, "Adolescents"},
        {3, "Young Adults"},
        {4, "Adults"},
        {5, "Seniors"},
        {6, "Elderly People"},
        {7, "Old People"},
    };
    return mappingTable[record.age_id_];
}

/*!
 * @brief This overloaded constructor takes a CSVRow object as input and writes information to the corresponding fields.
 * @param recordInfo is reference to the record object.
 */
RegistrationRecord::RegistrationRecord(const CSVRow& recordInfo)
    : id_(std::stoi(std::string{recordInfo[0]})), name_(recordInfo[1]), address_(recordInfo[2]),
      phone_(recordInfo[3]), wechat_(recordInfo[4]), email_(recordInfo[5]),
      profession_id_(std::stoi(std::string{recordInfo[6]})),
      risk_status_(std::stoi(std::string(recordInfo[8]))), local_queue_id_(std::stoi(std::string(recordInfo[9]))) {
    time(&timestamp_);
    birthday_ = str2time(std::string(recordInfo[7]));
    setAgeCategory();
    if (2 == risk_status_) {
        extension_ = 60;
    } else if (3 == risk_status_) {
        extension_ = std::numeric_limits<int>::max();  // Infinite waiting time.
    }
}

/*!
 * @brief This overloaded constructor takes a vector of strings as input and writes information to the corresponding fields.
 * @param recordInfo is reference to the record object.
 */
RegistrationRecord::RegistrationRecord(const std::vector<std::string>& recordInfo)
    : id_(std::stoi(std::string{recordInfo[0]})), name_(recordInfo[1]), address_(recordInfo[2]),
      phone_(recordInfo[3]), wechat_(recordInfo[4]), email_(recordInfo[5]),
      profession_id_(std::stoi(std::string{recordInfo[6]})),
      risk_status_(std::stoi(std::string(recordInfo[8]))), local_queue_id_(std::stoi(std::string(recordInfo[9]))) {
    time(&timestamp_);
    timestamp_ += static_cast<time_t>(halfDaysPassed * 12 * 3600);
    birthday_ = str2time(recordInfo[7]);
    setAgeCategory();
    if (2 == risk_status_) {
        extension_ = 60;
    } else if (3 == risk_status_) {
        extension_ = std::numeric_limits<int>::max();  // Infinite waiting time.
    }
}

/*!
 * @brief This function calculates the age of our patients and set the class property.
 */
void RegistrationRecord::setAgeCategory() {
    time_t t = time(nullptr);
    time_t age_time{t - birthday_};
    auto age_tm = localtime(&age_time);
    int age = age_tm->tm_year - 70;
    int bounds[]{12, 18, 35, 50, 65, 75};
    for (int bound : bounds) {
        age_id_++;
        if (age <= bound) { return; }
    }
    age_id_++; // For old people.
}

/*!
 * @brief This method applies a two-week extension (a.k.a. waiting time) for any records being
 * recovered with an extension shorter than two weeks.
 */
void RegistrationRecord::applyPenalty() {
    if (extension_ < 28) {
        extension_ = 28;  // Additional two weeks' waiting time.
    }
}

// Getters and setters.
/*!
 * @brief This overloaded insertion operator outputs the input record to the console.
 * @param os is reference to the ostream.
 * @param record is reference to the record.
 * @return a reference to the modified ostream.
 */
std::ostream& operator<<(std::ostream& os, const RegistrationRecord& record) {
    os << "ID: " << std::setw(10) << std::left << record.id_ << "\tName: " << std::setw(20) << std::left
       << record.name_ << "\tProfession Category: " << std::setw(10) << std::left
       << RegistrationRecord::displayProfessionCategory(record) << "\tRisk Status: " << std::setw(10) << std::left
       << record.risk_status_ << "\tWaiting Time: " << std::setw(3) << std::right << record.GetWaitingTime()
       << std::setw(10) << std::left << " days"
       << "\tAge Category: " << std::setw(5) << std::left << RegistrationRecord::displayAgeCategory(record);
    return os;
}

/*!
 * @brief This method gets the `profession_id_` field of the record.
 * @return the profession id of the record.
 */
int RegistrationRecord::GetProfessionId() const {
    return profession_id_;
}

/*!
 * @brief This method gets the `age_id_` field of the record.
 * @return the age id of the record.
 */
int RegistrationRecord::GetAgeId() const {
    return age_id_;
}

/*!
 * @brief This method gets the `timestamp_` field of the record.
 * @return the timestamp of the record.
 */
time_t RegistrationRecord::GetTimestamp() const {
    return timestamp_;
}

/*!
 * @brief This method gets the `id_` field of the record.
 * @return the id of the record.
 */
int RegistrationRecord::GetId() const {
    return id_;
}

/*!
 * @brief This method gets the `name_` field of the record.
 * @return the name of the record.
 */
const std::string& RegistrationRecord::GetName() const {
    return name_;
}

/*!
 * @brief This method gets the `risk_status_` field of the record.
 * @return the risk status of the record.
 */
int RegistrationRecord::GetRiskStatus() const {
    return risk_status_;
}

/*!
 * @brief This method gets the `extension_` field of the record.
 * @return the extension length (in 12-hour units) of the record.
 */
int RegistrationRecord::GetExtension() const {
    return extension_;
}

/*!
 * @brief This method gets the `local_queue_id_` field of the record.
 * @return the local queue (in which it registered) id of the record.
 */
int RegistrationRecord::GetLocalQueueId() const {
    return local_queue_id_;
}

/*!
 * @brief This method gets the total waiting time for the record.
 * @return number of days.
 */
int RegistrationRecord::GetWaitingTime() const {
    double w_time = treated_ ? difftime(final_time_, timestamp_) : difftime(getRQRSCurrTime(), timestamp_);
    return (int) ceil(w_time / 86400);
}

/*!
 * @brief This method gets the `treat_time_` field of the record.
 * @return the time when the record gets treated.
 */
time_t RegistrationRecord::GetTreatTime() const {
    return treat_time_;
}

/*!
 * @brief This method gets the `treat_loc_id_` field of the record.
 * @return the treated location (a.k.a. appointment location) of the record.
 */
int RegistrationRecord::GetTreatLocId() const {
    return treat_loc_id_;
}

/*!
 * @brief This method gets the `treat_slot_id_` field of the record.
 * @return the treated time slot of the record.
 */
int RegistrationRecord::GetTreatSlotId() const {
    return treat_slot_id_;
}

/*!
 * @brief This overloaded equality operator compares two records basing on their `id_` fields
 * and `name_` fields.
 * @param other is reference to another record object.
 * @return true if equal, false otherwise.
 */
bool RegistrationRecord::operator==(const RegistrationRecord& other) const {
    return id_ == other.id_ && name_ == other.name_;
}

/*!
 * @brief This method decrement the `extension_` field for the object.
 */
void RegistrationRecord::updateExtension() {
    extension_--;
}

/*!
 * @brief This method sets the `profession_id_` field of the record.
 * @param profession_id is the new value desired.
 */
void RegistrationRecord::SetProfessionId(int profession_id) {
    profession_id_ = profession_id;
}

/*!
 * @brief this method sets the `profession_id_` field of the record.
 * @param profession_id is the new value desired.
 */
void RegistrationRecord::SetRiskStatus(int risk_status) {
    risk_status_ = risk_status;
}

/*!
 * @brief this method sets the `extension_` field of the record.
 * @param extension is the new value desired.
 */
void RegistrationRecord::SetExtension(int extension) {
    extension_ = extension;
}

/*!
 * @brief this method sets the `treated_` field of the record.
 * @param treated is the new value desired.
 */
void RegistrationRecord::SetTreated(bool treated) {
    treated_ = treated;
}

/*!
 * @brief this method sets the `final_time_` field of the record.
 * @param final_waiting_time is the new value desired.
 */
void RegistrationRecord::SetFinalWaitingTime(time_t final_waiting_time) {
    final_time_ = final_waiting_time;
}

/*!
 * @brief this method sets the `treat_time_` field of the record.
 * @param treat_time is the new value desired.
 */
void RegistrationRecord::SetTreatTime(time_t treat_time) {
    treat_time_ = treat_time;
}

/*!
 * @brief this method sets the `treat_loc_id_` field of the record.
 * @param treat_loc_id is the new value desired.
 */
void RegistrationRecord::SetTreatLocId(int treat_loc_id) {
    treat_loc_id_ = treat_loc_id;
}

/*!
 * @brief this method sets the `treat_slot_id_` field of the record.
 * @param treat_slot_id is the new value desired.
 */
void RegistrationRecord::SetTreatSlotId(int treat_slot_id) {
    treat_slot_id_ = treat_slot_id;
}

