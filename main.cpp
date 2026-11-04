/*
 * ============================================================================
 *  Smart Campus Lost & Found Management System
 *  Course  : BCA I Semester (2026-2027)  |  Lab Sheet 10
 *  Language: Standard C++ (C++11 or newer), console based
 *
 *  Modules:
 *    1. Student Login            6. View All Records
 *    2. Report Lost Item         7. Delete Record
 *    3. Report Found Item        8. Item Statistics
 *    4. Search Item              9. Export Report
 *    5. Auto Match Lost & Found 10. Exit
 *
 *  Concepts used: structures, arrays, functions, file handling, loops,
 *                 conditional statements, strings, searching, sorting.
 *
 *  Run the program from the project root folder so the relative paths
 *  "data/records.txt" and "reports/item_report.csv" can be found.
 * ============================================================================
 */

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <cctype>
#include <cstdlib>
#include <ctime>

using namespace std;

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------
const int MAX_ITEMS = 500;          // maximum records the system can hold
const int MAX_MATCHES = 250;        // maximum lost/found pairs shown at once
const int CATEGORY_COUNT = 6;       // number of item categories
const int FIELDS_PER_RECORD = 10;   // number of fields stored per line in file
const int MATCH_THRESHOLD = 60;     // minimum score (out of 100) to call a pair a match
const int FIRST_ITEM_ID = 1001;     // IDs start from this number

const string DATA_FILE = "data/records.txt";
const string REPORT_FILE = "reports/item_report.csv";
const string ADMIN_ID = "ADMIN";
const string ADMIN_PASSWORD = "admin123";   // demo password for the practical project

const string TYPE_LOST = "LOST";
const string TYPE_FOUND = "FOUND";
const string STATUS_OPEN = "OPEN";
const string STATUS_RESOLVED = "RESOLVED";

const string CATEGORIES[CATEGORY_COUNT] = {
    "ID Card", "Wallet", "Keys", "Laptop", "Mobile Phone", "Other"
};

// ----------------------------------------------------------------------------
// Data structures
// ----------------------------------------------------------------------------

// One lost or found record.
struct Item {
    int id;                 // unique item ID
    string type;            // "LOST" or "FOUND"
    string name;            // item name, e.g. "Black Wallet"
    string category;        // one of CATEGORIES
    string location;        // where it was lost / found
    string description;     // extra details (colour, brand, marks...)
    string date;            // date reported (YYYY-MM-DD)
    string reporterId;      // student ID of the person who reported
    string reporterName;    // name of the person who reported
    string status;          // "OPEN" or "RESOLVED"
};

// A possible lost/found pair produced by the auto-match module.
struct MatchResult {
    int lostIndex;          // position of the lost item in items[]
    int foundIndex;         // position of the found item in items[]
    int score;              // similarity score out of 100
};

// ----------------------------------------------------------------------------
// Global data (kept simple on purpose for a first-semester project)
// ----------------------------------------------------------------------------
Item items[MAX_ITEMS];      // all records in memory
int itemCount = 0;          // how many records are currently stored

string currentStudentId = "";       // logged-in student
string currentStudentName = "";
bool currentIsAdmin = false;

// ----------------------------------------------------------------------------
// String helper functions
// ----------------------------------------------------------------------------

// Remove spaces/tabs/newlines from both ends of a string.
string trim(const string& text) {
    size_t start = text.find_first_not_of(" \t\r\n");
    if (start == string::npos) {
        return "";
    }
    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}

string toLower(string text) {
    for (size_t i = 0; i < text.size(); i++) {
        text[i] = (char)tolower((unsigned char)text[i]);
    }
    return text;
}

string toUpper(string text) {
    for (size_t i = 0; i < text.size(); i++) {
        text[i] = (char)toupper((unsigned char)text[i]);
    }
    return text;
}

// Case-insensitive "does text contain pattern?" (used for searching).
bool containsIgnoreCase(const string& text, const string& pattern) {
    return toLower(text).find(toLower(pattern)) != string::npos;
}

bool equalsIgnoreCase(const string& first, const string& second) {
    return toLower(first) == toLower(second);
}

// Cut long text so it fits inside a table column.
string truncateText(const string& text, size_t width) {
    if (text.size() <= width) {
        return text;
    }
    return text.substr(0, width - 2) + "..";
}

