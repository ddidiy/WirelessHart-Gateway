USE Monitor_Host;

SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;
SET AUTOCOMMIT=0;

START TRANSACTION;

/* WH command - it is issued only by Web; (it may have deviceIDs as parameters)  */
INSERT INTO Commands (DeviceID, CommandCode, CommandStatus, TimePosted, ErrorCode)
SELECT DeviceID, 123, 0, datetime('now'), 0
FROM Devices WHERE Address64 = '00-1B-1E-F9-81-00-00-02';

/*deviceID*/
INSERT INTO CommandParameters (CommandID, ParameterCode, ParameterValue)
SELECT last_insert_id(), 61, DeviceID FROM Devices WHERE Address64 = '00-1B-1E-F8-70-00-55-31';

/*deviceID*/
INSERT INTO CommandParameters (CommandID, ParameterCode, ParameterValue)
SELECT last_insert_id(), 61, DeviceID FROM Devices WHERE Address64 = '00-1B-1E-F8-70-00-55-32';


COMMIT;
SET AUTOCOMMIT=1;
