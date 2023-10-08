#include <iostream>
#include "sqlite/sqlite3.h"
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include <chrono>


using namespace std;

class Employee {
public:
    string surname;
    string name;
    string patronymic;
    char gender;  
    int birthYear;
    int birthMonth;
    int birthDay;
    int age;

   
    void sendToDatabase(sqlite3* db) {
        char insertSQL[500];
        sprintf_s(insertSQL, "INSERT INTO employees (surname, name, patronymic, gender, birthYear, birthMonth, birthDay) VALUES ('%s', '%s', '%s', '%c', %d, %d, %d);",
            surname.c_str(), name.c_str(), patronymic.c_str(),
            gender, birthYear, birthMonth, birthDay);

        char* errMsg = nullptr;
        int rc = sqlite3_exec(db, insertSQL, nullptr, 0, &errMsg);

        if (rc != SQLITE_OK) {
            cerr << "SQL error: " << errMsg << endl;
            sqlite3_free(errMsg);
        }
    }


    void calculateAge() {
        auto now = chrono::system_clock::now();
        time_t now_c = chrono::system_clock::to_time_t(now);
        tm current_tm;
        localtime_s(&current_tm, &now_c);

        int currentYear = current_tm.tm_year + 1900;
        int currentMonth = current_tm.tm_mon + 1;
        int currentDay = current_tm.tm_mday;

        age = currentYear - birthYear;
        if (currentMonth < birthMonth || (currentMonth == birthMonth && currentDay < birthDay)) {
            age--;
        }
    }
};

void createTable(sqlite3* db);
void data_entry(sqlite3* db);
void print(sqlite3* db);
void data_change(sqlite3* db);
void data_delete(sqlite3* db);
void data_sort(sqlite3* db);
vector<Employee> generateRandomEmployees(sqlite3* db, int count, int maleCount, int femaleCount);

int main() {
    sqlite3* db;
    int rc = sqlite3_open("employee.db", &db);
    if (rc) {
        cerr << "Cannot open database: " << sqlite3_errmsg(db) << endl;
        return 1;
    }

    createTable(db);

    int start;
    while (true) {
        cout << "Select an action:" << endl;
        cout << "(0) Exit the program" << endl;
        cout << "(1) Data entry" << endl;
        cout << "(2) Print" << endl;
        cout << "(3) Change" << endl;
        cout << "(4) Delete" << endl;
        cout << "(5) Generate Random Employees" << endl;
        cout << "(6) Sorting" << endl;
        cout << "Your choice: ";
        cin >> start;

        switch (start) {
        case 0:
            sqlite3_close(db);
            return 0;
        case 1:
            data_entry(db);
            break;
        case 2:
            print(db);
            break;
        case 3:
            data_change(db);
            break;
        case 4:
            data_delete(db);
            break;
        case 5:
        {
            auto start_time = chrono::high_resolution_clock::now();
            generateRandomEmployees(db, 100000, 50000, 50000);
            auto end_time = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
            cout << "Time taken: " << duration.count() << " milliseconds" << endl;
            break;
        }
        case 6:
            data_sort(db);
            break;
        default:
            cout << "Invalid action number" << endl;
            break;
        }
    }

    return 0;
}

void createTable(sqlite3* db) {
    const char* createTableSQL = "CREATE TABLE IF NOT EXISTS employees ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "surname TEXT,"
        "name TEXT,"
        "patronymic TEXT,"
        "gender CHAR(1),"
        "birthYear INT,"
        "birthMonth INT,"
        "birthDay INT"
        ");";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, createTableSQL, nullptr, 0, &errMsg);

    if (rc != SQLITE_OK) {
        cerr << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg);
    }
}

