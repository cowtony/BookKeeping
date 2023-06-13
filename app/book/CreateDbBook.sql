CREATE TABLE [auth_user](
  [user_id] INTEGER PRIMARY KEY AUTOINCREMENT,
  [password] TEXT,
  [last_login] DATETIME,
  [is_superuser] BOOLEAN,
  [username] TEXT NOT NULL,
  [first_name] TEXT,
  [last_name] TEXT,
  [email] TEXT,
  [is_staff] BOOLEAN,
  [is_active] BOOLEAN,
  [date_joined] DATETIME);

CREATE TABLE [book_account_types](
  [account_type_id] INTEGER PRIMARY KEY AUTOINCREMENT,
  [type_name] TEXT NOT NULL UNIQUE);

CREATE TABLE [book_account_categories](
  [category_id] INTEGER PRIMARY KEY AUTOINCREMENT,
  [user_id] INTEGER NOT NULL REFERENCES [auth_user]([user_id]) ON DELETE RESTRICT ON UPDATE CASCADE,
  [account_type_id] INTEGER NOT NULL REFERENCES [book_account_types]([account_type_id]) ON DELETE RESTRICT ON UPDATE CASCADE,
  [category_name] TEXT NOT NULL,
  UNIQUE([user_id], [account_type_id], [category_name]) ON CONFLICT ABORT);

CREATE TABLE [currency_types](
  [currency_id] INTEGER PRIMARY KEY AUTOINCREMENT,
  [Name] TEXT NOT NULL UNIQUE,
  [currency_symbol] TEXT NOT NULL);

CREATE TABLE [book_accounts](
  [account_id] INTEGER PRIMARY KEY AUTOINCREMENT,
  [category_id] INTEGER NOT NULL REFERENCES [book_account_categories]([category_id]) ON DELETE RESTRICT ON UPDATE CASCADE,
  [account_name] TEXT NOT NULL,
  [currency_id] INTEGER DEFAULT 1 REFERENCES [currency_types]([currency_id]) ON DELETE RESTRICT ON UPDATE CASCADE,
  [comment] TEXT,
  [is_investment] BOOL DEFAULT False,
  UNIQUE([category_id], [account_name]) ON CONFLICT ABORT);

CREATE TABLE [book_households](
  [household_id] INTEGER PRIMARY KEY AUTOINCREMENT,
  [user_id] INTEGER NOT NULL REFERENCES [auth_user]([user_id]) ON DELETE RESTRICT ON UPDATE CASCADE,
  [name] TEXT NOT NULL UNIQUE,
  [rank] INTEGER DEFAULT 99);

CREATE TABLE [book_transactions](
  [transaction_id] INTEGER PRIMARY KEY AUTOINCREMENT,
  [user_id] INTEGER NOT NULL REFERENCES [auth_user]([user_id]) ON DELETE RESTRICT ON UPDATE CASCADE,
  [date_time] TEXT NOT NULL,
  [description] TEXT NOT NULL ON CONFLICT REPLACE DEFAULT Empty,
  [deprecate_detail] TEXT);

CREATE TABLE [book_transaction_details](
  [detail_id] INTEGER PRIMARY KEY AUTOINCREMENT,
  [transaction_id] INTEGER NOT NULL REFERENCES [book_transactions]([transaction_id]) ON DELETE RESTRICT ON UPDATE CASCADE,
  [account_id] INTEGER NOT NULL REFERENCES [book_accounts]([account_id]) ON DELETE RESTRICT ON UPDATE CASCADE,
  [household_id] INTEGER REFERENCES [book_households]([household_id]) ON DELETE RESTRICT ON UPDATE CASCADE,
  [currency_id] INTEGER DEFAULT 1 REFERENCES [currency_types]([currency_id]) ON DELETE RESTRICT ON UPDATE CASCADE,
  [amount] REAL NOT NULL,
  UNIQUE([transaction_id], [account_id], [household_id]) ON CONFLICT ABORT);

CREATE TABLE "deprecate_book_asset"(
  [asset_id] INT PRIMARY KEY,
  [category_id] INT,
  [Category] TEXT NOT NULL DEFAULT undefined,
  [Name] TEXT NOT NULL UNIQUE,
  [Comment] TEXT,
  [Currency] TEXT NOT NULL DEFAULT USD REFERENCES "currency_types"([Name]) ON DELETE RESTRICT ON UPDATE CASCADE,
  [IsInvestment] BOOL DEFAULT False,
  UNIQUE([Category] ASC, [Name] ASC) ON CONFLICT FAIL);

