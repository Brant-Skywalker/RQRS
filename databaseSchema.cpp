/*!
 * @brief This class contains the implementation of class DBRecord.
 */

#include "databaseSchema.h"

int DBRecord::GetMedicalStatus() const {
    return medical_status_;
}

void DBRecord::SetMedicalStatus(int medical_status) {
    medical_status_ = medical_status;
}

int DBRecord::GetRegistration() const {
    return registration_;
}

[[maybe_unused]] void DBRecord::SetRegistration(int registration) {
    registration_ = registration;
}

int DBRecord::GetTreatment() const {
    return treatment_;
}

void DBRecord::SetTreatment(int treatment) {
    treatment_ = treatment;
}

DBRecord::DBRecord(RegistrationRecord& record, int registration)
    : record_(record), registration_{registration} {
}

RegistrationRecord DBRecord::GetRecord() const {
    return record_;
}

void DBRecord::SetRecord(const RegistrationRecord& record) {
    record_ = record;
}