void data_entry(sqlite3* db) {
    int n;
    cout << "Write the number of data: ";
    cin >> n;

    for (int i = 0; i < n; i++) {
        Employee emp;
        cout << "Enter the full name: ";
        cin >> emp.surname >> emp.name >> emp.patronymic;

        cout << "Enter the gender (M/F): ";
        cin >> emp.gender;

        int birthYear, birthMonth, birthDay;
        cout << "Enter the birthdate (year month day): ";
        cin >> birthYear >> birthMonth >> birthDay;

        emp.birthYear = birthYear;
        emp.birthMonth = birthMonth;
        emp.birthDay = birthDay;

        
        emp.calculateAge();
        emp.sendToDatabase(db);
    }
}

void print(sqlite3* db) {
    const char* selectSQL = "SELECT * FROM employees;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL error: " << sqlite3_errmsg(db) << endl;
        return;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        string surname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        string patronymic = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        char gender = sqlite3_column_text(stmt, 4)[0];
        int birthYear = sqlite3_column_int(stmt, 5);
        int birthMonth = sqlite3_column_int(stmt, 6);
        int birthDay = sqlite3_column_int(stmt, 7);

        cout << "ID: " << id << endl;
        cout << "Full name: " << surname << " " << name << " " << patronymic << endl;
        cout << "Gender: " << gender << endl;
        cout << "Birthdate: " << birthYear << "-" << birthMonth << "-" << birthDay << endl;
        cout << "Age: " << birthYear << " years" << endl;
        cout << "___________________________________" << endl;
    }

    if (rc != SQLITE_DONE) {
        cerr << "SQL error: " << sqlite3_errmsg(db) << endl;
    }

    sqlite3_finalize(stmt);
}

void data_change(sqlite3* db) {
    int id;
    cout << "Enter the item number to change: ";
    cin >> id;

    char selectSQL[100];
    sprintf_s(selectSQL, "SELECT * FROM employees WHERE id = %d;", id);

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL error: " << sqlite3_errmsg(db) << endl;
        return;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        string surname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        string patronymic = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        char gender = sqlite3_column_text(stmt, 4)[0];
        int birthYear = sqlite3_column_int(stmt, 5);
        int birthMonth = sqlite3_column_int(stmt, 6);
        int birthDay = sqlite3_column_int(stmt, 7);

        cout << "Current data:" << endl;
        cout << "ID: " << id << endl;
        cout << "Full name: " << surname << " " << name << " " << patronymic << endl;
        cout << "Gender: " << gender << endl;
        cout << "Birthdate: " << birthYear << "-" << birthMonth << "-" << birthDay << endl;
        cout << "Age: " << birthYear << " years" << endl;
        cout << "___________________________________" << endl;

        Employee newData;
        cout << "Enter the new data:" << endl;
        cout << "Full name: ";
        cin >> newData.surname >> newData.name >> newData.patronymic;
        cout << "Gender (M/F): ";
        cin >> newData.gender;

        int newBirthYear, newBirthMonth, newBirthDay;
        cout << "Enter the new birthdate (year month day): ";
        cin >> newBirthYear >> newBirthMonth >> newBirthDay;

        newData.birthYear = newBirthYear;
        newData.birthMonth = newBirthMonth;
        newData.birthDay = newBirthDay;

       
        newData.calculateAge();

        char updateSQL[500];
        sprintf_s(updateSQL, "UPDATE employees SET surname = '%s', name = '%s', patronymic = '%s', gender = '%c', birthYear = %d, birthMonth = %d, birthDay = %d WHERE id = %d;",
            newData.surname.c_str(), newData.name.c_str(), newData.patronymic.c_str(),
            newData.gender, newBirthYear, newBirthMonth, newBirthDay, id);

        char* errMsg = nullptr;
        rc = sqlite3_exec(db, updateSQL, nullptr, 0, &errMsg);

        if (rc != SQLITE_OK) {
            cerr << "SQL error: " << errMsg << endl;
            sqlite3_free(errMsg);
        }
    }
    else {
        cout << "Item not found." << endl;
    }

    sqlite3_finalize(stmt);
}

