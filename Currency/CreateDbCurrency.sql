CREATE TABLE [Currency](
  [Date] TEXT PRIMARY KEY ASC, 
  [EUR] REAL, 
  [USD] REAL, 
  [CNY] REAL, 
  [GBP] REAL) WITHOUT ROWID;

CREATE TABLE "Currency Types"([Name] TEXT PRIMARY KEY ASC) WITHOUT ROWID;
INSERT INTO "Currency Types"
    (Name)
VALUES
    ("CNY"),
    ("EUR"),
    ("GBP"),
    ("USD");