// True when the string is not empty and contains only digits 0-9.
bool isAllDigits(const string& text) {
    if (text.empty()) {
        return false;
    }
    for (size_t i = 0; i < text.size(); i++) {
        if (!isdigit((unsigned char)text[i])) {
            return false;
        }
    }
    return true;
}

// The data file uses '|' as a separator, so it must never appear in user text.
string replaceSeparator(string text) {
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == '|') {
            text[i] = '/';
        }
    }
    return text;
}

// Wrap a value in quotes for CSV and double any quote characters inside it.
string csvEscape(const string& text) {
    string result = "\"";
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == '"') {
            result += "\"\"";
        } else {
            result += text[i];
        }
    }
    result += "\"";
    return result;
}

// Return today's date as YYYY-MM-DD.
string getTodayDate() {
    time_t now = time(0);
    tm* localTime = localtime(&now);
    char buffer[16];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", localTime);
    return string(buffer);
}

// ----------------------------------------------------------------------------
// Safe input functions
// ----------------------------------------------------------------------------

// Read one full line. If the input stream ends (Ctrl+D / Ctrl+Z or end of a
// piped file) the program exits cleanly instead of looping forever.
// All records are already saved after every change, so no data is lost.
string readLine(const string& prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) {
        cout << "\n[Input ended - exiting the program.]\n";
        exit(0);
    }
    return trim(line);
}

// Read an integer between minValue and maxValue; repeat until valid.
int readInt(const string& prompt, int minValue, int maxValue) {
    while (true) {
        string line = readLine(prompt);
        if (!isAllDigits(line) || line.size() > 9) {
            cout << "  [!] Invalid input. Please enter a number between "
                 << minValue << " and " << maxValue << ".\n";
            continue;
        }
        int value = atoi(line.c_str());
        if (value < minValue || value > maxValue) {
            cout << "  [!] Number out of range. Enter " << minValue
                 << " to " << maxValue << ".\n";
            continue;
        }
        return value;
    }
}

// Read text with a maximum length. If 'required' is true, empty input is rejected.
// Optional empty input is stored as "-".
string readText(const string& prompt, size_t maxLength, bool required) {
    while (true) {
        string line = readLine(prompt);
        if (line.empty()) {
            if (required) {
                cout << "  [!] This field cannot be empty.\n";
                continue;
            }
            return "-";
        }
        if (line.size() > maxLength) {
            cout << "  [!] Too long. Maximum " << maxLength << " characters.\n";
            continue;
        }
        return replaceSeparator(line);
    }
}

// Ask a yes/no question; returns true for 'y' and false for 'n'.
bool readYesNo(const string& prompt) {
    while (true) {
        string answer = toLower(readLine(prompt));
        if (answer == "y" || answer == "yes") {
            return true;
        }
        if (answer == "n" || answer == "no") {
            return false;
        }
        cout << "  [!] Please type y or n.\n";
    }
}

void pauseScreen() {
    readLine("\nPress Enter to continue...");
}

// ----------------------------------------------------------------------------
// Validation helpers
// ----------------------------------------------------------------------------

// Student ID: 3 to 15 letters or digits.
bool isValidStudentId(const string& id) {
    if (id.size() < 3 || id.size() > 15) {
        return false;
    }
    for (size_t i = 0; i < id.size(); i++) {
        if (!isalnum((unsigned char)id[i])) {
            return false;
        }
    }
    return true;
}

// Student name: 2 to 40 characters, letters, spaces and dots only.
bool isValidPersonName(const string& name) {
    if (name.size() < 2 || name.size() > 40) {
        return false;
    }
    for (size_t i = 0; i < name.size(); i++) {
        char ch = name[i];
        if (!isalpha((unsigned char)ch) && ch != ' ' && ch != '.') {
            return false;
        }
    }
    return true;
}

// ----------------------------------------------------------------------------
// Array helpers (searching)
// ----------------------------------------------------------------------------

// Linear search: return the array position of an item ID, or -1 if not found.
int findItemIndexById(int id) {
    for (int i = 0; i < itemCount; i++) {
        if (items[i].id == id) {
            return i;
        }
    }
    return -1;
}

// Make sure a new ID is never already used.
bool isIdUsed(int id) {
    return findItemIndexById(id) != -1;
}