void data_delete(sqlite3* db) {
    int id;
    cout << "Enter the item number to delete: ";
    cin >> id;

    char deleteSQL[100];
    sprintf_s(deleteSQL, "DELETE FROM employees WHERE id = %d;", id);

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, deleteSQL, nullptr, 0, &errMsg);

    if (rc != SQLITE_OK) {
        cerr << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg);
    }
    else {
        cout << "Data deleted." << endl;
    }
}

void data_sort(sqlite3* db) {
    const char* selectSQL = "SELECT * FROM employees ORDER BY surname;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "SQL error: " << sqlite3_errmsg(db) << endl;
        return;
    }

    vector<Employee> data;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        string surname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        string patronymic = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        char gender = sqlite3_column_text(stmt, 4)[0];
        int birthYear = sqlite3_column_int(stmt, 5);
        int birthMonth = sqlite3_column_int(stmt, 6);
        int birthDay = sqlite3_column_int(stmt, 7);

        Employee emp;
        emp.surname = surname;
        emp.name = name;
        emp.patronymic = patronymic;
        emp.gender = gender;
        emp.birthYear = birthYear;
        emp.birthMonth = birthMonth;
        emp.birthDay = birthDay;

        data.push_back(emp);
    }

    if (rc != SQLITE_DONE) {
        cerr << "SQL error: " << sqlite3_errmsg(db) << endl;
    }

    sqlite3_finalize(stmt);

    cout << "Sorted data:" << endl;
    for (const Employee& emp : data) {
        cout << "Full name: " << emp.surname << " " << emp.name << " " << emp.patronymic << endl;
        cout << "Gender: " << emp.gender << endl;
        cout << "Birthdate: " << emp.birthYear << "-" << emp.birthMonth << "-" << emp.birthDay << endl;
        cout << "Age: " << emp.age << " years" << endl;
        cout << "___________________________________" << endl;
    }
}

vector<Employee> generateRandomEmployees(sqlite3* db, int count, int maleCount, int femaleCount) {
    vector<Employee> employees;

    srand(static_cast<unsigned>(time(nullptr)));

    string maleNames[] = { "James", "John", "Robert", "Michael", "William", "David", "Richard", "Joseph", "Charles", "Thomas" };
    string femaleNames[] = { "Mary", "Patricia", "Jennifer", "Linda", "Elizabeth", "Susan", "Jessica", "Sarah", "Karen", "Nancy" };
    string lastNames[] = { "Smith", "Johnson", "Brown", "Taylor", "Miller", "Anderson", "Thomas", "Jackson", "White", "Harris" };

    sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, 0, nullptr);


    string batchInsertSQL = "INSERT INTO employees (surname, name, patronymic, gender, birthYear, birthMonth, birthDay) VALUES ";

    for (int i = 0; i < count; i++) {
        string gender;
        string name;
        if (i < maleCount) {
            gender = "M";
            name = maleNames[rand() % 10];
        }
        else {
            gender = "F";
            name = femaleNames[rand() % 10];
        }

        Employee emp;
        emp.surname = lastNames[rand() % 10];
        emp.name = name;
        emp.patronymic = lastNames[rand() % 10];
        emp.gender = gender[0];
        emp.birthYear = 1980 + rand() % 21;
        emp.birthMonth = 1 + rand() % 12;
        emp.birthDay = 1 + rand() % 28;
        emp.calculateAge();


        batchInsertSQL += "('" + emp.surname + "', '" + emp.name + "', '" + emp.patronymic + "', '" + emp.gender + "', " +
            to_string(emp.birthYear) + ", " + to_string(emp.birthMonth) + ", " + to_string(emp.birthDay) + ")";

        if (i < count - 1) {

            batchInsertSQL += ", ";
        }

     
        employees.push_back(emp);
    }


    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, (batchInsertSQL + ";").c_str(), nullptr, 0, &errMsg);


    if (rc != SQLITE_OK) {
        cerr << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg);
    }
    else {

        sqlite3_exec(db, "COMMIT", nullptr, 0, nullptr);
    }

    return employees;
}