CREATE TABLE "deprecate_book_equity"(
  [Category] TEXT NOT NULL DEFAULT undefined,
  [Name] TEXT NOT NULL,
  [Comment] TEXT,
  UNIQUE([Category] ASC, [Name] ASC) ON CONFLICT FAIL);

CREATE TABLE "deprecate_book_expense"(
  [Category] TEXT NOT NULL DEFAULT undefined,
  [Name] TEXT NOT NULL,
  [Comment] TEXT,
  UNIQUE([Category] ASC, [Name] ASC) ON CONFLICT FAIL);

CREATE TABLE "deprecate_book_liability"(
  [Category] TEXT NOT NULL DEFAULT undefined,
  [Name] TEXT NOT NULL UNIQUE,
  [Comment] TEXT,
  [Currency] TEXT NOT NULL DEFAULT USD REFERENCES "currency_types"([Name]) ON DELETE RESTRICT ON UPDATE CASCADE,
  UNIQUE([Category] ASC, [Name] ASC) ON CONFLICT FAIL);

CREATE TABLE "deprecate_book_revenue"(
  [revenue_id] INTEGER PRIMARY KEY AUTOINCREMENT,
  [Category] TEXT NOT NULL DEFAULT undefined,
  [Name] TEXT NOT NULL UNIQUE,
  [Comment] TEXT,
  UNIQUE([Category] ASC, [Name] ASC) ON CONFLICT FAIL);

CREATE TABLE [Log](
  [Time] TEXT PRIMARY KEY NOT NULL UNIQUE,
  [Query] TEXT) WITHOUT ROWID;

CREATE TABLE [Log Time](
  [Date] TEXT PRIMARY KEY NOT NULL UNIQUE,
  [Seconds] INT) WITHOUT ROWID;

CREATE VIEW [accounts_view]
AS
SELECT
       [auth_user].[user_id],
       [auth_user].[username],
       [t].[account_type_id],
       [t].[type_name],
       [c].[category_id],
       [c].[category_name],
       [a].[account_id],
       [a].[account_name],
       [a].[comment],
       [cur].[Name] AS [currency_name],
       [a].[is_investment]
FROM   [book_accounts] AS [a]
       INNER JOIN [book_account_categories] AS [c] ON [a].[category_id] = [c].[category_id]
       INNER JOIN [currency_types] AS [cur] ON [a].[currency_id] = [cur].[currency_id]
       INNER JOIN [auth_user] ON [auth_user].[user_id] = [c].[user_id]
       INNER JOIN [book_account_types] AS [t] ON [t].[account_type_id] = [c].[account_type_id];

CREATE VIEW [transaction_details_view]
AS
SELECT
       [t].[user_id],
       [t].[transaction_id],
       [t].[date_time],
       [t].[description],
       [a].[category_id],
       [d].[account_id],
       [a].[type_name],
       [a].[category_name],
       [a].[account_name],
       [h].[name] AS [household_name],
       [c].[currency_symbol],
       [d].[amount]
FROM   [book_transactions] AS [t]
       JOIN [book_transaction_details] AS [d] ON [t].[transaction_id] = [d].[transaction_id]
       JOIN [accounts_view] AS [a] ON [a].[account_id] = [d].[account_id]
       LEFT JOIN [book_households] AS [h] ON [h].[household_id] = [d].[household_id]
       JOIN [currency_types] AS [c] ON [c].[currency_id] = [d].[currency_id];

CREATE VIEW [transactions_view]
AS
SELECT
       [user_id],
       [transaction_id],
       [date_time] AS [Timestamp],
       [description] AS [Description],
       GROUP_CONCAT (CASE WHEN [type_name] = 'Expense' THEN [category_name] || '|' || [account_name] || ', ' || [household_name] || ', ' || [currency_symbol] || PRINTF ('%.2f', [amount]) ELSE NULL END, '\n') AS [Expense],
       GROUP_CONCAT (CASE WHEN [type_name] = 'Revenue' THEN [category_name] || '|' || [account_name] || ', ' || [household_name] || ', ' || [currency_symbol] || PRINTF ('%.2f', [amount]) ELSE NULL END, '\n') AS [Revenue],
       GROUP_CONCAT (CASE WHEN [type_name] = 'Asset' THEN [category_name] || '|' || [account_name] || ', ' || [currency_symbol] || PRINTF ('%.2f', [amount]) ELSE NULL END, '\n') AS [Asset],
       GROUP_CONCAT (CASE WHEN [type_name] = 'Liability' THEN [category_name] || '|' || [account_name] || ', ' || [currency_symbol] || PRINTF ('%.2f', [amount]) ELSE NULL END, '\n') AS [Liability]
FROM   [transaction_details_view]
GROUP  BY [transaction_id];

CREATE INDEX [] ON [book_transaction_details]([transaction_id]);