// Generate the next unique ID (highest existing ID + 1).
int generateNextId() {
    int highest = FIRST_ITEM_ID - 1;
    for (int i = 0; i < itemCount; i++) {
        if (items[i].id > highest) {
            highest = items[i].id;
        }
    }
    int candidate = highest + 1;
    while (isIdUsed(candidate)) {   // extra safety against duplicates
        candidate++;
    }
    return candidate;
}

// ----------------------------------------------------------------------------
// File handling
// ----------------------------------------------------------------------------

// Split one line at '|' into parts[]. Returns the number of parts found.
int splitRecordLine(const string& line, string parts[], int maxParts) {
    int count = 0;
    string current = "";
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == '|') {
            if (count < maxParts) {
                parts[count] = current;
            }
            count++;
            current = "";
        } else {
            current += line[i];
        }
    }
    if (count < maxParts) {
        parts[count] = current;
    }
    count++;
    return count;
}

// Save all records to data/records.txt. Returns false if the file cannot be written.
bool saveRecords() {
    ofstream file(DATA_FILE.c_str());
    if (!file) {
        cout << "  [ERROR] Could not write to '" << DATA_FILE
             << "'. Run the program from the project root folder.\n";
        return false;
    }
    file << "# ID|TYPE|NAME|CATEGORY|LOCATION|DESCRIPTION|DATE|REPORTER_ID|REPORTER_NAME|STATUS\n";
    for (int i = 0; i < itemCount; i++) {
        file << items[i].id << '|' << items[i].type << '|' << items[i].name << '|'
             << items[i].category << '|' << items[i].location << '|'
             << items[i].description << '|' << items[i].date << '|'
             << items[i].reporterId << '|' << items[i].reporterName << '|'
             << items[i].status << '\n';
    }
    return true;
}

// Load records at start-up. Bad or duplicate lines are skipped, not fatal.
void loadRecords() {
    itemCount = 0;
    ifstream file(DATA_FILE.c_str());
    if (!file) {
        cout << "  [Info] No data file found. Starting with an empty database.\n";
        return;
    }

    string line;
    int skipped = 0;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {       // ignore blank lines and comments
            continue;
        }

        string parts[FIELDS_PER_RECORD];
        int partCount = splitRecordLine(line, parts, FIELDS_PER_RECORD);

        bool valid = (partCount == FIELDS_PER_RECORD) && isAllDigits(parts[0]) &&
                     parts[0].size() <= 9 &&
                     (parts[1] == TYPE_LOST || parts[1] == TYPE_FOUND) &&
                     (parts[9] == STATUS_OPEN || parts[9] == STATUS_RESOLVED);
        if (valid && (isIdUsed(atoi(parts[0].c_str())) || itemCount >= MAX_ITEMS)) {
            valid = false;                          // duplicate ID or array full
        }
        if (!valid) {
            skipped++;
            continue;
        }

        Item item;
        item.id = atoi(parts[0].c_str());
        item.type = parts[1];
        item.name = parts[2];
        item.category = parts[3];
        item.location = parts[4];
        item.description = parts[5];
        item.date = parts[6];
        item.reporterId = parts[7];
        item.reporterName = parts[8];
        item.status = parts[9];
        items[itemCount++] = item;
    }

    cout << "  [Info] Loaded " << itemCount << " record(s) from " << DATA_FILE;
    if (skipped > 0) {
        cout << " (" << skipped << " invalid/duplicate line(s) skipped)";
    }
    cout << ".\n";
}

// ----------------------------------------------------------------------------
// Display helpers
// ----------------------------------------------------------------------------

void printLine(char symbol, int length) {
    cout << string(length, symbol) << "\n";
}

void printBanner(const string& title) {
    cout << "\n";
    printLine('=', 74);
    int padding = (74 - (int)title.size()) / 2;
    if (padding < 0) {
        padding = 0;
    }
    cout << string(padding, ' ') << title << "\n";
    printLine('=', 74);
}

void printTableHeader() {
    cout << left << setw(6) << "ID" << setw(7) << "TYPE" << setw(18) << "ITEM"
         << setw(14) << "CATEGORY" << setw(18) << "LOCATION" << setw(11) << "DATE"
         << setw(9) << "STATUS" << "\n";
    printLine('-', 83);
}

void printTableRow(const Item& item) {
    cout << left << setw(6) << item.id << setw(7) << item.type
         << setw(18) << truncateText(item.name, 17)
         << setw(14) << truncateText(item.category, 13)
         << setw(18) << truncateText(item.location, 17)
         << setw(11) << item.date << setw(9) << item.status << "\n";
}

