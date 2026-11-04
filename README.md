# Smart Campus Lost & Found Management System

A console-based C++ application that helps students report lost items, submit found items, search belongings and automatically match lost and found records.

**Course:** BCA I Semester (2026-2027) | **Lab Sheet:** 10 | **Language:** C++ (standard library only)

---

## 1. Project Introduction

Every college loses ID cards, wallets, keys, laptops and mobile phones every week. Notices on boards and word of mouth are slow and unorganised. This project replaces them with a small, menu-driven program that stores every lost and found report in a file, lets anyone search it, and suggests likely matches automatically.

## 2. Objective

To develop a system where students can:

- report lost items and submit found items,
- search for belongings,
- match lost and found records automatically.

## 3. Problem Statement

Lost items are hard to trace because records are scattered and unsearchable. This system keeps all records in one place, gives each item a unique ID, and compares lost reports with found reports so owners and finders can be connected faster.

## 4. Features

| # | Module | What it does |
|---|--------|--------------|
| 1 | Student Login | Student ID + name (validated). Admin account needs a password. |
| 2 | Report Lost Item | Category, name, location, description. Unique ID and date are generated automatically. |
| 3 | Report Found Item | Same as above for found items. |
| 4 | Search Item | By item name, category or location (case-insensitive, partial match). |
| 5 | Auto Match Lost & Found | Scores every open lost/found pair, lists matches best-first, can mark a pair as RESOLVED. |
| 6 | View All Records | Table of all records, sortable by ID, name, category, date or type. |
| 7 | Delete Record | Asks for confirmation. Students delete only their own records; admin can delete any. |
| 8 | Item Statistics | Totals, open/resolved, recovery rate, category-wise table with a text chart. |
| 9 | Export Report | Writes all records to `reports/item_report.csv` (opens in Excel). |
| 10 | Exit | Saves and quits (data is saved after every change). |

Extra quality features: unique IDs that are never duplicated, input validation everywhere, file-error handling, corrupt-line protection when loading, and an instant "possible match" hint right after a new report.

## 5. Technologies Used

- C++ (tested with C++11, C++14, C++17 and C++20 using g++)
- Standard library only: `iostream`, `fstream`, `string`, `iomanip`, `cctype`, `cstdlib`, `ctime`
- Plain text file (`|` separated) for storage, CSV for export
- Any text editor or IDE (VS Code, Code::Blocks, Dev-C++ ...)

## 6. C++ Concepts Used

| Concept | Where it is used |
|---------|------------------|
| Structures | `struct Item` (a record) and `struct MatchResult` (a lost/found pair) |
| Arrays | `items[500]` stores all records; `CATEGORIES[6]`; index arrays for sorting and search results |
| Functions | One function per module plus helpers (`readInt`, `readText`, `trim`, `saveRecords` ...) |
| File handling | `ifstream` / `ofstream` for `data/records.txt` and the CSV export |
| Loops | Menus, input re-prompting, table printing, searching, sorting |
| Conditional statements | Validation, `switch` menu, permission checks |
| Strings | Case-insensitive comparison, trimming, splitting records at `|` |
| Searching | Linear search by ID and by keyword |
| Sorting | Selection sort (view records), bubble sort (match results) |

## 7. Project Structure

```
Smart-Campus-Lost-Found/
├── src/
│   └── main.cpp            # complete source code
├── data/
│   └── records.txt         # persistent storage (8 sample records included)
├── reports/
│   └── item_report.csv     # sample export generated from the sample data
├── docs/
│   ├── README.md           # this file
│   ├── TEST_REPORT.md      # test cases and real results
│   └── test_log.txt        # raw output of the automated test run
├── tests/
│   └── run_tests.sh        # automated test script (Linux / macOS / Git Bash)
├── screenshots/            # put your own screenshots here
└── .gitignore
```

## 8. Algorithm and Working Logic

**Start-up**
1. Load `data/records.txt` into the `items[]` array (blank lines and `#` comment lines are ignored; invalid or duplicate-ID lines are skipped and counted).
2. Ask the student to log in. Then show the main menu in a loop until Exit.

**Unique ID** - highest existing ID + 1 (first ID is 1001). A safety loop makes sure the ID is not already used.

**Search** - linear search through `items[]`. Name and location use a case-insensitive "contains" test; category uses an exact (case-insensitive) match.

**Auto match** - for every OPEN lost item and every OPEN found item, a score out of 100 is calculated:

| Check | Points |
|-------|--------|
| Item name identical | 50 |
| One item name contains the other (e.g. "Samsung Mobile" / "Samsung Mobile Phone") | 35 |
| Same category | 30 |
| Location identical | 20 |
| One location contains the other | 10 |

A pair with **score >= 60** is a match. Matches are bubble-sorted by score (highest first) and printed together with both reporters' names and IDs so they can contact each other. Name and category/location must agree - category + location alone (50) is not enough.

**View all** - an array of positions is sorted with selection sort, so the stored order is never changed.

**Delete** - find by ID, check permission, show details, ask `y/n`, shift later records left, save to file. If saving fails the record is restored in memory.

**Saving** - the whole file is rewritten after every change, so closing the program at any time never loses data.

## 9. Compilation Instructions

