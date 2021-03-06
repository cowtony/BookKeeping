CREATE TABLE "Currency Types"([Name] TEXT PRIMARY KEY ASC) WITHOUT ROWID;

CREATE TABLE [Asset](
  [Category] TEXT NOT NULL DEFAULT undefined, 
  [Name] TEXT NOT NULL, 
  [Currency] TEXT NOT NULL DEFAULT USD REFERENCES [Currency Types]([Name]) ON DELETE RESTRICT ON UPDATE CASCADE, 
  UNIQUE([Category] ASC, [Name] ASC) ON CONFLICT FAIL);

CREATE TABLE [Equity](
  [Category] TEXT NOT NULL DEFAULT undefined, 
  [Name] TEXT NOT NULL, 
  UNIQUE([Category] ASC, [Name] ASC) ON CONFLICT FAIL);

CREATE TABLE [Expense](
  [Category] TEXT NOT NULL DEFAULT undefined, 
  [Name] TEXT NOT NULL, 
  UNIQUE([Category] ASC, [Name] ASC) ON CONFLICT FAIL);

CREATE TABLE [Liability](
  [Category] TEXT NOT NULL DEFAULT undefined, 
  [Name] TEXT NOT NULL, 
  [Currency] TEXT NOT NULL DEFAULT USD REFERENCES [Currency Types]([Name]) ON DELETE RESTRICT ON UPDATE CASCADE, 
  UNIQUE([Category] ASC, [Name] ASC) ON CONFLICT FAIL);

CREATE TABLE [Log](
  [Time] TEXT PRIMARY KEY NOT NULL UNIQUE, 
  [Query] TEXT) WITHOUT ROWID;

CREATE TABLE [Log Time](
  [Date] TEXT PRIMARY KEY NOT NULL UNIQUE, 
  [Seconds] INT) WITHOUT ROWID;

CREATE TABLE [Revenue](
  [Category] TEXT NOT NULL DEFAULT undefined, 
  [Name] TEXT NOT NULL, 
  UNIQUE([Category] ASC, [Name] ASC) ON CONFLICT FAIL);

CREATE TABLE [Transactions](
  [Date] TEXT PRIMARY KEY ASC NOT NULL, 
  [Description] TEXT NOT NULL ON CONFLICT REPLACE DEFAULT Empty, 
  [Expense] TEXT NOT NULL ON CONFLICT REPLACE DEFAULT Empty, 
  [Revenue] TEXT NOT NULL ON CONFLICT REPLACE DEFAULT Empty, 
  [Asset] TEXT NOT NULL ON CONFLICT REPLACE DEFAULT Empty, 
  [Liability] TEXT NOT NULL ON CONFLICT REPLACE DEFAULT Empty) WITHOUT ROWID;