// Print a table for the items whose positions are listed in indexes[].
void printItemTable(const int indexes[], int count) {
    printTableHeader();
    for (int i = 0; i < count; i++) {
        printTableRow(items[indexes[i]]);
    }
    printLine('-', 83);
}

void printItemDetails(const Item& item) {
    cout << "\n  +-- Item Details ------------------------------------------\n";
    cout << "  | ID          : " << item.id << "\n";
    cout << "  | Type        : " << item.type << "\n";
    cout << "  | Item name   : " << item.name << "\n";
    cout << "  | Category    : " << item.category << "\n";
    cout << "  | Location    : " << item.location << "\n";
    cout << "  | Description : " << item.description << "\n";
    cout << "  | Date        : " << item.date << "\n";
    cout << "  | Reported by : " << item.reporterName << " (" << item.reporterId << ")\n";
    cout << "  | Status      : " << item.status << "\n";
    cout << "  +----------------------------------------------------------\n";
}

// After a table, let the student open full details of one item.
void offerItemDetails() {
    int id = readInt("Enter an Item ID to see full details (0 to skip): ", 0, 999999999);
    if (id == 0) {
        return;
    }
    int index = findItemIndexById(id);
    if (index == -1) {
        cout << "  [!] No record with ID " << id << ".\n";
    } else {
        printItemDetails(items[index]);
    }
}

// Show the category list and return the chosen category name.
string chooseCategory() {
    cout << "\n  Select category:\n";
    for (int i = 0; i < CATEGORY_COUNT; i++) {
        cout << "    " << (i + 1) << ". " << CATEGORIES[i] << "\n";
    }
    int choice = readInt("  Enter category number (1-6): ", 1, CATEGORY_COUNT);
    return CATEGORIES[choice - 1];
}

// ----------------------------------------------------------------------------
// Matching logic (used by Report and Auto Match modules)
// ----------------------------------------------------------------------------

// Give a lost/found pair a similarity score out of 100.
//   Item name : exact = 50, one name contained in the other = 35
//   Category  : same = 30
//   Location  : exact = 20, one contained in the other = 10
// A pair is a "match" when the total score is at least MATCH_THRESHOLD.
int calculateMatchScore(const Item& lostItem, const Item& foundItem) {
    int score = 0;

    string lostName = toLower(lostItem.name);
    string foundName = toLower(foundItem.name);
    if (lostName == foundName) {
        score += 50;
    } else if (lostName.find(foundName) != string::npos ||
               foundName.find(lostName) != string::npos) {
        score += 35;
    }

    if (equalsIgnoreCase(lostItem.category, foundItem.category)) {
        score += 30;
    }

    string lostPlace = toLower(lostItem.location);
    string foundPlace = toLower(foundItem.location);
    if (lostPlace == foundPlace) {
        score += 20;
    } else if (lostPlace.find(foundPlace) != string::npos ||
               foundPlace.find(lostPlace) != string::npos) {
        score += 10;
    }

    return score;
}

// Count how many open items of the opposite type match a newly added item.
int countPossibleMatches(const Item& newItem) {
    int count = 0;
    for (int i = 0; i < itemCount; i++) {
        if (items[i].id == newItem.id || items[i].status != STATUS_OPEN ||
            items[i].type == newItem.type) {
            continue;
        }
        int score = (newItem.type == TYPE_LOST) ? calculateMatchScore(newItem, items[i])
                                                : calculateMatchScore(items[i], newItem);
        if (score >= MATCH_THRESHOLD) {
            count++;
        }
    }
    return count;
}

// ----------------------------------------------------------------------------
// Module 1: Student Login
// ----------------------------------------------------------------------------
const int LOGIN_FAILED = 0;
const int LOGIN_OK = 1;
const int LOGIN_CANCELLED = -1;

