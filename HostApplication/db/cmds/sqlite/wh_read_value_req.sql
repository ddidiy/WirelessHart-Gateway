
BEGIN TRANSACTION;

/* WH read value command - it is issued only by Web */
INSERT INTO Commands (DeviceID, CommandCode, CommandStatus, TimePosted, ErrorCode)
SELECT DeviceID, 1, 0, datetime('now'), 0
FROM Devices WHERE Address64 = '001B:1EF8:7000:0703';

CREATE TEMPORARY TABLE Var (
col1 CHAR(10),
col2 INTEGER
);
INSERT into Var VALUES ('cmd_id', last_insert_rowid());

/*channelID*/
INSERT INTO CommandParameters (CommandID, ParameterCode, ParameterValue)
SELECT col2, 10, 3 FROM Var WHERE col1='cmd_id';

/*bypassIOCache - optional*/
INSERT INTO CommandParameters (CommandID, ParameterCode, ParameterValue)
SELECT col2, 1300, 0 FROM Var WHERE col1='cmd_id';


DROP TABLE Var;
COMMIT;