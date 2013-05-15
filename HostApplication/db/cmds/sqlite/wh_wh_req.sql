
BEGIN TRANSACTION; 

/* WH command - it is issued only by Web */
INSERT INTO Commands (DeviceID, CommandCode, CommandStatus, TimePosted, ErrorCode)
SELECT DeviceID, 11, 0, datetime('now'), 0
FROM Devices WHERE Address64 = '00-1B-1E-F9-81-00-00-02';

CREATE TEMPORARY TABLE Var (
col1 CHAR(10),
col2 INTEGER
);
INSERT into Var VALUES ('cmd_id', last_insert_rowid());

/*CmdNo*/
INSERT INTO CommandParameters (CommandID, ParameterCode, ParameterValue)
SELECT col2, 92, 13 FROM Var WHERE col1='cmd_id';

/*DAtaBuffer - if required */
INSERT INTO CommandParameters (CommandID, ParameterCode, ParameterValue)
SELECT col2, 93, '' FROM Var WHERE col1='cmd_id';

/*bypassIOCache - optional*/
INSERT INTO CommandParameters (CommandID, ParameterCode, ParameterValue)
SELECT col2, 1300, 0 FROM Var WHERE col1='cmd_id';


DROP TABLE Var;
COMMIT;