int loginStudent() {
    printBanner("STUDENT LOGIN");
    cout << "  (Type 0 as Student ID to cancel)\n\n";

    string id;
    while (true) {
        id = toUpper(readLine("  Student ID (3-15 letters/digits): "));
        if (id == "0") {
            return LOGIN_CANCELLED;
        }
        if (isValidStudentId(id)) {
            break;
        }
        cout << "  [!] Invalid ID. Use 3 to 15 letters or digits, no spaces.\n";
    }

    // The administrator account needs a password and can manage every record.
    if (id == ADMIN_ID) {
        string password = readLine("  Admin password: ");
        if (password != ADMIN_PASSWORD) {
            cout << "  [!] Wrong password. Login failed.\n";
            return LOGIN_FAILED;
        }
        currentStudentId = ADMIN_ID;
        currentStudentName = "Administrator";
        currentIsAdmin = true;
        cout << "\n  Login successful. Welcome, Administrator!\n";
        return LOGIN_OK;
    }

    string name;
    while (true) {
        name = readLine("  Full name: ");
        if (isValidPersonName(name)) {
            break;
        }
        cout << "  [!] Invalid name. Use 2-40 letters, spaces or dots.\n";
    }

    currentStudentId = id;
    currentStudentName = name;
    currentIsAdmin = false;
    cout << "\n  Login successful. Welcome, " << currentStudentName << "!\n";
    return LOGIN_OK;
}

// ----------------------------------------------------------------------------
// Modules 2 & 3: Report Lost Item / Report Found Item (shared logic)
// ----------------------------------------------------------------------------
void reportItem(const string& itemType) {
    if (itemCount >= MAX_ITEMS) {
        cout << "  [ERROR] Database is full (" << MAX_ITEMS << " records). Delete old records first.\n";
        return;
    }

    bool isLost = (itemType == TYPE_LOST);
    printBanner(isLost ? "REPORT LOST ITEM" : "REPORT FOUND ITEM");

    Item item;
    item.id = generateNextId();
    item.type = itemType;
    item.category = chooseCategory();
    item.name = readText("  Item name (e.g. Black Wallet): ", 30, true);
    item.location = readText(isLost ? "  Where did you lose it? " : "  Where did you find it? ", 30, true);
    item.description = readText("  Description - colour, brand, marks (optional): ", 60, false);
    item.date = getTodayDate();
    item.reporterId = currentStudentId;
    item.reporterName = currentStudentName;
    item.status = STATUS_OPEN;

    items[itemCount++] = item;
    if (!saveRecords()) {
        itemCount--;                // undo, because it could not be saved
        return;
    }

    cout << "\n  [OK] " << (isLost ? "Lost" : "Found") << " item recorded. Your Item ID is "
         << item.id << ".\n";

    // Tell the student straight away if a likely match already exists.
    int possible = countPossibleMatches(item);
    if (possible > 0) {
        cout << "  [!] " << possible << " possible match(es) found. Use option 5 (Auto Match) to view them.\n";
    }
}

// ----------------------------------------------------------------------------
// Module 4: Search Item (linear search, case-insensitive)
// ----------------------------------------------------------------------------
void searchItems() {
    printBanner("SEARCH ITEM");
    cout << "  1. Search by item name\n";
    cout << "  2. Search by category\n";
    cout << "  3. Search by location\n";
    cout << "  0. Back to main menu\n";
    int choice = readInt("  Enter your choice (0-3): ", 0, 3);
    if (choice == 0) {
        return;
    }

    string keyword = "";
    if (choice == 2) {
        keyword = chooseCategory();
    } else {
        keyword = readText(choice == 1 ? "  Enter item name to search: " : "  Enter location to search: ", 30, true);
    }

    int results[MAX_ITEMS];
    int resultCount = 0;
    for (int i = 0; i < itemCount; i++) {
        bool found = false;
        if (choice == 1) {
            found = containsIgnoreCase(items[i].name, keyword);
        } else if (choice == 2) {
            found = equalsIgnoreCase(items[i].category, keyword);
        } else {
            found = containsIgnoreCase(items[i].location, keyword);
        }
        if (found) {
            results[resultCount++] = i;
        }
    }

    cout << "\n  Search results for \"" << keyword << "\": " << resultCount << " record(s) found.\n\n";
    if (resultCount == 0) {
        cout << "  No matching records. Try a shorter keyword.\n";
        return;
    }
    printItemTable(results, resultCount);
    offerItemDetails();
}

