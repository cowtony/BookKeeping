## Deploy

`C:\Qt\6.4.1\mingw_64\bin\windeployqt.exe C:\Users\cowto\GitHub\build-BookKeeping-Desktop_Qt_6_4_1_MinGW_64_bit-Release\BookKeeping\release\Currency.dll`

## TODO
- PreTax asset account
- Household manager
- QDatabaseModel
- Move account is buggy.
- Return string as error message instead of bool
- Person 1, Person 2, etc. capably of rename people.
- keep copy-to-clipboard as all months.
- After adding a transaction, the list always goes to the top
- Separated bar chart
- Account Manager: Add currency selection

BookKeeping:
MAC dynamic library


## Schema

Fact Table: Transaction
  PK: Date
  Column: Description
  Column: Asset (repeated)
  Column: Revenue (repeated)
  Column: Liability (repeated)
  Column: Receivable (repeated)

Dimension table: Asset
  PK: ID
  Column: Type
  Column: Name