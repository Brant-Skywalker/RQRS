/*!
 * @brief This function contains functions that call record processing functions based on @em events.
 * They are the core or backbone of our program.
 */
#ifndef CS225_SP22_C1_EVENTDRIVER_H_
#define CS225_SP22_C1_EVENTDRIVER_H_

#include <iostream>
#include <utility>

#include "recordProcessor.h"

void eventTrigger(Container& container);
void move12Hours(Container& container);
void moveNDays(int numberOfDays, Container& container);
void loadRecords(Container& container);
void loadPreferences(Container& container);
void forwardRegistrationRecords(Container& container);
void updateProfessionId(int id, int targetID, Container& container);
void updateRiskStatus(int id, int targetID, Container& container);
void generateWeeklyReports(int order, Container& container);
void generateMonthlyReports(Container& container);

#endif //CS225_SP22_C1_EVENTDRIVER_H_