// ----------------------------------------------------------------------------
// Module 5: Auto Match Lost & Found
// ----------------------------------------------------------------------------
void autoMatchItems() {
    printBanner("AUTO MATCH LOST & FOUND");

    MatchResult matches[MAX_MATCHES];
    int matchCount = 0;

    // Compare every OPEN lost item with every OPEN found item.
    for (int i = 0; i < itemCount && matchCount < MAX_MATCHES; i++) {
        if (items[i].type != TYPE_LOST || items[i].status != STATUS_OPEN) {
            continue;
        }
        for (int j = 0; j < itemCount && matchCount < MAX_MATCHES; j++) {
            if (items[j].type != TYPE_FOUND || items[j].status != STATUS_OPEN) {
                continue;
            }
            int score = calculateMatchScore(items[i], items[j]);
            if (score >= MATCH_THRESHOLD) {
                matches[matchCount].lostIndex = i;
                matches[matchCount].foundIndex = j;
                matches[matchCount].score = score;
                matchCount++;
            }
        }
    }

    if (matchCount == 0) {
        cout << "  No matches found right now. Check again after new items are reported.\n";
        return;
    }

    // Bubble sort: best score first.
    for (int i = 0; i < matchCount - 1; i++) {
        for (int j = 0; j < matchCount - 1 - i; j++) {
            if (matches[j].score < matches[j + 1].score) {
                MatchResult temp = matches[j];
                matches[j] = matches[j + 1];
                matches[j + 1] = temp;
            }
        }
    }

    cout << "  " << matchCount << " possible match(es) found (score = similarity out of 100):\n";
    for (int i = 0; i < matchCount; i++) {
        const Item& lostItem = items[matches[i].lostIndex];
        const Item& foundItem = items[matches[i].foundIndex];
        cout << "\n  Match #" << (i + 1) << "  -  Score: " << matches[i].score << "/100\n";
        cout << "    LOST  [" << lostItem.id << "] " << lostItem.name << " | " << lostItem.category
             << " | " << lostItem.location << " | by " << lostItem.reporterName
             << " (" << lostItem.reporterId << ")\n";
        cout << "    FOUND [" << foundItem.id << "] " << foundItem.name << " | " << foundItem.category
             << " | " << foundItem.location << " | by " << foundItem.reporterName
             << " (" << foundItem.reporterId << ")\n";
    }

    // Optionally mark one pair as resolved (item returned to its owner).
    cout << "\n  If an item has been returned, you can mark the pair as RESOLVED.\n";
    int lostId = readInt("  Enter LOST item ID to resolve (0 to skip): ", 0, 999999999);
    if (lostId == 0) {
        return;
    }
    int foundId = readInt("  Enter matching FOUND item ID: ", 1, 999999999);

    // The chosen pair must be one of the matches displayed above.
    int chosen = -1;
    for (int i = 0; i < matchCount; i++) {
        if (items[matches[i].lostIndex].id == lostId && items[matches[i].foundIndex].id == foundId) {
            chosen = i;
            break;
        }
    }
    if (chosen == -1) {
        cout << "  [!] That pair is not in the match list above. Nothing changed.\n";
        return;
    }

    Item& lostItem = items[matches[chosen].lostIndex];
    Item& foundItem = items[matches[chosen].foundIndex];
    if (!currentIsAdmin && lostItem.reporterId != currentStudentId && foundItem.reporterId != currentStudentId) {
        cout << "  [!] Only the owner, the finder or the admin can resolve this pair.\n";
        return;
    }

    lostItem.status = STATUS_RESOLVED;
    foundItem.status = STATUS_RESOLVED;
    if (saveRecords()) {
        cout << "  [OK] Items " << lostId << " and " << foundId << " marked as RESOLVED.\n";
    }
}

// ----------------------------------------------------------------------------
// Module 6: View All Records (with selection sort)
// ----------------------------------------------------------------------------

// Return true when record 'first' should be placed AFTER record 'second'.
bool shouldComeAfter(const Item& first, const Item& second, int sortKey) {
    switch (sortKey) {
        case 2:     // item name A-Z
            if (toLower(first.name) != toLower(second.name)) {
                return toLower(first.name) > toLower(second.name);
            }
            break;
        case 3:     // category A-Z
            if (first.category != second.category) {
                return first.category > second.category;
            }
            break;
        case 4:     // date, newest first
            if (first.date != second.date) {
                return first.date < second.date;
            }
            break;
        case 5:     // type (FOUND before LOST), then ID
            if (first.type != second.type) {
                return first.type > second.type;
            }
            break;
        default:    // sort key 1 = by ID only
            break;
    }
    return first.id > second.id;    // tie-breaker: lower ID first
}

