# Test Report

All results below come from a real run of `bash tests/run_tests.sh` (raw output in `test_log.txt`).

| Item | Value |
|------|-------|
| Compiler | g++ 13.3.0 (Ubuntu 24.04) |
| Compile flags | `-std=c++11 -Wall -Wextra -pedantic` -> **0 errors, 0 warnings** |
| Also compiled with | `-std=c++14`, `-std=c++17`, `-std=c++20` (all OK) |
| Automated checks | **74 passed, 0 failed** |
| Sanitizer run | One full session (admin login, report lost/found, search, auto-match, view, delete, statistics, export) built with `-fsanitize=address,undefined` -> no runtime errors or leaks reported |

## How testing works

The script copies the sample data into a temporary folder, runs the program with scripted keyboard input and then checks the screen output and the saved files. The real `data/records.txt` is never modified. Sample data = 8 records (4 lost, 4 found, 2 resolved).

## Test cases

| ID | Scenario | Input / action | Expected result | Result |
|----|----------|----------------|-----------------|--------|
| T01 | Student login | `bca2650`, `Test Student` | Welcome message; ID shown in upper case | PASS |
| T02 | Invalid login input | ID `ab`, `bad id!`; name `R2D2`, `T` | Errors shown, login succeeds after correction | PASS |
| T03 | Wrong admin password / cancel | `admin`, `wrong`, then `0` | "Wrong password"; `0` cancels | PASS |
| T04 | Admin login | `ADMIN` / `admin123` | `[ADMIN]` shown | PASS |
| T05 | Switch user | Menu option 1 | New user shown in menu | PASS |
| T06 | Report lost item | Name `Room Keys\|X` | ID 1009; `\|` saved as `/`; empty description saved as `-` | PASS |
| T07 | Report found item | Found `BLACK WALLET` at `library` | ID 1009; possible-match hint (case-insensitive) | PASS |
| T08 | Invalid report input | Category `9`, `0`; empty name | Errors shown, report completes after correction | PASS |
| T09 | Over-long text | 40-character item name | Rejected (max 30) | PASS |
| T10 | Search by name | `wallet` | 2 records | PASS |
| T11 | Search by category | Mobile Phone | 2 records; details view works | PASS |
| T12 | Search by location | `LAB` | 2 records | PASS |
| T13 | No result / bad menu input | `zzzz`, `4`, `7`, `abc` | "0 record(s)"; errors handled | PASS |
| T14 | Auto match | Sample data | 2 matches, 100 before 85; owner resolves pair; status saved; recovery rate 50.0% | PASS |
| T15 | Resolve permission | Unrelated student; pair not in list | Rejected both times | PASS |
| T16 | New item creates match | Lost "Hostel Keys", found "Room Keys" at another place | Match with score 65 (35 + 30) | PASS |
| T17 | Unrelated item | New "Pink Bag" | No extra matches | PASS |
| T18 | View all, sort by name | Option 6 -> 2 | A-Z order; 8 records; details of 1005 | PASS |
| T19 | Sort by date | Option 6 -> 4 | Newest first; unknown ID handled | PASS |
| T20 | Sort by type | Option 6 -> 5 | FOUND before LOST | PASS |
| T21 | Sort by category | Option 6 -> 3 | Runs correctly | PASS |
| T22 | Delete someone else's record | Student `STU999` deletes 1001 | "Access denied"; record kept | PASS |
| T23 | Delete own record | `n`, invalid answer, `y`, unknown ID, `0` | Kept on `n`; re-asked on invalid; deleted on `y`; removed from file | PASS |
| T24 | Admin delete | Admin deletes 1008 | Deleted | PASS |
| T25 | Statistics | Sample data | Total 8, lost 4, found 4, resolved 2, recovery 25.0% | PASS |
| T26 | Export CSV | After adding a record whose description is `Red, "big" bag` | 9 records exported; header + 9 rows; comma and quotes correctly escaped | PASS |
| T27 | Restart persistence | Run 1 adds record; run 2 (new process) | Record visible; next ID 1010; no duplicate IDs in file | PASS |
| T28 | Invalid menu input | `abc`, empty, `0`, `11`, `-1`, `1.5` | All rejected; program still exits normally | PASS |
| T29 | End of input | Input stops after login | "Input ended", clean exit, no infinite loop | PASS |
| T30 | Corrupt data file | 5 bad/duplicate lines appended | 8 valid records loaded; "5 ... skipped" reported | PASS |
| T31 | Missing data file | `records.txt` removed | Empty start; statistics work; file re-created with ID 1001 | PASS |
| T32 | Folders missing | `data/` and `reports/` removed | Save and export errors shown; failed save rolled back (no ghost record) | PASS |

## Not tested

- Compiling on Windows (MinGW / MSVC / Code::Blocks) or macOS - only g++ on Ubuntu was used. The code uses only standard headers, so it should work, but this has not been verified here.
- Interactive typing in a real terminal window (tests use piped input). The screen layout was checked from captured output, not screenshots; the `screenshots/` folder is empty for you to fill in.
- Valgrind (not installed); AddressSanitizer/UBSan was used instead.
