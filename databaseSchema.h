/*!
 * @brief This class contains the declaration of class DBRecord.
 */
#ifndef CS225_SP22_C2_DATABASESCHEMA_H_
#define CS225_SP22_C2_DATABASESCHEMA_H_
#include "registrationRecord.h"

class DBRecord {
private:
    RegistrationRecord record_{};  // i.e. the relation PERSON.
    int medical_status_{0};
    int registration_{0};  // This field should never be changed again.
    int treatment_{-1};
public:
    DBRecord() = default;
    explicit DBRecord(RegistrationRecord& record, int registration);
    virtual ~DBRecord() = default;
    [[nodiscard]] RegistrationRecord GetRecord() const;
    [[nodiscard]] int GetMedicalStatus() const;
    [[nodiscard]] int GetRegistration() const;
    [[nodiscard]] int GetTreatment() const;
    void SetRecord(const RegistrationRecord& record);
    void SetMedicalStatus(int medical_status);
    [[maybe_unused]] void SetRegistration(int registration);
    void SetTreatment(int treatment);
};

#endif //CS225_SP22_C2_DATABASESCHEMA_H_