Open a terminal **in the project root folder** (the program uses relative paths such as `data/records.txt`).

**Linux / macOS / Windows (MinGW or Git Bash)**

```bash
g++ -std=c++11 -Wall -Wextra -o lost_found src/main.cpp
./lost_found            # on Windows: lost_found.exe
```

**Code::Blocks / Dev-C++ / VS Code:** add `src/main.cpp` to a project, set the working directory to the project root folder, build and run.

If the working directory is wrong you will see `Could not write to 'data/records.txt'`.

**Demo logins**

| User | Student ID | Name / password |
|------|-----------|-----------------|
| Any student | e.g. `BCA2601` | any valid name (letters, spaces, dots) |
| Owner of sample lost wallet | `BCA2601` | `Aarav Sharma` |
| Administrator | `ADMIN` | password `admin123` (demo only) |

## 10. Sample Input / Output

Real output from a run on the included sample data (input shown after each prompt as `<<`). Dates come from the system clock.

**Login and menu**

```
==========================================================================
                              STUDENT LOGIN
==========================================================================
  Student ID (3-15 letters/digits): << bca2650
  Full name: << Riya Bisht

  Login successful. Welcome, Riya Bisht!

==========================================================================
        SMART CAMPUS LOST & FOUND MANAGEMENT SYSTEM
==========================================================================
  Logged in as: Riya Bisht (BCA2650)
--------------------------------------------------------------------------
   1. Student Login (switch user)
   2. Report Lost Item
   ...
  10. Exit
```

**Report lost item** (choices: category `6`, name `Scientific Calculator`, location `Exam Hall`, description `Casio fx-991, name written on back`)

```
  [OK] Lost item recorded. Your Item ID is 1009.
```

**Search by name** (`4`, `1`, `wallet`)

```
  Search results for "wallet": 2 record(s) found.

ID    TYPE   ITEM              CATEGORY      LOCATION          DATE       STATUS
-----------------------------------------------------------------------------------
1001  LOST   Black Wallet      Wallet        Library           2026-10-12 OPEN
1002  FOUND  Black Wallet      Wallet        Library           2026-10-13 OPEN
-----------------------------------------------------------------------------------
```

**Auto match** (option 5 on the unchanged sample data)

```
  Match #1  -  Score: 100/100
    LOST  [1001] Black Wallet | Wallet | Library | by Aarav Sharma (BCA2601)
    FOUND [1002] Black Wallet | Wallet | Library | by Priya Negi (BCA2614)

  Match #2  -  Score: 85/100
    LOST  [1003] Samsung Mobile | Mobile Phone | Canteen | by Rohan Verma (BCA2622)
    FOUND [1004] Samsung Mobile Phone | Mobile Phone | Canteen | by Neha Joshi (BCA2630)
```

**Statistics** (after adding the calculator above)

```
  Total records     : 9
  Lost items        : 5
  Found items       : 4
  Open records      : 7
  Resolved records  : 2
  Recovery rate     : 20.0%

  Category     Lost   Found  Total  Chart
  ID Card      0      1      1      #
  Wallet       1      1      2      ##
  Keys         1      0      1      #
  Laptop       1      1      2      ##
  Mobile Phone 1      1      2      ##
  Other        1      0      1      #
```

**Exported CSV** (`reports/item_report.csv`)

```
ID,Type,Item Name,Category,Location,Description,Date,Reporter ID,Reporter Name,Status
1001,LOST,"Black Wallet","Wallet","Library","Leather wallet with college ID inside",2026-10-12,"BCA2601","Aarav Sharma",OPEN
```

**Data file format** (`data/records.txt`)

```
# ID|TYPE|NAME|CATEGORY|LOCATION|DESCRIPTION|DATE|REPORTER_ID|REPORTER_NAME|STATUS
1001|LOST|Black Wallet|Wallet|Library|Leather wallet with college ID inside|2026-10-12|BCA2601|Aarav Sharma|OPEN
```

## 11. Testing

An automated script (`tests/run_tests.sh`) compiles the program and feeds scripted keyboard input to it inside a temporary copy of the project, so your real data is never changed.

```bash
bash tests/run_tests.sh
```

Result of the last run: **74 checks passed, 0 failed.** The scenarios cover login, reporting lost and found items, searching, auto matching, viewing/sorting, deleting, statistics, export, restart persistence, and invalid input. Full list and notes: [`TEST_REPORT.md`](TEST_REPORT.md).

## 12. Limitations

- Maximum 500 records (fixed-size array, as taught in Semester I).
- Student login is by self-declared ID and name (no registration or password) - suitable for a lab project, not for real deployment. The admin password is hard-coded for demonstration.
- Matching is text-based, so very different wording ("mobile" vs "phone") may not be matched; the search module can still find it.

## 13. Future Enhancements

These are ideas only and are **not** implemented:

- QR code verification when an item is handed back
- Email notification to the owner when a match is found
- Face recognition for verifying ID card ownership
- Mobile application for reporting and searching on the go

## 14. Conclusion

The project shows how structures, arrays, functions, file handling, searching and sorting combine into a complete, real-world application. It is written in plain standard C++, stores data permanently, validates all input and is easy to extend.