void viewAllRecords() {
    printBanner("ALL RECORDS");
    if (itemCount == 0) {
        cout << "  The database is empty. Report a lost or found item first.\n";
        return;
    }

    cout << "  Sort records by:\n";
    cout << "    1. Item ID\n    2. Item name (A-Z)\n    3. Category\n";
    cout << "    4. Date (newest first)\n    5. Type (FOUND / LOST)\n";
    int sortKey = readInt("  Enter your choice (1-5): ", 1, 5);

    // Sort an array of positions so the original records stay untouched.
    int order[MAX_ITEMS];
    for (int i = 0; i < itemCount; i++) {
        order[i] = i;
    }

    // Selection sort.
    for (int i = 0; i < itemCount - 1; i++) {
        int smallest = i;
        for (int j = i + 1; j < itemCount; j++) {
            if (shouldComeAfter(items[order[smallest]], items[order[j]], sortKey)) {
                smallest = j;
            }
        }
        int temp = order[i];
        order[i] = order[smallest];
        order[smallest] = temp;
    }

    cout << "\n";
    printItemTable(order, itemCount);
    cout << "  Total records: " << itemCount << "\n";
    offerItemDetails();
}

// ----------------------------------------------------------------------------
// Module 7: Delete Record
// ----------------------------------------------------------------------------
void deleteRecord() {
    printBanner("DELETE RECORD");
    if (itemCount == 0) {
        cout << "  There are no records to delete.\n";
        return;
    }

    int id = readInt("  Enter Item ID to delete (0 to cancel): ", 0, 999999999);
    if (id == 0) {
        cout << "  Delete cancelled.\n";
        return;
    }

    int index = findItemIndexById(id);
    if (index == -1) {
        cout << "  [!] No record found with ID " << id << ".\n";
        return;
    }

    // Students may delete only their own records; the admin may delete any.
    if (!currentIsAdmin && items[index].reporterId != currentStudentId) {
        cout << "  [!] Access denied. You can delete only the records you reported.\n";
        return;
    }

    printItemDetails(items[index]);
    if (!readYesNo("  Are you sure you want to delete this record? (y/n): ")) {
        cout << "  Delete cancelled. Record kept.\n";
        return;
    }

    // Shift the remaining records one position to the left.
    Item backup[1];
    backup[0] = items[index];
    for (int i = index; i < itemCount - 1; i++) {
        items[i] = items[i + 1];
    }
    itemCount--;

    if (saveRecords()) {
        cout << "  [OK] Record " << id << " deleted.\n";
    } else {
        // Saving failed: restore the record in memory to stay consistent with the file.
        for (int i = itemCount; i > index; i--) {
            items[i] = items[i - 1];
        }
        items[index] = backup[0];
        itemCount++;
    }
}

// ----------------------------------------------------------------------------
// Module 8: Item Statistics
// ----------------------------------------------------------------------------
void showStatistics() {
    printBanner("ITEM STATISTICS DASHBOARD");

    int lostTotal = 0, foundTotal = 0, resolvedTotal = 0, resolvedLost = 0;
    int lostByCategory[CATEGORY_COUNT] = {0};
    int foundByCategory[CATEGORY_COUNT] = {0};

    for (int i = 0; i < itemCount; i++) {
        // Find the category position (unknown names are counted as "Other").
        int categoryIndex = CATEGORY_COUNT - 1;
        for (int c = 0; c < CATEGORY_COUNT; c++) {
            if (equalsIgnoreCase(items[i].category, CATEGORIES[c])) {
                categoryIndex = c;
                break;
            }
        }

        if (items[i].type == TYPE_LOST) {
            lostTotal++;
            lostByCategory[categoryIndex]++;
        } else {
            foundTotal++;
            foundByCategory[categoryIndex]++;
        }
        if (items[i].status == STATUS_RESOLVED) {
            resolvedTotal++;
            if (items[i].type == TYPE_LOST) {
                resolvedLost++;
            }
        }
    }

    cout << "  Total records     : " << itemCount << "\n";
    cout << "  Lost items        : " << lostTotal << "\n";
    cout << "  Found items       : " << foundTotal << "\n";
    cout << "  Open records      : " << (itemCount - resolvedTotal) << "\n";
    cout << "  Resolved records  : " << resolvedTotal << "\n";

    // Recovery rate = lost items that have been returned / all lost items.
    double recoveryRate = (lostTotal > 0) ? (resolvedLost * 100.0 / lostTotal) : 0.0;
    cout << "  Recovery rate     : " << fixed << setprecision(1) << recoveryRate << "%\n";

    cout << "\n  Category-wise breakdown\n";
    printLine('-', 60);
    cout << left << setw(15) << "  Category" << setw(7) << "Lost" << setw(7) << "Found"
         << setw(7) << "Total" << "Chart\n";
    printLine('-', 60);
    for (int c = 0; c < CATEGORY_COUNT; c++) {
        int total = lostByCategory[c] + foundByCategory[c];
        cout << "  " << left << setw(13) << CATEGORIES[c] << setw(7) << lostByCategory[c]
             << setw(7) << foundByCategory[c] << setw(7) << total
             << string(total, '#') << "\n";
    }
    printLine('-', 60);
}

