/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

#include "settingslistkey.h"

#include <ostream>
#include <iostream>
#include <sstream>


SSettingsListKeyData SettingsListKey::stringToStruct(QString inputString) {
    // first split the values from comma delimited to a vector of strings
    std::vector<std::string> valueVector;
    std::stringstream input(inputString.toStdString());
    while (input.good()) {
        std::string value;
        std::getline(input, value, ',');
        valueVector.push_back(value);
    }
    // check validity
    SSettingsListKeyData outputStruct;
    if (valueVector.size() == 3) {
        outputStruct.name = QString::fromStdString(valueVector[0]);
        outputStruct.index = QString::fromStdString(valueVector[1]).toInt();
        outputStruct.type = (ECommType)QString::fromStdString(valueVector[2]).toInt();
    } else {
        qDebug() << "something went wrong with the key...";
    }
    return outputStruct;
}


QString SettingsListKey::structToString(SSettingsListKeyData listStruct) {
    QString returnString = "";
    returnString = returnString + listStruct.name + "," + QString::number(listStruct.index) + "," + QString::number((int)listStruct.type);
    //qDebug() << "this is the return string" << returnString;
    return returnString;
}
