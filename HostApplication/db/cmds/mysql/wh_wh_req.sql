USE Monitor_Host_17;

SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;
SET AUTOCOMMIT=0;

START TRANSACTION;

/* WH command - it is issued only by Web */
INSERT INTO Commands (DeviceID, CommandCode, CommandStatus, TimePosted, ErrorCode)
SELECT DeviceID, 11, 0, Now(), 0
FROM Devices WHERE Address64 = '00-1B-1E-F9-81-00-00-02';


/*CmdNo -> in ParameterValue*/ 
INSERT INTO CommandParameters (CommandID, ParameterCode, ParameterValue)
VALUES(last_insert_id(), 92, 18);

/*DataBuffer - if required -> in ParameterValue */
INSERT INTO CommandParameters (CommandID, ParameterCode, ParameterValue)
VALUES(last_insert_id(), 93, '0600600600412D82747EFDC0C6FDE87DB274E0B7B6');

/*bypassIOCache - optional*/
INSERT INTO CommandParameters (CommandID, ParameterCode, ParameterValue)
VALUES(last_insert_id(), 1300, 0);

COMMIT;
SET AUTOCOMMIT=1;

