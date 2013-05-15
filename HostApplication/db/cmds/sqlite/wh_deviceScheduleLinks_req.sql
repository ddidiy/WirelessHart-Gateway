
BEGIN TRANSACTION;

/* WH command - it is issued only by Web; (it may have deviceIDs as parameters)*/
INSERT INTO Commands (DeviceID, CommandCode, CommandStatus, TimePosted, ErrorCode)
SELECT DeviceID, 123, 0, datetime('now'), 0
FROM Devices WHERE Address64 = '00-1B-1E-F9-81-00-00-02';

CREATE TEMPORARY TABLE Var (
col1 CHAR(10),
col2 INTEGER
);
INSERT into Var VALUES ('cmd_id', last_insert_rowid());

/*deviceID*/
INSERT INTO CommandParameters (CommandID, ParameterCode, ParameterValue)
SELECT col2, 61, DeviceID FROM Var, Devices WHERE col1='cmd_id' AND Address64 = '00-1B-1E-F8-70-00-55-31';

/*deviceID*/
INSERT INTO CommandParameters (CommandID, ParameterCode, ParameterValue)
SELECT col2, 61, DeviceID FROM Var, Devices WHERE col1='cmd_id' AND Address64 = '00-1B-1E-F8-70-00-55-32';



DROP TABLE Var;
COMMIT;