// ----------------------------------------------------------------------------
// Module 9: Export Report (CSV, opens in Excel)
// ----------------------------------------------------------------------------
void exportReport() {
    printBanner("EXPORT REPORT");

    ofstream file(REPORT_FILE.c_str());
    if (!file) {
        cout << "  [ERROR] Could not create '" << REPORT_FILE
             << "'. Run the program from the project root and make sure the 'reports' folder exists.\n";
        return;
    }

    file << "ID,Type,Item Name,Category,Location,Description,Date,Reporter ID,Reporter Name,Status\n";
    for (int i = 0; i < itemCount; i++) {
        file << items[i].id << ',' << items[i].type << ','
             << csvEscape(items[i].name) << ',' << csvEscape(items[i].category) << ','
             << csvEscape(items[i].location) << ',' << csvEscape(items[i].description) << ','
             << items[i].date << ',' << csvEscape(items[i].reporterId) << ','
             << csvEscape(items[i].reporterName) << ',' << items[i].status << '\n';
    }
    file.close();

    cout << "  [OK] " << itemCount << " record(s) exported to " << REPORT_FILE << "\n";
}

// ----------------------------------------------------------------------------
// Menu
// ----------------------------------------------------------------------------
void printMainMenu() {
    cout << "\n";
    printLine('=', 74);
    cout << "        SMART CAMPUS LOST & FOUND MANAGEMENT SYSTEM\n";
    printLine('=', 74);
    cout << "  Logged in as: " << currentStudentName << " (" << currentStudentId << ")"
         << (currentIsAdmin ? " [ADMIN]" : "") << "\n";
    printLine('-', 74);
    cout << "   1. Student Login (switch user)\n";
    cout << "   2. Report Lost Item\n";
    cout << "   3. Report Found Item\n";
    cout << "   4. Search Item\n";
    cout << "   5. Auto Match Lost & Found\n";
    cout << "   6. View All Records\n";
    cout << "   7. Delete Record\n";
    cout << "   8. Item Statistics\n";
    cout << "   9. Export Report\n";
    cout << "  10. Exit\n";
    printLine('-', 74);
}

int main() {
    cout << "\n  Starting Smart Campus Lost & Found System...\n";
    loadRecords();

    // A student must log in before using the system.
    bool loggedIn = false;
    while (!loggedIn) {
        int result = loginStudent();
        if (result == LOGIN_CANCELLED) {
            cout << "\n  Goodbye!\n";
            return 0;
        }
        loggedIn = (result == LOGIN_OK);
    }

    bool running = true;
    while (running) {
        printMainMenu();
        int choice = readInt("  Enter your choice (1-10): ", 1, 10);

        switch (choice) {
            case 1:
                loginStudent();     // on failure the previous user stays logged in
                break;
            case 2:
                reportItem(TYPE_LOST);
                break;
            case 3:
                reportItem(TYPE_FOUND);
                break;
            case 4:
                searchItems();
                break;
            case 5:
                autoMatchItems();
                break;
            case 6:
                viewAllRecords();
                break;
            case 7:
                deleteRecord();
                break;
            case 8:
                showStatistics();
                break;
            case 9:
                exportReport();
                break;
            case 10:
                running = false;
                cout << "\n  Thank you for using the system. All records are saved. Goodbye!\n";
                continue;           // skip the pause below
        }
        pauseScreen();
    }
    return 0;
}
